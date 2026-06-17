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

        static const QMap<ErrorType, QString> errorTemplates = {
            {ErrorType::inputFileNotExist,          "Ошибка: Входной файл %1 не существует или недоступен для чтения."},
            {ErrorType::outputFileCreateFail,       "Ошибка: Не удалось создать выходной файл %1. Проверьте права доступа к директории."},
            {ErrorType::emptyInputFile,             "Ошибка: Входной файл %1 пуст."},
            {ErrorType::jsonSyntaxError,            "Ошибка синтаксиса JSON в файле %1. Проверьте корректность структуры, баланс скобок и кавычек."},
            {ErrorType::emptyClassesArray,          "Ошибка валидации: Массив \"classes\" пуст или отсутствует в корне JSON-документа."},
            {ErrorType::tooManyClasses,             "Ошибка валидации: Превышено максимальное количество классов. Получено элементов: %1, установленный лимит: 100."},
            {ErrorType::missingClassName,           "Ошибка валидации: В элементе массива \"classes\" под индексом %1 отсутствует обязательное поле \"class_name\"."},
            {ErrorType::invalidClassNameLength,     "Ошибка в классе %1: Недопустимая длина имени класса. Ожидается от 1 до 255 символов (фактическая длина: %2)."},
            {ErrorType::missingProperties,          "Ошибка в классе %1: Отсутствует обязательное поле \"properties\" (массив свойств)."},
            {ErrorType::emptyProperties,            "Ошибка в классе %1: Массив \"properties\" пуст. Каждый класс должен содержать хотя бы одно свойство."},
            {ErrorType::tooManyProperties,          "Ошибка в классе %1: Слишком много свойств в массиве \"properties\". Получено: %2, лимит: 100."},
            {ErrorType::missingPropertyName,        "Ошибка в классе %1: У свойства под индексом %2 в массиве \"properties\" отсутствует обязательное поле \"name\"."},
            {ErrorType::invalidPropertyNameLength,  "Ошибка: Обнаружено свойство с недопустимой длиной имени (длина: %1 знаков). Проверьте свойства в объекте %2."},
            {ErrorType::invalidValueCountType,      "Ошибка в свойстве %1: Поле \"value_count\" должно быть массивом, содержащим строго одно целое число."},
            {ErrorType::invalidValueType,           "Ошибка в свойстве %1: Поле \"expected_value\" должно быть массивом, состоящим исключительно из целых чисел."},
            {ErrorType::invalidValueRange,          "Ошибка в свойстве %1: Значение (%2) выходит за пределы допустимого диапазона [1, 1000]."},
            {ErrorType::emptyExpectedValue,         "Ошибка в свойстве %1: Массив ограничений \"expected_value\" присутствует, но не содержит элементов."},
            {ErrorType::tooManyExpectedValues,      "Ошибка в свойстве %1: Размер массива \"expected_value\" составляет %2 элементов, что превышает лимит [1, 100]."},
            {ErrorType::ambiguousRule,              "Ошибка в свойстве %1: Обнаружены противоречивые условия. Запрещено одновременно указывать \"value_count\" и \"expected_value\"."},
            {ErrorType::extraField,                 "Ошибка валидации: Обнаружено лишнее поле %1."}
        };

        if (type == ErrorType::invalidCharacters) {
            if (incorrectChar.isEmpty()) {
                return QString("Ошибка: Поле наименования в объекте %1 должно быть строковым типом, а не числом/массивом/null.").arg(propText);
            }
            return QString("Ошибка: Наименование %1 содержит недопустимый символ '%2'. Разрешены только буквы (рус/англ), цифры, подчёркивание '_' и дефис '-'.").arg(propText).arg(incorrectChar);
        }

        QString tmpl = errorTemplates.value(type, "Неизвестная критическая ошибка валидации структуры входных данных.");
        QString indexStr = incorrectSize > 0 ? QString::number(incorrectSize) : "(неизвестный индекс)";

        return tmpl.arg(pathText)
            .arg(propText)
            .arg(indexStr)
            .arg(incorrectValue);
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
