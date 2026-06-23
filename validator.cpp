#include "validator.h"
#include <QRegularExpression>
#include <QJsonValue>

/*!
 * \brief Конструктор по умолчанию класса validator.
 */
validator::validator() {}

/*!
 * \brief Внутренняя функция для проверки соответствия имени регулярному выражению.
 * * Допускает использование букв латиницы и кириллицы (включая ё/Ё), цифр,
 * знаков подчёркивания и дефисов.
 * * \param[in] name Проверяемая строка.
 * \return \c true, если строка содержит только валидные символы, иначе \c false.
 */
static bool isValidName(const QString& name) {
    // Статическое регулярное выражение, разрешающее буквы, цифры, дефис и подчеркивание
    static QRegularExpression re("^[A-Za-z0-9_А-Яа-яёЁ\\-]+$");
    return re.match(name).hasMatch();
}

/*!
 * \brief Внутренняя функция комплексной проверки строк на ограничения длины и символьный состав.
 * * Выявляет недопустимые символы, посимвольно локализует первый некорректный знак
 * для вывода подробного контекста ошибки.
 * * \param[in] name Проверяемое текстовое имя.
 * \param[in] contextError Текстовое описание контекста для логирования.
 * \param[out] errors Контейнер для фиксации ошибок.
 * \param[in] isClass Флаг сущности: \c true — проверяется имя класса, \c false — имя свойства.
 * \return \c true, если имя прошло все проверки, иначе \c false.
 */
bool checkStringCharacters(const QString& name, const QString& contextError, QSet<Error>& errors, bool isClass) {
    // Анализ соответствия длины переданной строки (255 знаков)
    if (name.isEmpty() || name.length() > 255) {
        ErrorType lengthError = isClass ? ErrorType::invalidClassNameLength
                                        : ErrorType::invalidPropertyNameLength;
        errors.insert(Error(lengthError, name, "", 0, name.length()));
    }

    // Посимвольный поиск некорректного знака в случае провала регулярного выражения
    if (!isValidName(name)) {
        QString badChar;
        for (QChar ch : name) {
            if (!isValidName(QString(ch))) { badChar = QString(ch); break; }
        }
        errors.insert(Error(ErrorType::invalidCharacters, name, badChar));
        return false; // Вернуть false если обнаружены невалидные символы
    }
    return true;
}

/*!
 * \brief Внутренняя функция валидации правила "value_count".
 * * Проверяет, является ли значение массивом строго из 1 целого числа в диапазоне от 1 до 1000.
 * * \param[in] propObj Текущий JSON-объект свойства.
 * \param[in] propName Имя свойства.
 * \param[out] prop Заполняемая структура свойства.
 * \param[out] errors Контейнер ошибок.
 */
void checkValueCountRule(const QJsonObject& propObj, const QString& propName, Property& prop, QSet<Error>& errors) {
    // Пропуск этапа, если поле фиксации количества элементов не объявлено
    if (!propObj.contains("value_count")) return;

    // Валидация строгого соответствия: массив из одного числового элемента
    QJsonValue vcVal = propObj.value("value_count");
    if (!vcVal.isArray() || vcVal.toArray().size() != 1 || !vcVal.toArray().at(0).isDouble()) {
        errors.insert(Error(ErrorType::invalidValueCountType, propName));
        return;
    }

    // Проверка извлеченного числа на целочисленность и лимиты диапазона
    double dVal = vcVal.toArray().at(0).toDouble();
    int val = vcVal.toArray().at(0).toInt();
    if (dVal != val || val < 1 || val > 1000) {
        errors.insert(Error(ErrorType::invalidValueRange, propName, "", val));
    } else {
        prop.valueCount.append(val); // Сохранение счетчика и обновление метатипа правила
        prop.ruleType = PropertyRuleType::HasPropertyWithCount;
    }
}

