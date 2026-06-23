#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QJsonObject>
#include <QJsonArray>
#include <QSet>
#include "structures.h"

/**
 * @file validator.h
 * @brief Заголовочный файл валидатора
 */

/*!
 *
 * \class validator
 * \brief Класс для комплексной проверки входных JSON-структур.
 *
 * Осуществляет синтаксическую и семантическую верификацию полей документа, проверяет
 * длины строк, диапазоны чисел, присутствие обязательных полей, отсутствие дубликатов
 * и недопустимых символов. Наполняет контейнер ошибок в случае обнаружения несоответствий.
 *
 * Основной метод:
 * - ::validateInput - выполняет полную валидацию JSON-объекта
 *
 * Вспомогательные функции (объявлены в глобальной области):
 * - ::isValidName - проверка имени по регулярному выражению
 * - ::checkStringCharacters - комплексная проверка строк
 * - ::checkValueCountRule - валидация правила value_count
 * - ::checkExpectedValueRule - валидация правила expected_value
 * - ::validatePropertyObject - валидация отдельного свойства
 * - ::validateClassStructuredObject - валидация класса
 *
 * \sa Error
 * \sa Property
 * \sa Class
 * \sa hierarchy
 */
class validator
{
public:
    /*!
     * \brief Конструктор по умолчанию класса validator.
     */
    validator();

    /*!
     * \brief Выполняет полную валидацию переданного JSON-объекта.
     *
     * Метод каскадно проверяет структуру. Если ошибок не обнаружено, извлекает
     * данные и формирует список объектов Class для дальнейшего построения иерархии.
     *
     * Алгоритм работы:
     * 1. Проверяет наличие и тип поля "classes"
     * 2. Проверяет ограничения на количество классов (максимум 100)
     * 3. Для каждого класса вызывает ::validateClassStructuredObject
     * 4. Заполняет parsedClasses при успешной валидации
     *
     * \param[in] json Корневой объект JSON, считанный из файла конфигурации.
     * \param[out] errors Контейнер (множество), куда записываются все обнаруженные в ходе проверки ошибки.
     * \param[out] parsedClasses Массив структур, заполняемый валидными данными при успешной проверке.
     * \return \c true, если JSON полностью соответствует спецификации и не содержит ошибок, иначе \c false.
     *
     * \sa ::validateClassStructuredObject
     * \sa Error
     * \sa Class
     */
    bool validateInput(const QJsonObject& json, QSet<Error>& errors, QVector<Class>& parsedClasses);
};

/*!
 * \fn bool isValidName(const QString& name)
 * \brief Внутренняя функция для проверки соответствия имени регулярному выражению.
 *
 * Допускает использование букв латиницы и кириллицы (включая ё/Ё), цифр,
 * знаков подчёркивания и дефисов.
 *
 * Регулярное выражение: `^[A-Za-z0-9_А-Яа-яёЁ\\-]+$`
 *
 * \param[in] name Проверяемая строка.
 * \return \c true, если строка содержит только валидные символы, иначе \c false.
 *
 * \sa ::checkStringCharacters
 * \sa ErrorType::invalidCharacters
 */
static bool isValidName(const QString& name);

/*!
 * \fn bool checkStringCharacters(const QString& name, const QString& contextError, QSet<Error>& errors, bool isClass)
 * \brief Внутренняя функция комплексной проверки строк на ограничения длины и символьный состав.
 *
 * Выявляет недопустимые символы, посимвольно локализует первый некорректный знак
 * для вывода подробного контекста ошибки.
 *
 * Выполняет проверки:
 * 1. Длина строки (1-255 символов) с помощью ::isValidName
 * 2. Состав символов с помощью ::isValidName
 * 3. При обнаружении ошибки добавляет ее в errors
 *
 * \param[in] name Проверяемое текстовое имя.
 * \param[in] contextError Текстовое описание контекста для логирования.
 * \param[out] errors Контейнер для фиксации ошибок.
 * \param[in] isClass Флаг сущности: \c true — проверяется имя класса, \c false — имя свойства.
 * \return \c true, если имя прошло все проверки, иначе \c false.
 *
 * \sa ::isValidName
 * \sa ErrorType::invalidClassNameLength
 * \sa ErrorType::invalidPropertyNameLength
 * \sa ErrorType::invalidCharacters
 */
bool checkStringCharacters(const QString& name, const QString& contextError,
                           QSet<Error>& errors, bool isClass = true);

/*!
 * \fn void checkValueCountRule(const QJsonObject& propObj, const QString& propName, Property& prop, QSet<Error>& errors)
 * \brief Внутренняя функция валидации правила "value_count".
 *
 * Проверяет, является ли значение массивом строго из 1 целого числа в диапазоне от 1 до 1000.
 *
 * Используется для правил типа PropertyRuleType::HasPropertyWithCount.
 *
 * Алгоритм:
 * 1. Проверяет наличие поля "value_count"
 * 2. Проверяет, что значение является массивом из одного числа
 * 3. Проверяет диапазон значения [1, 1000]
 * 4. При успехе устанавливает prop.ruleType = PropertyRuleType::HasPropertyWithCount
 *
 * \param[in] propObj Текущий JSON-объект свойства.
 * \param[in] propName Имя свойства.
 * \param[out] prop Заполняемая структура свойства.
 * \param[out] errors Контейнер ошибок.
 *
 * \sa PropertyRuleType::HasPropertyWithCount
 * \sa ErrorType::invalidValueCountType
 * \sa ErrorType::invalidValueRange
 * \sa ::checkExpectedValueRule
 */
