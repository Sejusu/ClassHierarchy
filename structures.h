#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QString>
#include <QVector>
#include <QList>
#include <QSet>
#include <QMap>
#include <QHash>

/**
 * @file structures.h
 * @brief Заголовочный файл всех структур
 */

/*!
 * \enum PropertyRuleType
 * \brief Перечисление, определяющее тип строгости правила для свойства.
 *
 * Используется в структурах \ref Property и \ref PropertyRule для указания
 * типа проверки, применяемой к свойству.
 *
 * \see Property
 * \see PropertyRule
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
 *
 * Используется на этапе валидации и парсинга входного JSON-файла.
 * После успешной валидации преобразуется в \ref PropertyRule для использования в графе.
 *
 * \see PropertyRule
 * \see Class
 * \see validator::validateInput
 */
struct Property {
    QString name;                 /*!< Имя свойства (идентификатор). */
    PropertyRuleType ruleType = PropertyRuleType::HasProperty;    /*!< Тип правила валидации свойства.
                                                                       Определяет, какая проверка применяется:
                                                                       - \ref HasProperty - только наличие
                                                                       - \ref HasPropertyWithCount - проверка количества
                                                                       - \ref HasPropertyWithValue - проверка значения
                                                                       - \ref HasPropertyWithValues - проверка массива значений */
    QVector<int> valueCount;      /*!< Ограничение количества: массив, всегда содержащий строго 1 элемент.
                                       Используется при \ref PropertyRuleType::HasPropertyWithCount */
    QVector<int> expectedValues;  /*!< Ожидаемые значения: массив размером от 1 до 100 элементов.
                                       Используется при \ref PropertyRuleType::HasPropertyWithValue и \ref PropertyRuleType::HasPropertyWithValues */
};

/*!
 * \struct Class
 * \brief Структура данных, представляющая описание класса на этапе первичного парсинга.
 *
 * Содержит имя класса и список его свойств. Используется как промежуточный
 * формат данных между валидацией и построением иерархии.
 *
 * \see ClassNode
 * \see ClassHierarchy
 * \see validator::validateInput
 * \see buildClassHierarchy
 */
struct Class {
    QString name;                 /*!< Уникальное имя класса. */
    QVector<Property> properties; /*!< Список правил-свойств, определяющих данный класс. */
};

/*!
 * \enum ErrorType
 * \brief Перечисление всех возможных типов ошибок валидации структуры и синтаксиса входных данных.
 *
 * Используется в классе \ref Error для классификации обнаруженных проблем.
 * Каждому типу соответствует свое сообщение в методе \ref Error::generateErrorMessage.
 *
 * \see Error
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
 *
 * Предоставляет контекстную информацию о месте возникновения ошибки, включая имена свойств,
 * ошибочные значения, индексы и пути к файлам. Поддерживает хеширование для хранения в QSet.
 *
 * Используется в:
 * - \ref validator для накопления ошибок валидации
 * - \ref main для вывода сообщений пользователю
 *
 * \see ErrorType
 * \see validator
 * \see main
 */
class Error {
public:
    ErrorType type;             /*!< Категория/тип произошедшей ошибки. \see ErrorType */
    QString incorrectProperty;  /*!< Контекст: имя класса или свойства, вызвавшего ошибку. */
    QString incorrectChar;      /*!< Контекст: символ, не прошедший валидацию регулярным выражением. */
    int incorrectValue;         /*!< Контекст: числовое значение, вышедшее за пределы диапазона. */
    int incorrectSize;          /*!< Контекст: фактический ошибочный размер структуры или индекс массива. */
    QString filePath;           /*!< Контекст: путь к файлу, в котором обнаружена проблема. */

    /*!
     * \brief Универсальный конструктор для инициализации контекста ошибки.
     * \param[in] t Тип ошибки. \see ErrorType
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
     *
     * Используется для сравнения объектов Error при вставке в контейнер QSet.
     *
     * \param[in] other Другой объект ошибки для сравнения.
     * \return \c true, если все контекстные поля полностью совпадают.
     */
    bool operator==(const Error& other) const {
        return type == other.type
               && incorrectProperty == other.incorrectProperty
               && incorrectChar == other.incorrectChar
               && incorrectValue == other.incorrectValue
               && incorrectSize == other.incorrectSize
               && filePath == other.filePath;
    }