/*!
 * \brief Внутренняя функция валидации правила "expected_value".
 * * Проверяет, является ли значение непустым целочисленным массивом (размер до 100 элементов),
 * где каждое число находится в диапазоне от 1 до 1000.
 * * \param[in] propObj Текущий JSON-объект свойства.
 * \param[in] propName Имя свойства.
 * \param[out] prop Заполняемая структура свойства.
 * \param[out] errors Контейнер ошибок.
 */
void checkExpectedValueRule(const QJsonObject& propObj, const QString& propName, Property& prop, QSet<Error>& errors) {
    // Выход из процедуры, если ограничительное правило ожидаемых значений отсутствует
    if (!propObj.contains("expected_value")) return;

    // Контроль типа данных и структуры массива ожидаемых значений
    QJsonValue evVal = propObj.value("expected_value");
    if (!evVal.isArray()) {
        errors.insert(Error(ErrorType::invalidValueType, propName));
        return;
    }

    // Валидация количественных ограничений на размерность списка значений
    QJsonArray evArray = evVal.toArray();
    if (evArray.isEmpty()) {
        errors.insert(Error(ErrorType::emptyExpectedValue, propName));
        return;
    }
    if (evArray.size() > 100) {
        errors.insert(Error(ErrorType::tooManyExpectedValues, propName, "", 0, evArray.size()));
        return;
    }

    // Итерационный контроль типов элементов и попадания чисел в диапазон [1, 1000]
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
        prop.expectedValues.append(val); // Добавление валидного целочисленного значения
    }
    // Переключение финального типа правила в зависимости от размерности множества
    if (validItems) {
        prop.ruleType = (evArray.size() == 1) ? PropertyRuleType::HasPropertyWithValue
                                              : PropertyRuleType::HasPropertyWithValues;
    }
}

/*!
 * \brief Внутренняя функция валидации отдельного JSON-объекта свойства.
 * * Проверяет отсутствие лишних полей, наличие обязательного поля "name", строковый тип имени,
 * а также выявляет логическую неопределенность (когда заданы и "value_count", и "expected_value").
 * * \param[in] propObj JSON-объект свойства для проверки.
 * \param[in] index Порядковый индекс свойства в родительском массиве.
 * \param[in] className Имя класса, содержащего данное свойство.
 * \param[out] errors Ссылка на набор ошибок.
 * \param[out] prop Выходная структура для сохранения извлеченных данных.
 * \return \c true, если структура свойства валидна, иначе \c false.
 */
bool validatePropertyObject(const QJsonObject& propObj, int index, const QString& className, QSet<Error>& errors, Property& prop) {
    // Проверить наличие лишних полей
    for (const QString& key : propObj.keys()) {
        if (key != "name" && key != "value_count" && key != "expected_value") {
            errors.insert(Error(ErrorType::extraField, key));
        }
    }
    // Проверить наличие названия свойства
    if (!propObj.contains("name")) {
        errors.insert(Error(ErrorType::missingPropertyName, className, "", 0, index + 1));
        return false;
    }
    QJsonValue propNameVal = propObj.value("name");
    if (!propNameVal.isString()) {
        errors.insert(Error(ErrorType::invalidCharacters, QString("свойства №%1 в классе %2").arg(index + 1).arg(className)));
        return false;
    }

    // Проверить имя свойства на наличие запрещенных символов
    QString propName = propNameVal.toString();
    checkStringCharacters(propName, className, errors, false);

    // Инициализация базового правила и проверка на противоречивость условий
    prop.name = propName;
    prop.ruleType = PropertyRuleType::HasProperty;

    if (propObj.contains("value_count") && propObj.contains("expected_value")) {
        errors.insert(Error(ErrorType::ambiguousRule, propName));
    }

    // Проверить другие правила специальными функциями
    checkValueCountRule(propObj, propName, prop, errors);
    checkExpectedValueRule(propObj, propName, prop, errors);
    return true;
}

/*!
 * \brief Внутренняя функция структурированной валидации одного объекта класса.
 * * Проверяет наличие обязательных полей "class_name" и массива "properties",
 * контролирует лимиты размеров и запускает валидацию вложенных свойств.
 * * \param[in] classObj JSON-объект класса для проверки.
 * \param[in] index Порядковый индекс класса в корневом массиве документов.
 * \param[out] errors Набор ошибок.
 * \param[out] parsedClasses Накопительный вектор распарсенных классов.
 */
