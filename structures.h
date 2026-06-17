#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QString>
#include <QVector>
#include <QList>
#include <QSet>
#include <QMap>
#include <QHash>

/*!
 * \enum PropertyRuleType
 * \brief Перечисление, определяющее тип строгости правила для свойства.
 */
enum class PropertyRuleType {
    HasProperty,           /*!< Просто наличие свойства с любым значением. */
    HasPropertyWithCount,  /*!< Наличие свойства со строгим ограничением на количество элементов. */
    HasPropertyWithValue,  /*!< Наличие свойства, содержащего конкретное одиночное значение. */
    HasPropertyWithValues  /*!< Наличие свойства, содержащего определенный массив значений. */
};

/*!
 * \struct Property
 * \brief Структура данных для хранения информации о свойстве, полученном при первичном парсинге JSON.
 */
struct Property {
    QString name;                 /*!< Имя свойства (идентификатор). */
    PropertyRuleType ruleType;    /*!< Тип правила валидации свойства. */
    QVector<int> valueCount;      /*!< Ограничение количества: массив, всегда содержащий строго 1 элемент. */
    QVector<int> expectedValues;  /*!< Ожидаемые значения: массив размером от 1 до 100 элементов. */
};

/*!
 * \struct Class
 * \brief Структура данных, представляющая описание класса на этапе первичного парсинга.
 */
struct Class {
    QString name;                 /*!< Уникальное имя класса. */
    QVector<Property> properties; /*!< Список правил-свойств, определяющих данный класс. */
};

/*!
 * \enum ErrorType
 * \brief Перечисление всех возможных типов ошибок валидации структуры и синтаксиса входных данных.
 */
enum class ErrorType {
    noError,                     /*!< Ошибки отсутствуют. */
    inputFileNotExist,           /*!< Входной файл не найден по указанному пути. */
    outputFileCreateFail,        /*!< Не удалось создать результирующий файл на диске. */
    emptyInputFile,              /*!< Входной файл имеет нулевой размер. */
    jsonSyntaxError,             /*!< Нарушена валидность синтаксиса JSON-пакета. */
    emptyClassesArray,           /*!< Корневой массив "classes" пуст или отсутствует. */
    tooManyClasses,              /*!< Количество классов превышает лимит в 100 элементов. */
    missingClassName,            /*!< У объекта класса отсутствует поле "class_name". */
    missingProperties,           /*!< У объекта класса отсутствует обязательный массив "properties". */
    emptyProperties,             /*!< Массив "properties" присутствует, но не содержит элементов. */
    missingPropertyName,         /*!< У конкретного свойства отсутствует обязательное поле "name". */
    tooManyProperties,           /*!< Количество свойств в одном классе превышает лимит в 100 элементов. */
    invalidValueType,            /*!< Поле "expected_value" содержит нецелочисленные типы данных. */
    invalidValueRange,           /*!< Числовое значение выходит за рамки допустимого диапазона [1, 1000]. */
    invalidValueCountType,       /*!< Поле "value_count" не является массивом из одного целого числа. */
    emptyExpectedValue,          /*!< Массив ограничений "expected_value" не содержит элементов. */
    tooManyExpectedValues,       /*!< Размер массива "expected_value" превысил лимит в 100 элементов. */
    invalidClassNameLength,      /*!< Длина имени класса выходит за рамки ограничений (1-255 символов). */
    invalidPropertyNameLength,   /*!< Длина имени свойства некорректна. */
    invalidCharacters,           /*!< Имя содержит недопустимые символы (разрешены: буквы, цифры, '_', '-'). */
    ambiguousRule,               /*!< Противоречие: одновременно заданы "value_count" и "expected_value". */
    extraField                   /*!< Обнаружено неизвестное/лишнее поле на любом из уровней структуры. */
};

/*!
 * \class Error
 * \brief Класс-контейнер для обработки, уникализации и генерации детальных сообщений об ошибках.
 * * Предоставляет контекстную информацию о месте возникновения ошибки, включая имена свойств,
 * ошибочные значения, индексы и пути к файлам. Поддерживает хеширование для хранения в QSet.
 */
class Error {
public:
    ErrorType type;             /*!< Категория/тип произошедшей ошибки. */
    QString incorrectProperty;  /*!< Контекст: имя класса или свойства, вызвавшего ошибку. */
    QString incorrectChar;      /*!< Контекст: символ, не прошедший валидацию регулярным выражением. */
    int incorrectValue;         /*!< Контекст: числовое значение, вышедшее за пределы диапазона. */
    int incorrectSize;          /*!< Контекст: фактический ошибочный размер структуры или индекс массива. */
    QString filePath;           /*!< Контекст: путь к файлу, в котором обнаружена проблема. */