    /*!
     * \brief Глобальная дружественная функция для вычисления хеш-значения объекта Error.
     *
     * Позволяет использовать структуру Error в качестве элементов контейнера QSet.
     *
     * \param[in] key Объект ошибки, для которого считается хеш.
     * \param[in] seed Базовое значение для хеширования.
     * \return Вычисленное значение хэш-функции.
     *
     * \see qHash
     */
    friend size_t qHash(const Error& key, size_t seed) {
        seed = qHash(static_cast<int>(key.type), seed);
        seed = qHash(key.incorrectProperty, seed);
        seed = qHash(key.incorrectChar, seed);
        seed = qHash(key.incorrectValue, seed);
        seed = qHash(key.incorrectSize, seed);
        return qHash(key.filePath, seed);
    }

    struct ErrorFormatRule {
        QString templateStr;
        enum ArgsType { None, Path, Prop, Size, DoublePropSize, DoublePropValue, CustomChars } argsType;
    };

    /*!
     * \brief Формирует понятное человеку, локализованное текстовое сообщение об ошибке.
     *
     * Метод автоматически подставляет контекстные данные структуры (индексы, лимиты, имена)
     * в соответствующий языковой шаблон.
     *
     * Для ошибок типа \ref ErrorType::invalidCharacters генерирует специальное сообщение
     * с указанием недопустимого символа.
     *
     * \return Строка с подробным описанием ошибки.
     *
     * \see ErrorType
     */
    QString generateErrorMessage() const {
        // Статическая таблица правил (заполняется один раз при первом вызове)
        static const QHash<ErrorType, ErrorFormatRule> rules = {
            {ErrorType::inputFileNotExist,          {"Ошибка: Входной файл %1 не существует или недоступен для чтения.", ErrorFormatRule::Path}},
            {ErrorType::outputFileCreateFail,       {"Ошибка: Не удалось создать выходной файл %1. Проверьте права доступа к директории.", ErrorFormatRule::Path}},
            {ErrorType::emptyInputFile,             {"Ошибка: Входной файл %1 пуст.", ErrorFormatRule::Path}},
            {ErrorType::jsonSyntaxError,            {"Ошибка синтаксиса JSON в файле %1. Проверьте корректность структуры, баланс скобок и кавычек.", ErrorFormatRule::Path}},

            {ErrorType::emptyClassesArray,          {"Ошибка валидации: Массив \"classes\" пуст или отсутствует в корне JSON-документа.", ErrorFormatRule::None}},
            {ErrorType::ambiguousRule,              {"Ошибка в свойстве %1: Обнаружены противоречивые условия. Запрещено одновременно указывать \"value_count\" и \"expected_value\".", ErrorFormatRule::Prop}},

            {ErrorType::tooManyClasses,             {"Ошибка валидации: Превышено максимальное количество классов. Получено элементов: %1, установленный лимит: 100.", ErrorFormatRule::Size}},
            {ErrorType::missingClassName,           {"Ошибка валидации: В элементе массива \"classes\" под индексом %1 отсутствует обязательное поле \"class_name\".", ErrorFormatRule::Size}},

            {ErrorType::missingProperties,          {"Ошибка в классе %1: Отсутствует обязательное поле \"properties\" (массив свойств).", ErrorFormatRule::Prop}},
            {ErrorType::emptyProperties,            {"Ошибка в классе %1: Массив \"properties\" пуст. Каждый класс должен содержать хотя бы одно свойство.", ErrorFormatRule::Prop}},
            {ErrorType::invalidValueCountType,      {"Ошибка в свойстве %1: Поле \"value_count\" должно быть массивом, содержащим строго одно целое число.", ErrorFormatRule::Prop}},
            {ErrorType::invalidValueType,           {"Ошибка в свойстве %1: Поле \"expected_value\" должно быть массивом, состоящим исключительно из целых чисел.", ErrorFormatRule::Prop}},
            {ErrorType::emptyExpectedValue,         {"Ошибка в свойстве %1: Массив ограничений \"expected_value\" присутствует, но не содержит элементов.", ErrorFormatRule::Prop}},
            {ErrorType::extraField,                 {"Ошибка валидации: Обнаружено лишнее поле %1.", ErrorFormatRule::Prop}},

            {ErrorType::invalidClassNameLength,     {"Ошибка в классе %1: Недопустимая длина имени класса. Ожидается от 1 до 255 символов (фактическая длина: %2).", ErrorFormatRule::DoublePropSize}},
            {ErrorType::tooManyProperties,          {"Ошибка в классе %1: Слишком много свойств в массиве \"properties\". Получено: %2, лимит: 100.", ErrorFormatRule::DoublePropSize}},
            {ErrorType::missingPropertyName,        {"Ошибка в классе %1: У свойства под индексом %2 in массиве \"properties\" отсутствует обязательное поле \"name\".", ErrorFormatRule::DoublePropSize}},
            {ErrorType::invalidPropertyNameLength,  {"Ошибка: Обнаружено свойство с недопустимой длиной имени (длина: %1 знаков). Проверьте свойства в объекте %2.", ErrorFormatRule::DoublePropSize}}, // Порядок будет обработан ниже
            {ErrorType::tooManyExpectedValues,      {"Ошибка в свойстве %1: Размер массива \"expected_value\" составляет %2 элементов, что превышает лимит [1, 100].", ErrorFormatRule::DoublePropSize}},

            {ErrorType::invalidValueRange,          {"Ошибка в свойстве %1: Значение (%2) выходит за пределы допустимого диапазона [1, 1000].", ErrorFormatRule::DoublePropValue}},
            {ErrorType::invalidCharacters,          {"", ErrorFormatRule::CustomChars}} // Специфическая логика
        };

        // Точка ветвления 1: поиск правила в хэш-таблице (if/сокращенный поиск)
        ErrorFormatRule rule = rules.value(type, {"Неизвестная критическая ошибка валидации структуры входных данных.", ErrorFormatRule::None});

        QString propText = incorrectProperty.isEmpty() ? "(имя не указано)" : QString("'%1'").arg(incorrectProperty);
        QString pathText = filePath.isEmpty() ? "указанный файл" : QString("'%1'").arg(filePath);

        // Точка ветвления 2: Обработка CustomChars (invalidCharacters) и всех остальных через switch по типам аргументов
        switch (rule.argsType) {
        case ErrorFormatRule::Path:              return rule.templateStr.arg(pathText);
        case ErrorFormatRule::Prop:              return rule.templateStr.arg(propText);
        case ErrorFormatRule::Size:              return rule.templateStr.arg(incorrectSize);
        case ErrorFormatRule::DoublePropValue:   return rule.templateStr.arg(propText).arg(incorrectValue);

        case ErrorFormatRule::DoublePropSize:
            // Универсальный разворот для invalidPropertyNameLength, где размер идет ПЕРВЫМ
            return (type == ErrorType::invalidPropertyNameLength)
                       ? rule.templateStr.arg(incorrectSize).arg(propText)
                       : rule.templateStr.arg(propText).arg(incorrectSize);

        case ErrorFormatRule::CustomChars:
            return incorrectChar.isEmpty()
                       ? QString("Ошибка: Поле наименования в объекте %1 должно быть строковым типом, а не числом/массивом/null.").arg(propText)
                       : QString("Ошибка: Наименование %1 содержит недопустимый символ '%2'. Разрешены только буквы (рус/англ), цифры, подчёркивание '_' и дефис '-'.").arg(propText).arg(incorrectChar);

        default: return rule.templateStr;
        }
    }
};

