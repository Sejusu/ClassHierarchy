#include "validator.h"
#include <QRegularExpression>
#include <QJsonValue>

validator::validator() {}

// Проверка на допустимые символы (алфавиты, цифры, подчёркивание, дефис)
static bool isValidName(const QString& name) {
    static QRegularExpression re("^[A-Za-z0-9_А-Яа-яёЁ\\-]+$");
    return re.match(name).hasMatch();
}

bool checkStringCharacters(const QString& name, const QString& contextError, QSet<Error>& errors, bool isClass = true) {
    if (name.isEmpty() || name.length() > 255) {
        ErrorType lengthError = isClass ? ErrorType::invalidClassNameLength
                                        : ErrorType::invalidPropertyNameLength;
        errors.insert(Error(lengthError, name, "", 0, name.length()));
    }

    if (!isValidName(name)) {
        QString badChar;
        for (QChar ch : name) {
            if (!isValidName(QString(ch))) { badChar = QString(ch); break; }
        }
        errors.insert(Error(ErrorType::invalidCharacters, name, badChar));
        return false;
    }
    return true;
}

void checkValueCountRule(const QJsonObject& propObj, const QString& propName, Property& prop, QSet<Error>& errors) {
    if (!propObj.contains("value_count")) return;

    QJsonValue vcVal = propObj.value("value_count");
    if (!vcVal.isArray() || vcVal.toArray().size() != 1 || !vcVal.toArray().at(0).isDouble()) {
        errors.insert(Error(ErrorType::invalidValueCountType, propName));
        return;
    }

    double dVal = vcVal.toArray().at(0).toDouble();
    int val = vcVal.toArray().at(0).toInt();
    if (dVal != val || val < 1 || val > 1000) {
        errors.insert(Error(ErrorType::invalidValueRange, propName, "", val));
    } else {
        prop.valueCount.append(val);
        prop.ruleType = PropertyRuleType::HasPropertyWithCount;
    }
}

void checkExpectedValueRule(const QJsonObject& propObj, const QString& propName, Property& prop, QSet<Error>& errors) {
    if (!propObj.contains("expected_value")) return;

    QJsonValue evVal = propObj.value("expected_value");
    if (!evVal.isArray()) {
        errors.insert(Error(ErrorType::invalidValueType, propName));
        return;
    }

    QJsonArray evArray = evVal.toArray();
    if (evArray.isEmpty()) {
        errors.insert(Error(ErrorType::emptyExpectedValue, propName));
        return;
    }
    if (evArray.size() > 100) {
        errors.insert(Error(ErrorType::tooManyExpectedValues, propName, "", 0, evArray.size()));
        return;
    }

    bool validItems = true;
    for (int k = 0; k < evArray.size(); ++k) {
        if (!evArray.at(k).isDouble()) {
            errors.insert(Error(ErrorType::invalidValueType, propName));
            validItems = false; break;
        }
        double dVal = evArray.at(k).toDouble();
        int val = evArray.at(k).toInt();
        if (dVal != val || val < 1 || val > 1000) {
            errors.insert(Error(ErrorType::invalidValueRange, propName, "", val));
            validItems = false; break;
        }
        prop.expectedValues.append(val);
    }
    if (validItems) {
        prop.ruleType = (evArray.size() == 1) ? PropertyRuleType::HasPropertyWithValue
                                              : PropertyRuleType::HasPropertyWithValues;
    }
}

bool validatePropertyObject(const QJsonObject& propObj, int index, const QString& className, QSet<Error>& errors, Property& prop) {
    for (const QString& key : propObj.keys()) {
        if (key != "name" && key != "value_count" && key != "expected_value") {
            errors.insert(Error(ErrorType::extraField, key));
        }
    }
    if (!propObj.contains("name")) {
        errors.insert(Error(ErrorType::missingPropertyName, className, "", 0, index + 1));
        return false;
    }
    QJsonValue propNameVal = propObj.value("name");
    if (!propNameVal.isString()) {
        errors.insert(Error(ErrorType::invalidCharacters, QString("свойства №%1 в классе %2").arg(index + 1).arg(className)));
        return false;
    }

    QString propName = propNameVal.toString();
    checkStringCharacters(propName, className, errors, false);

    prop.name = propName;
    prop.ruleType = PropertyRuleType::HasProperty;

    if (propObj.contains("value_count") && propObj.contains("expected_value")) {
        errors.insert(Error(ErrorType::ambiguousRule, propName));
    }

    checkValueCountRule(propObj, propName, prop, errors);
    checkExpectedValueRule(propObj, propName, prop, errors);
    return true;
}

void validateClassStructuredObject(const QJsonObject& classObj, int index, QSet<Error>& errors, QVector<Class>& parsedClasses) {
    for (const QString& key : classObj.keys()) {
        if (key != "class_name" && key != "properties") errors.insert(Error(ErrorType::extraField, key));
    }
    if (!classObj.contains("class_name")) {
        errors.insert(Error(ErrorType::missingClassName, "", "", 0, index + 1)); return;
    }
    QJsonValue nameVal = classObj.value("class_name");
    if (!nameVal.isString()) {
        errors.insert(Error(ErrorType::invalidCharacters, QString("класса №%1").arg(index + 1))); return;
    }

    QString className = nameVal.toString();
    checkStringCharacters(className, QString("класса №%1").arg(index + 1), errors);

    if (!classObj.contains("properties") || !classObj.value("properties").isArray()) {
        errors.insert(Error(ErrorType::missingProperties, className)); return;
    }

    QJsonArray propArray = classObj.value("properties").toArray();
    if (propArray.isEmpty()) errors.insert(Error(ErrorType::emptyProperties, className));
    if (propArray.size() > 100) errors.insert(Error(ErrorType::tooManyProperties, className, "", 0, propArray.size()));

    Class cls; cls.name = className;
    for (int j = 0; j < propArray.size(); ++j) {
        if (!propArray.at(j).isObject()) continue;
        Property prop;
        if (validatePropertyObject(propArray.at(j).toObject(), j, className, errors, prop)) {
            cls.properties.append(prop);
        }
    }
    parsedClasses.append(cls);
}

bool validateInput(const QJsonObject& rootObj, QSet<Error>& errors, QVector<Class>& parsedClasses) {

    if (!rootObj.contains("classes") || !rootObj.value("classes").isArray()) {
        errors.insert(Error(ErrorType::emptyClassesArray));
        return false;
    }

    QJsonArray classesArray = rootObj.value("classes").toArray();

    if (classesArray.isEmpty()) {
        errors.insert(Error(ErrorType::emptyClassesArray));
        return false;
    }

    if (classesArray.size() > 100) {
        errors.insert(Error(ErrorType::tooManyClasses, "", "", 0, classesArray.size()));
        return false;
    }

    for (int i = 0; i < classesArray.size(); ++i) {
        if (classesArray.at(i).isObject()) {
            validateClassStructuredObject(classesArray.at(i).toObject(), i, errors, parsedClasses);
        }
    }

    return errors.isEmpty();
}
