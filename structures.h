#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QString>
#include <QVector>
#include <QList>
#include <QSet>
#include <QMap>
#include <QHash>

enum class PropertyRuleType {
    HasProperty,
    HasPropertyWithCount,
    HasPropertyWithValue,
    HasPropertyWithValues
};

// Структуры для первоначального парсинга данных
struct Property {
    QString name;
    PropertyRuleType ruleType;
    QVector<int> valueCount;     // Всегда массив (размер 1)
    QVector<int> expectedValues; // Всегда массив (размер от 1 до 100)
};

struct Class {
    QString name;
    QVector<Property> properties;
};

// Типы возможных ошибок
enum class ErrorType {
    noError, inputFileNotExist, outputFileCreateFail, emptyInputFile, jsonSyntaxError,
    emptyClassesArray, tooManyClasses, missingClassName, missingProperties, emptyProperties,
    missingPropertyName, tooManyProperties, invalidValueType, invalidValueRange, invalidValueCountType,
    emptyExpectedValue, tooManyExpectedValues, invalidClassNameLength, invalidPropertyNameLength,
    invalidCharacters, ambiguousRule, extraField
};

// Класс для обработки и генерации подробных сообщений об ошибках
class Error {
public:
    ErrorType type;
    QString incorrectProperty; // Имя класса или свойства, где произошла ошибка
    QString incorrectChar;     // Конкретный недопустимый символ
    int incorrectValue;        // Ошибочное числовое значение
    int incorrectSize;         // Полученный размер/длина для вывода ограничений
    QString filePath;          // Путь к файлу

    // Универсальный конструктор для полной инициализации контекста ошибки
    Error(ErrorType t, const QString& prop = "", const QString& ch = "", int val = 0, int size = 0, const QString& path = "")
        : type(t), incorrectProperty(prop), incorrectChar(ch), incorrectValue(val), incorrectSize(size), filePath(path) {}

    // Оператор сравнения (учитывает контекст, чтобы в QSet попадали уникальные ошибки для разных классов/свойств)
    bool operator==(const Error& other) const {
        return type == other.type &&
               incorrectProperty == other.incorrectProperty &&
               incorrectChar == other.incorrectChar &&
               incorrectValue == other.incorrectValue &&
               incorrectSize == other.incorrectSize &&
               filePath == other.filePath;
    }

    // Хеш-функция для QSet
    friend size_t qHash(const Error& key, size_t seed = 0) {
        return qHash(static_cast<int>(key.type), seed) ^
               qHash(key.incorrectProperty, seed) ^
               qHash(key.incorrectChar, seed) ^
               qHash(key.incorrectValue, seed);
    }