/*!
 * \class PropertyRule
 * \brief Представление правила-свойства, адаптированное под эффективную работу в графе.
 *
 * В отличие от первичной структуры \ref Property, использует контейнеры \c QList, оптимизированные
 * для частых операций чтения/сравнения при сопоставлении вершин графа.
 *
 * Используется в:
 * - \ref ClassNode - как множество свойств узла графа
 * - \ref isSubset - для сравнения свойств классов
 * - \ref matchProperty - для проверки соответствия свойств
 *
 * \see Property
 * \see ClassNode
 * \see matchProperty
 * \see isSubset
 */
class PropertyRule {
public:
    QString name;                 /*!< Имя свойства. */
    PropertyRuleType ruleType = PropertyRuleType::HasProperty;    /*!< Тип логического правила.
                                                                       \see PropertyRuleType */
    QList<int> valueCount;        /*!< Ограничение на количество значений.
                                       Используется при \ref PropertyRuleType::HasPropertyWithCount */
    QList<int> expectedValues;    /*!< Список допустимых значений свойства.
                                       Используется при \ref PropertyRuleType::HasPropertyWithValue и \ref PropertyRuleType::HasPropertyWithValues */

    /*!
     * \brief Оператор поэлементного сравнения двух правил.
     *
     * Сравнивает все поля: имя, тип правила, valueCount и expectedValues.
     *
     * \param[in] other Другое правило для сравнения.
     * \return \c true, если все поля совпадают.
     */
    bool operator==(const PropertyRule& other) const {
        return name == other.name && ruleType == other.ruleType && valueCount == other.valueCount && expectedValues == other.expectedValues;
    }