void validateClassStructuredObject(const QJsonObject& classObj, int index, QSet<Error>& errors, QVector<Class>& parsedClasses) {
    // Проверить наличие лишних полей у классов
    for (const QString& key : classObj.keys()) {
        if (key != "class_name" && key != "properties") errors.insert(Error(ErrorType::extraField, key));
    }
    // Проверить наличие названия класса
    if (!classObj.contains("class_name")) {
        errors.insert(Error(ErrorType::missingClassName, "", "", 0, index + 1)); return;
    }
    QJsonValue nameVal = classObj.value("class_name");
    if (!nameVal.isString()) {
        errors.insert(Error(ErrorType::invalidCharacters, QString("класса №%1").arg(index + 1))); return;
    }

    // Лексический анализ и валидация длины имени извлеченного класса
    QString className = nameVal.toString();
    checkStringCharacters(className, QString("класса №%1").arg(index + 1), errors);

    // Валидация наличия и корректности структуры массива свойств
    if (!classObj.contains("properties") || !classObj.value("properties").isArray()) {
        errors.insert(Error(ErrorType::missingProperties, className)); return;
    }

    // Контроль граничных лимитов размера массива свойств
    QJsonArray propArray = classObj.value("properties").toArray();
    if (propArray.isEmpty()) errors.insert(Error(ErrorType::emptyProperties, className));
    if (propArray.size() > 100) errors.insert(Error(ErrorType::tooManyProperties, className, "", 0, propArray.size()));

    // Пообъектный разбор и валидация каждого вложенного правила свойства
    Class cls; cls.name = className;
    for (int j = 0; j < propArray.size(); ++j) {
        if (!propArray.at(j).isObject()) continue;
        Property prop;
        if (validatePropertyObject(propArray.at(j).toObject(), j, className, errors, prop)) {
            cls.properties.append(prop); // Фиксация успешно валидированного свойства
        }
    }
    parsedClasses.append(cls); // Добавить валидный класс в итоговый вектор
}

/*!
 * \brief Точка входа для валидации входного JSON-документа.
 * \param[in] rootObj Корневой объект JSON.
 * \param[out] errors Контейнер для записи обнаруженных несоответствий схеме.
 * \param[out] parsedClasses Контейнер для сохранения извлеченных структур данных.
 * \return \c true, если документ полностью валиден, иначе \c false.
 */
bool validator::validateInput(const QJsonObject& rootObj, QSet<Error>& errors, QVector<Class>& parsedClasses) {
    // Проверить наличие лишних полей на самом верхнем уровне
    for (const QString& key : rootObj.keys()) {
        if (key != "classes") {
            errors.insert(Error(ErrorType::extraField, key));
        }
    }

    // Проверка наличия и корректности типа корневого поля "classes"
    if (!rootObj.contains("classes") || !rootObj.value("classes").isArray()) {
        errors.insert(Error(ErrorType::emptyClassesArray));
        return false;
    }

    QJsonArray classesArray = rootObj.value("classes").toArray();

    // Если массив классов пустой, вернуть ошибку emptyClassesArray
    if (classesArray.isEmpty()) {
        errors.insert(Error(ErrorType::emptyClassesArray));
        return false;
    }

    // Если размер массива больше 100, вернуть ошибку tooManyClasses
    if (classesArray.size() > 100) {
        errors.insert(Error(ErrorType::tooManyClasses, "", "", 0, classesArray.size()));
        return false;
    }

    // Для каждого класса
    for (int i = 0; i < classesArray.size(); ++i) {
        // Проверить структуру класса
        if (classesArray.at(i).isObject()) {
            validateClassStructuredObject(classesArray.at(i).toObject(), i, errors, parsedClasses);
        }
    }

    // Вернуть true - если массив errors пуст и false - если не пуст
    return errors.isEmpty();
}