    // Метод генерации информативного сообщения с указанием места и сути ошибки
    QString generateErrorMessage() const {
        QString propText = incorrectProperty.isEmpty() ? "(имя не указано)" : QString("'%1'").arg(incorrectProperty);
        QString pathText = filePath.isEmpty() ? "указанный файл" : QString("'%1'").arg(filePath);

        switch(type) {
        case ErrorType::inputFileNotExist:
            return QString("Ошибка: Входной файл %1 не существует или недоступен для чтения.").arg(pathText);
        case ErrorType::outputFileCreateFail:
            return QString("Ошибка: Не удалось создать выходной файл %1. Проверьте права доступа к директории.").arg(pathText);
        case ErrorType::emptyInputFile:
            return QString("Ошибка: Входной файл %1 пуст.").arg(pathText);
        case ErrorType::jsonSyntaxError:
            return QString("Ошибка синтаксиса JSON в файле %1. Проверьте корректность структуры, баланс скобок и кавычек.").arg(pathText);

        case ErrorType::emptyClassesArray:
            return "Ошибка валидации: Массив \"classes\" пуст или отсутствует в корне JSON-документа.";
        case ErrorType::tooManyClasses:
            return QString("Ошибка валидации: Превышено максимальное количество классов. Получено элементов: %1, установленный лимит: 100.").arg(incorrectSize);

        case ErrorType::missingClassName:
            return QString("Ошибка валидации: В элементе массива \"classes\" под индексом %1 отсутствует обязательное поле \"class_name\".")
                .arg(incorrectSize > 0 ? QString::number(incorrectSize) : "(неизвестный индекс)");

        case ErrorType::invalidCharacters: {
            if (incorrectChar.isEmpty()) {
                return QString("Ошибка: Поле наименования в объекте %1 должно быть строковым типом, а не числом/массивом/null.").arg(propText);
            }
            return QString("Ошибка: Наименование %1 содержит недопустимый символ '%2'. Разрешены только буквы (рус/англ), цифры, подчёркивание '_' и дефис '-'.").arg(propText).arg(incorrectChar);
        }

        case ErrorType::invalidClassNameLength:
            return QString("Ошибка в классе %1: Недопустимая длина имени класса. Ожидается от 1 до 255 символов (фактическая длина: %2).").arg(propText).arg(incorrectSize);

        case ErrorType::missingProperties:
            return QString("Ошибка в классе %1: Отсутствует обязательное поле \"properties\" (массив свойств).").arg(propText);
        case ErrorType::emptyProperties:
            return QString("Ошибка в классе %1: Массив \"properties\" пуст. Каждый класс должен содержать хотя бы одно свойство.").arg(propText);
        case ErrorType::tooManyProperties:
            return QString("Ошибка в классе %1: Слишком много свойств в массиве \"properties\". Получено: %2, лимит: 100.").arg(propText).arg(incorrectSize);

        case ErrorType::missingPropertyName:
            return QString("Ошибка в классе %1: У свойства под индексом %2 в массиве \"properties\" отсутствует обязательное поле \"name\".")
                .arg(propText).arg(incorrectSize > 0 ? QString::number(incorrectSize) : "(неизвестный индекс)");

        case ErrorType::invalidPropertyNameLength:
            return QString("Ошибка: Обнаружено свойство с недопустимой длиной имени (длина: %1 знаков). Проверьте свойства в объекте %2.")
                .arg(incorrectSize).arg(propText);

        case ErrorType::invalidValueCountType:
            return QString("Ошибка в свойстве %1: Поле \"value_count\" должно быть массивом, содержащим строго одно целое число.").arg(propText);
        case ErrorType::invalidValueType:
            return QString("Ошибка в свойстве %1: Поле \"expected_value\" должно быть массивом, состоящим исключительно из целых чисел.").arg(propText);

        case ErrorType::invalidValueRange:
            return QString("Ошибка в свойстве %1: Значение (%2) выходит за пределы допустимого диапазона [1, 1000].")
                .arg(propText).arg(incorrectValue);

        case ErrorType::emptyExpectedValue:
            return QString("Ошибка в свойстве %1: Массив ограничений \"expected_value\" присутствует, но не содержит элементов.").arg(propText);
        case ErrorType::tooManyExpectedValues:
            return QString("Ошибка в свойстве %1: Размер массива \"expected_value\" составляет %2 элементов, что превышает лимит [1, 100].").arg(propText).arg(incorrectSize);

        case ErrorType::ambiguousRule:
            return QString("Ошибка в свойстве %1: Обнаружены противоречивые условия. Запрещено одновременно указывать \"value_count\" и \"expected_value\".").arg(propText);

        case ErrorType::extraField:
            return QString("Ошибка валидации: Обнаружено лишнее поле %1.").arg(propText);

        default:
            return "Неизвестная критическая ошибка валидации структуры входных данных.";
        }
    }
};

// Представление правил для работы с графом
class PropertyRule {
public:
    QString name;
    PropertyRuleType ruleType;
    QList<int> valueCount;
    QList<int> expectedValues;

    bool operator==(const PropertyRule& other) const {
        return name == other.name && ruleType == other.ruleType && valueCount == other.valueCount && expectedValues == other.expectedValues;
    }

    friend size_t qHash(const PropertyRule& key, size_t seed = 0) {
        return qHash(key.name, seed) ^ qHash(static_cast<int>(key.ruleType), seed);
    }
};

class ClassNode {
public:
    QString className;
    QSet<PropertyRule> properties;

    ClassNode(const QString& name) : className(name) {}
};

class ClassHierarchy {
public:
    QSet<ClassNode*> classes;
    QMap<QString, QList<QString>> edges;

    ~ClassHierarchy() {
        qDeleteAll(classes);
    }
};

#endif // STRUCTURES_H