    /*!
     * \brief Функция хеширования правила для оптимизации поиска внутри QSet.
     *
     * Используется для хранения уникальных правил в \ref ClassNode::properties.
     *
     * \param[in] key Правило для хеширования.
     * \param[in] seed Базовое значение для хеширования.
     * \return Вычисленное хеш-значение.
     *
     * \see qHash
     */
    friend size_t qHash(const PropertyRule& key, size_t seed) {
        return qHash(key.name, seed) ^ qHash(static_cast<int>(key.ruleType), seed);
    }
};

/*!
 * \class ClassNode
 * \brief Класс узла графа, представляющий отдельный класс и уникальное множество его правил.
 *
 * Используется в \ref ClassHierarchy для построения графа иерархии классов.
 * Содержит имя класса и набор свойств, определяющих его.
 *
 * \see Class
 * \see ClassHierarchy
 * \see buildClassHierarchy
 * \see removeTransitiveEdges
 */
class ClassNode {
public:
    QString className;             /*!< Имя класса (уникальный идентификатор узла). */
    QSet<PropertyRule> properties; /*!< Множество правил, сопоставленных данному классу.
                                        Используется в \ref isSubset для проверки вхождения. */

    /*!
     * \brief Конструктор узла с установкой имени класса.
     * \param[in] name Строковое имя класса.
     */
    ClassNode(const QString& name) : className(name) {}
};

/*!
 * \class ClassHierarchy
 * \brief Класс, инкапсулирующий структуру ориентированного графа иерархии.
 *
 * Содержит в себе набор динамически выделенных узлов классов и таблицу смежности
 * для хранения направленных ребер зависимостей. Гарантирует очистку памяти в деструкторе.
 *
 * Основные операции:
 * - \ref buildClassHierarchy - построение начального графа
 * - \ref removeTransitiveEdges - оптимизация графа
 * - \ref hierarchy::generateDot - экспорт в DOT-формат
 *
 * \see ClassNode
 * \see buildClassHierarchy
 * \see removeTransitiveEdges
 * \see hierarchy::generateDot
 */
class ClassHierarchy {
public:
    QSet<ClassNode*> classes;          /*!< Множество указателей на все вершины (узлы) графа.
                                            Каждый узел содержит имя класса и его свойства. */
    QMap<QString, QList<QString>> edges; /*!< Таблица смежности: Имя_Родителя -> Список_Имен_Потомков.
                                              Используется для хранения направленных ребер графа. */

    /*!
     * \brief Деструктор класса ClassHierarchy.
     *
     * Автоматически освобождает динамическую память, выделенную под объекты ClassNode в куче.
     * Использует qDeleteAll для безопасного удаления всех узлов.
     *
     * \see qDeleteAll
     */
    ~ClassHierarchy() {
        qDeleteAll(classes);
    }
};

#endif // STRUCTURES_H