    /*!
     * \brief Универсальный конструктор для инициализации контекста ошибки.
     * \param[in] t Тип ошибки.
     * \param[in] prop Имя ошибочного класса/свойства (опционально).
     * \param[in] ch Недопустимый символ (опционально).
     * \param[in] val Ошибочное числовое значение (опционально).
     * \param[in] size Фактическая длина или индекс (опционально).
     * \param[in] path Путь к обрабатываемому файлу (опционально).
     */
    Error(ErrorType t, const QString& prop = "", const QString& ch = "", int val = 0, int size = 0, const QString& path = "")
        : type(t), incorrectProperty(prop), incorrectChar(ch), incorrectValue(val), incorrectSize(size), filePath(path) {}

    /*!
     * \brief Оператор равенства для обеспечения уникальности ошибок в QSet.
     * \param[in] other Другой объект ошибки для сравнения.
     * \return \c true, если все контекстные поля полностью совпадают.
     */
    bool operator==(const Error& other) const {
        return type == other.type &&
               incorrectProperty == other.incorrectProperty &&
               incorrectChar == other.incorrectChar &&
               incorrectValue == other.incorrectValue &&
               incorrectSize == other.incorrectSize &&
               filePath == other.filePath;
    }

    /*!
     * \brief Глобальная дружественная функция для вычисления хеш-значения объекта Error.
     * Позволяет использовать структуру Error в качестве элементов контейнера QSet.
     * \param[in] key Объект ошибки, для которого считается хеш.
     * \param[in] seed Базовое значение для хеширования.
     * \return Вычисленное значение хэш-функции.
     */
    friend size_t qHash(const Error& key, size_t seed) {
        return qHash(static_cast<int>(key.type), seed) ^
               qHash(key.incorrectProperty, seed) ^
               qHash(key.incorrectChar, seed) ^
               qHash(key.incorrectValue, seed);
    }

    /*!
     * \brief Формирует понятное человеку, локализованное текстовое сообщение об ошибке.
     * * Метод автоматически подставляет контекстные данные структуры (индексы, лимиты, имена)
     * в соответствующий языковой шаблон.
     * * \return Строка с подробным описанием ошибки.
     */
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

/*!
 * \class PropertyRule
 * \brief Представление правила-свойства, адаптированное под эффективную работу в графе.
 * * В отличие от первичной структуры \c Property, использует контейнеры \c QList, оптимизированные
 * для частых операций чтения/сравнения при сопоставлении вершин графа.
 */
class PropertyRule {
public:
    QString name;                 /*!< Имя свойства. */
    PropertyRuleType ruleType;    /*!< Тип логического правила. */
    QList<int> valueCount;        /*!< Ограничение на количество значений. */
    QList<int> expectedValues;    /*!< Список допустимых значений свойства. */

    /*!
     * \brief Оператор поэлементного сравнения двух правил.
     */
    bool operator==(const PropertyRule& other) const {
        return name == other.name && ruleType == other.ruleType && valueCount == other.valueCount && expectedValues == other.expectedValues;
    }

    /*!
     * \brief Функция хеширования правила для оптимизации поиска внутри QSet.
     */
    friend size_t qHash(const PropertyRule& key, size_t seed) {
        return qHash(key.name, seed) ^ qHash(static_cast<int>(key.ruleType), seed);
    }
};

/*!
 * \class ClassNode
 * \brief Класс узла графа, представляющий отдельный класс и уникальное множество его правил.
 */
class ClassNode {
public:
    QString className;             /*!< Имя класса (уникальный идентификатор узла). */
    QSet<PropertyRule> properties; /*!< Множество правил, сопоставленных данному классу. */

    /*!
     * \brief Конструктор узла с установкой имени класса.
     * \param[in] name Строковое имя класса.
     */
    ClassNode(const QString& name) : className(name) {}
};

/*!
 * \class ClassHierarchy
 * \brief Класс, инкапсулирующий структуру ориентированного графа иерархии.
 * * Содержит в себе набор динамически выделенных узлов классов и таблицу смежности
 * для хранения направленных ребер зависимостей. Гарантирует очистку памяти в деструкторе.
 */
class ClassHierarchy {
public:
    QSet<ClassNode*> classes;          /*!< Множество указателей на все вершины (узлы) графа. */
    QMap<QString, QList<QString>> edges; /*!< Таблица смежности: Имя_Родителя -> Список_Имен_Потомков. */

    /*!
     * \brief Деструктор класса ClassHierarchy.
     * Автоматически освобождает динамическую память, выделенную под объекты ClassNode в куче.
     */
    ~ClassHierarchy() {
        qDeleteAll(classes);
    }
};

#endif // STRUCTURES_H