void checkValueCountRule(const QJsonObject& propObj, const QString& propName,
                         Property& prop, QSet<Error>& errors);

/*!
 * \fn void checkExpectedValueRule(const QJsonObject& propObj, const QString& propName, Property& prop, QSet<Error>& errors)
 * \brief Внутренняя функция валидации правила "expected_value".
 *
 * Проверяет, является ли значение непустым целочисленным массивом (размер до 100 элементов),
 * где каждое число находится в диапазоне от 1 до 1000.
 *
 * Используется для правил типа:
 * - PropertyRuleType::HasPropertyWithValue (массив из 1 элемента)
 * - PropertyRuleType::HasPropertyWithValues (массив из >1 элемента)
 *
 * Алгоритм:
 * 1. Проверяет наличие поля "expected_value"
 * 2. Проверяет, что значение является массивом
 * 3. Проверяет, что массив не пустой
 * 4. Проверяет размер массива (≤ 100)
 * 5. Проверяет каждый элемент на целочисленность и диапазон [1, 1000]
 * 6. При успехе устанавливает соответствующий тип правила
 *
 * \param[in] propObj Текущий JSON-объект свойства.
 * \param[in] propName Имя свойства.
 * \param[out] prop Заполняемая структура свойства.
 * \param[out] errors Контейнер ошибок.
 *
 * \sa PropertyRuleType::HasPropertyWithValue
 * \sa PropertyRuleType::HasPropertyWithValues
 * \sa ErrorType::invalidValueType
 * \sa ErrorType::emptyExpectedValue
 * \sa ErrorType::tooManyExpectedValues
 * \sa ErrorType::invalidValueRange
 * \sa ::checkValueCountRule
 */
void checkExpectedValueRule(const QJsonObject& propObj, const QString& propName,
                            Property& prop, QSet<Error>& errors);

/*!
 * \fn bool validatePropertyObject(const QJsonObject& propObj, int index, const QString& className, QSet<Error>& errors, Property& prop)
 * \brief Внутренняя функция валидации отдельного JSON-объекта свойства.
 *
 * Проверяет отсутствие лишних полей, наличие обязательного поля "name", строковый тип имени,
 * а также выявляет логическую неопределенность (когда заданы и "value_count", и "expected_value").
 *
 * Алгоритм:
 * 1. Проверяет наличие лишних полей (ErrorType::extraField)
 * 2. Проверяет наличие поля "name" (ErrorType::missingPropertyName)
 * 3. Проверяет тип поля "name" (должен быть строкой)
 * 4. Проверяет имя свойства через ::checkStringCharacters
 * 5. Проверяет на конфликт правил (ErrorType::ambiguousRule)
 * 6. Вызывает ::checkValueCountRule и ::checkExpectedValueRule
 *
 * \param[in] propObj JSON-объект свойства для проверки.
 * \param[in] index Порядковый индекс свойства в родительском массиве.
 * \param[in] className Имя класса, содержащего данное свойство.
 * \param[out] errors Ссылка на набор ошибок.
 * \param[out] prop Выходная структура для сохранения извлеченных данных.
 * \return \c true, если структура свойства валидна, иначе \c false.
 *
 * \sa ::checkStringCharacters
 * \sa ::checkValueCountRule
 * \sa ::checkExpectedValueRule
 * \sa ErrorType
 * \sa ::validateClassStructuredObject
 */
bool validatePropertyObject(const QJsonObject& propObj, int index,
                            const QString& className, QSet<Error>& errors,
                            Property& prop);

/*!
 * \fn void validateClassStructuredObject(const QJsonObject& classObj, int index, QSet<Error>& errors, QVector<Class>& parsedClasses)
 * \brief Внутренняя функция структурированной валидации одного объекта класса.
 *
 * Проверяет наличие обязательных полей "class_name" и массива "properties",
 * контролирует лимиты размеров и запускает валидацию вложенных свойств.
 *
 * Алгоритм:
 * 1. Проверяет наличие лишних полей в объекте класса
 * 2. Проверяет наличие поля "class_name" (ErrorType::missingClassName)
 * 3. Проверяет тип поля "class_name" (должен быть строкой)
 * 4. Проверяет имя класса через ::checkStringCharacters
 * 5. Проверяет наличие поля "properties" (ErrorType::missingProperties)
 * 6. Проверяет тип поля "properties" (должен быть массивом)
 * 7. Проверяет размер массива (не пустой, ≤ 100)
 * 8. Для каждого свойства вызывает ::validatePropertyObject
 *
 * \param[in] classObj JSON-объект класса для проверки.
 * \param[in] index Порядковый индекс класса в корневом массиве документов.
 * \param[out] errors Набор ошибок.
 * \param[out] parsedClasses Накопительный вектор распарсенных классов.
 *
 * \sa ::validatePropertyObject
 * \sa ::checkStringCharacters
 * \sa ErrorType
 * \sa validator::validateInput
 */
void validateClassStructuredObject(const QJsonObject& classObj, int index,
                                   QSet<Error>& errors, QVector<Class>& parsedClasses);

#endif // VALIDATOR_H
