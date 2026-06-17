#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTemporaryFile>
#include <QSet>

#include "structures.h"

// Прототипы тестируемых функций проекта
bool validateInput(const QJsonObject& json, QSet<Error>& errors, QVector<Class>& parsedClasses);
bool matchProperty(const PropertyRule& rule, const PropertyRule& candidate);
bool isSubset(const ClassNode& A, const ClassNode& B);
void buildClassHierarchy(ClassHierarchy& hierarchy, const QVector<Class>& parsedClasses);
void removeTransitiveEdges(ClassHierarchy& hierarchy);

/*!
 * \brief Класс test реализует модульное тестирование (Unit Testing) всех ключевых модулей системы.
 * * Тестовый класс покрывает три основных этапа работы:
 * 1. Валидация входных JSON-структур (граничные значения, некорректные типы, ошибки синтаксиса).
 * 2. Отношения вложенности свойств и подмножеств классов (isSubset, matchProperty).
 * 3. Логика построения ориентированного графа и алгоритм его транзитивного сокращения.
 */
class test : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Конструктор тестового класса по умолчанию.
     */
    test();

    /*!
     * \brief Деструктор тестового класса.
     */
    ~test();

private slots:

    /* ========================================================================
     * Тесты для функции validateInput
     * ======================================================================== */

    /*! \brief Проверка обработки пустого JSON-объекта `{}`. */
    void validateInput_t1_emptyJson();

    /*! \brief Проверка отсутствия обязательного корневого ключа "classes". */
    void validateInput_t2_missingClasses();

    /*! \brief Проверка передачи пустого массива классов `"classes": []`. */
    void validateInput_t3_emptyClassesArray();

    /*! \brief Проверка отсутствия поля "class_name" внутри объекта класса. */
    void validateInput_t4_missingClassName();

    /*! \brief Проверка отсутствия поля "properties" внутри объекта класса. */
    void validateInput_t5_missingProperties();

    /*! \brief Проверка передачи пустого массива свойств `"properties": []`. */
    void validateInput_t6_emptyProperties();

    /*! \brief Проверка отсутствия поля "name" у элемента массива свойств. */
    void validateInput_t7_missingPropertyName();

    /*! \brief Проверка выявления двусмысленности правил (одновременно заданы value_count и expected_value). */
    void validateInput_t8_ambiguousRule();

    /*! \brief Проверка выявления неизвестных/избыточных полей (extra fields) в JSON-объекте. */
    void validateInput_t9_extraField();

    /*! \brief Проверка валидации недопустимых символов в имени класса (например, знаков восклицания). */
    void validateInput_t10_invalidCharactersInClassName();

    /*! \brief Проверка ограничения на максимальное количество классов (лимит 100 элементов). */
    void validateInput_t11_tooManyClasses();

    /*! \brief Проверка реакции на нестроковый тип значения в поле "class_name". */
    void validateInput_t12_invalidClassNameType();

    /*! \brief Проверка валидации нулевой длины имени класса. */
    void validateInput_t13_emptyClassNameLength();

    /*! \brief Проверка превышения максимальной длины имени класса (лимит 255 символов). */
    void validateInput_t14_exceededClassNameLength();

    /*! \brief Проверка реакции на невалидный тип данных поля "properties" (строка вместо массива). */
    void validateInput_t15_invalidPropertiesType();

    /*! \brief Проверка ограничения на максимальное количество свойств в одном классе (лимит 100 элементов). */
    void validateInput_t16_tooManyProperties();

    /*! \brief Проверка реакции на нестроковый тип значения в имени свойства "name". */
    void validateInput_t17_invalidPropertyNameType();

    /*! \brief Проверка валидации нулевой длины имени свойства. */
    void validateInput_t18_emptyPropertyNameLength();

    /*! \brief Проверка реакции на невалидный тип данных в поле "value_count" (строка вместо массива чисел). */
    void validateInput_t19_invalidValueCountType();

    /*! \brief Проверка выхода числового значения "value_count" за допустимые границы диапазона [1, 1000]. */
    void validateInput_t20_valueCountOutOfRange();

    /*! \brief Проверка передачи пустого массива ограничений "expected_value". */
    void validateInput_t21_emptyExpectedValue();

    /*! \brief Проверка превышения лимита размера массива "expected_value" (лимит 100 элементов). */
    void validateInput_t22_tooManyExpectedValues();

    /*! \brief Проверка выявления нечисловых типов (например, строк) внутри массива "expected_value". */
    void validateInput_t23_expectedValueNotANumber();


    /* ========================================================================
     * Тесты для функции buildClassHierarchy
     * ======================================================================== */

    /*! \brief Тест базового одиночного наследования: связь родитель -> потомок. */
    void buildClassHierarchy_t1_simpleInheritance();

    /*! \brief Тест многоуровневого последовательного наследования: связи A -> B -> C и A -> C. */
    void buildClassHierarchy_t2_multilevelHierarchy();

    /*! \brief Тест изоляции классов: проверка отсутствия ребер при непересекающихся свойствах. */
    void buildClassHierarchy_t3_independentClasses();

    /*! \brief Тест ромбовидного (Diamond) наследования: проверка связей в сложных ветвлениях. */
    void buildClassHierarchy_t4_diamondHierarchy();

    /*! \brief Пустой список классов на входе: проверка инициализации пустой структуры. */
    void buildClassHierarchy_t5_emptyInput();

    /*! \brief Циклическая зависимость (А содержит Б, Б содержит А): проверка устойчивости логики. */
    void buildClassHierarchy_t6_cyclicDependency();

    /*! \brief Полный граф (каждый класс является подмножеством каждого): проверка генерации всех связей. */
    void buildClassHierarchy_t7_fullGraph();


    /* ========================================================================
     * Тесты для функции removeTransitiveEdges
     * ======================================================================== */

    /*! \brief Проверка удаления прямой транзитивной связи A -> C при наличии цепочки A -> B -> C. */
    void removeTransitiveEdges_t1_removal();

    /*! \brief Проверка сохранения графа без изменений, если транзитивные дубликаты отсутствуют. */
    void removeTransitiveEdges_t2_noTransitivity();

    /*! \brief Тест глубокого транзитивного сокращения на комплексном многосвязном графе. */
    void removeTransitiveEdges_t3_multipleEdges();

    /*! \brief Пустой граф: проверка работы алгоритма на пустом контейнере связей. */
    void removeTransitiveEdges_t4_emptyGraph();

    /*! \brief Вершины без ребер (изолированные классы): проверка отсутствия ложных исключений. */
    void removeTransitiveEdges_t5_isolatedNodes();

    /*! \brief Длинная линейная цепочка (A->B->C->D->E): удаление всех сквозных ребер (A->C, A->D, B->D...). */
    void removeTransitiveEdges_t6_longLinearChain();

    /*! \brief Цикл из двух вершин (A->B и B->A): проверка сохранения структуры (не транзитивность). */
    void removeTransitiveEdges_t7_loopSize2();

    /*! \brief Цикл из трех вершин (A->B->C->A): проверка отсутствия бесконечного зацикливания. */
    void removeTransitiveEdges_t8_loopSize3();

    /*! \brief Петля на самого себя (A->A): проверка удаления/сохранения селф-петли. */
    void removeTransitiveEdges_t9_selfLoop();

    /*! \brief Одна вершина имеет несколько независимых потомков (звезда): ребра не должны удаляться. */
    void removeTransitiveEdges_t10_starTopology();

    /*! \brief Двудольный граф (несколько родителей, несколько независимых потомков). */
    void removeTransitiveEdges_t11_bipartiteGraph();

    /*! \brief Дублирование ребер в векторе смежности (если С++ контейнер хранит дубли): проверка уникализации. */
    void removeTransitiveEdges_t12_duplicateEdgesInVector();

    /*! \brief Сложный граф с "ложными" путями, которые длиннее, но не покрывают транзитивность. */
    void removeTransitiveEdges_t13_complexFalseTransitivity();

    /*! \brief Граф с множеством изолированных подграфов (компоненты связности). */
    void removeTransitiveEdges_t14_disconnectedSubgraphs();


    /* ========================================================================
     * Тесты для функции isSubset
     * ======================================================================== */

    /*! \brief Проверка идентичных наборов правил: два класса со строго одинаковыми свойствами. */
    void isSubset_t1_fullMatch();

    /*! \brief Проверка классического расширения: базовый класс покрывает часть свойств дочернего. */
    void isSubset_t2_baseInDerived();

    /*! \brief Проверка нарушения подмножества, если у базового класса есть свойства, отсутствующие в дочернем. */
    void isSubset_t3_notAllIncluded();

    /*! \brief Проверка классов с полностью уникальными, непересекающимися именами свойств. */
    void isSubset_t4_completelyDifferent();

    /*! \brief Проверка совместимости подмножеств по правилу общего "value_count". */
    void isSubset_t5_valueCountInheritance();

    /*! \brief Нарушение отношения подмножества из-за несовпадения значений "value_count". */
    void isSubset_t6_valueCountMismatch();

    /*! \brief Проверка совместимости подмножеств по правилу точного "expected_value". */
    void isSubset_t7_expectedValueInheritance();

    /*! \brief Нарушение отношения подмножества из-за несовпадения значений внутри "expected_value". */
    void isSubset_t8_expectedValueMismatch();


    /* ========================================================================
     * Тесты для функции matchProperty
     * ======================================================================== */

    /*! \brief Тест эквивалентности простых атомарных свойств (без дополнительных условий). */
    void matchProperty_t1_simplePropertyMatch();

    /*! \brief Проверка несовпадения свойств из-за различий в наименовании (идентификаторе). */
    void matchProperty_t2_propertyNameMismatch();

    /*! \brief Тест эквивалентности правил с идентичным числовым значением "value_count". */
    void matchProperty_t3_valueCountMatch();

    /*! \brief Проверка несовпадения правил "value_count" с разными числовыми значениями. */
    void matchProperty_t4_valueCountMismatch();

    /*! \brief Тест эквивалентности правил с одинаковым одиночным "expected_value". */
    void matchProperty_t5_expectedValueMatch();

    /*! \brief Проверка несовпадения правил одиночных ожидаемых значений при их различии. */
    void matchProperty_t6_expectedValueMismatch();

    /*! \brief Тест эквивалентности правил со строго идентичными массивами "expected_value". */
    void matchProperty_t7_expectedValueArrayMatch();

    /*! \brief Проверка несовпадения правил массивов "expected_value" при несовпадении хотя бы одного элемента. */
    void matchProperty_t8_expectedValueArrayMismatch();
};

test::test() {}

test::~test() {}

/*!
 * \brief Вспомогательный статический метод для быстрой компиляции строки в JSON-объект.
 * \param[in] text Валидная строка с JSON-разметкой.
 * \return Объект класса QJsonObject.
 */
static QJsonObject parseJson(const QString& text) {
    return QJsonDocument::fromJson(text.toUtf8()).object();
}

/*!
 * \brief Вспомогательный предикат для поиска типа ошибки в контейнере зафиксированных исключений.
 * \param[in] errors Множество обнаруженных в ходе валидации ошибок.
 * \param[in] type Искомый тип ошибки валидации.
 * \return \c true, если ошибка указанной категории присутствует в наборе, иначе \c false.
 */
static bool hasError(const QSet<Error>& errors, ErrorType type) {
    for (const auto& err : errors) {
        if (err.type == type) return true;
    }
    return false;
}

/*!
 * \brief Вспомогательный предикат для верификации существования направленного ребра в графе таблицы смежности.
 * \param[in] hierarchy Объект иерархии (граф).
 * \param[in] from Имя вершины-источника (родитель).
 * \param[in] to Имя вершины-приемника (потомок).
 * \return \c true, если связь зафиксирована в графе, иначе \c false.
 */
static bool hasEdge(const ClassHierarchy& hierarchy, const QString& from, const QString& to) {
    return hierarchy.edges.contains(from) && hierarchy.edges.value(from).contains(to);
}

// ============================================================================
// РЕАЛИЗАЦИЯ ТЕСТОВ: Приложение 1. validateInput
// ============================================================================

void test::validateInput_t1_emptyJson() {
    QJsonObject json = parseJson("{}");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    bool res = validateInput(json, errors, parsedClasses);
    QCOMPARE(res, false);
    QVERIFY(hasError(errors, ErrorType::emptyClassesArray) || hasError(errors, ErrorType::extraField));
}

void test::validateInput_t2_missingClasses() {
    QJsonObject json = parseJson("{ \"data\": [] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    bool res = validateInput(json, errors, parsedClasses);
    QCOMPARE(res, false);
    QVERIFY(hasError(errors, ErrorType::emptyClassesArray) || hasError(errors, ErrorType::extraField));
}

void test::validateInput_t3_emptyClassesArray() {
    QJsonObject json = parseJson("{ \"classes\": [] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    bool res = validateInput(json, errors, parsedClasses);
    QCOMPARE(res, false);
    QVERIFY(hasError(errors, ErrorType::emptyClassesArray));
}

void test::validateInput_t4_missingClassName() {
    QJsonObject json = parseJson("{ \"classes\": [ { \"properties\": [ { \"name\": \"size\" } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    bool res = validateInput(json, errors, parsedClasses);
    QCOMPARE(res, false);
    QVERIFY(hasError(errors, ErrorType::missingClassName));
}

void test::validateInput_t5_missingProperties() {
    QJsonObject json = parseJson("{ \"classes\": [ { \"class_name\": \"Device\" } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    bool res = validateInput(json, errors, parsedClasses);
    QCOMPARE(res, false);
    QVERIFY(hasError(errors, ErrorType::missingProperties));
}

void test::validateInput_t6_emptyProperties() {
    QJsonObject json = parseJson("{ \"classes\": [ { \"class_name\": \"Device\", \"properties\": [] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    bool res = validateInput(json, errors, parsedClasses);
    QCOMPARE(res, false);
    QVERIFY(hasError(errors, ErrorType::emptyProperties) || !errors.isEmpty());
}

void test::validateInput_t7_missingPropertyName() {
    QJsonObject json = parseJson("{ \"classes\": [ { \"class_name\": \"Device\", \"properties\": [ { \"value_count\": [2] } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    bool res = validateInput(json, errors, parsedClasses);
    QCOMPARE(res, false);
    QVERIFY(hasError(errors, ErrorType::missingPropertyName));
}

void test::validateInput_t8_ambiguousRule() {
    QJsonObject json = parseJson("{ \"classes\": [ { \"class_name\": \"Device\", \"properties\": [ { \"name\": \"size\", \"value_count\": [2], \"expected_value\": [1,2] } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::ambiguousRule));
}

void test::validateInput_t9_extraField() {
    QJsonObject json = parseJson("{ \"classes\": [ { \"class_name\": \"Device\", \"abc\": 10, \"properties\": [ { \"name\": \"size\" } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::extraField));
}

void test::validateInput_t10_invalidCharactersInClassName() {
    QJsonObject json = parseJson("{ \"classes\": [ { \"class_name\": \"Device!!!\", \"properties\": [ { \"name\": \"size\" } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::invalidCharacters));
}

void test::validateInput_t11_tooManyClasses() {
    QJsonObject json;
    QJsonArray classesArray;
    for(int i = 0; i < 102; ++i) {
        QJsonObject cls;
        cls["class_name"] = QString("Class%1").arg(i);
        QJsonObject prop; prop["name"] = "p";
        QJsonArray props; props.append(prop);
        cls["properties"] = props;
        classesArray.append(cls);
    }
    json["classes"] = classesArray;

    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::tooManyClasses));
}

void test::validateInput_t12_invalidClassNameType() {
    QJsonObject json = parseJson("{\"classes\": [ { \"class_name\": 123, \"properties\": [ { \"name\": \"size\" } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::invalidCharacters));
}

void test::validateInput_t13_emptyClassNameLength() {
    QJsonObject json = parseJson("{\"classes\": [ { \"class_name\": \"\", \"properties\": [ { \"name\": \"size\" } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::invalidClassNameLength));
}

void test::validateInput_t14_exceededClassNameLength() {
    QString longName = QString(256, 'A');
    QJsonObject json;
    QJsonObject cls;
    cls["class_name"] = longName;
    QJsonObject prop; prop["name"] = "size";
    QJsonArray props; props.append(prop);
    cls["properties"] = props;
    QJsonArray classes; classes.append(cls);
    json["classes"] = classes;

    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::invalidClassNameLength));
}

void test::validateInput_t15_invalidPropertiesType() {
    QJsonObject json = parseJson("{\"classes\": [ { \"class_name\": \"Device\", \"properties\": \"not_an_array\" } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::missingProperties) || !errors.isEmpty());
}

void test::validateInput_t16_tooManyProperties() {
    QJsonObject json;
    QJsonObject cls;
    cls["class_name"] = "Device";
    QJsonArray props;
    for(int i = 0; i < 102; ++i) {
        QJsonObject p; p["name"] = QString("prop%1").arg(i);
        props.append(p);
    }
    cls["properties"] = props;
    QJsonArray classes; classes.append(cls);
    json["classes"] = classes;

    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::tooManyProperties));
}

void test::validateInput_t17_invalidPropertyNameType() {
    QJsonObject json = parseJson("{\"classes\": [ { \"class_name\": \"Device\", \"properties\": [ { \"name\": true } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::invalidCharacters));
}

void test::validateInput_t18_emptyPropertyNameLength() {
    QJsonObject json = parseJson("{\"classes\": [ { \"class_name\": \"Device\", \"properties\": [ { \"name\": \"\" } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::invalidPropertyNameLength));
}

void test::validateInput_t19_invalidValueCountType() {
    QJsonObject json = parseJson("{\"classes\": [ { \"class_name\": \"Device\", \"properties\": [ { \"name\": \"p\", \"value_count\": \"three\" } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::invalidValueCountType));
}

void test::validateInput_t20_valueCountOutOfRange() {
    QJsonObject json = parseJson("{\"classes\": [ { \"class_name\": \"Device\", \"properties\": [ { \"name\": \"p\", \"value_count\": [1500] } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::invalidValueRange));
}

void test::validateInput_t21_emptyExpectedValue() {
    QJsonObject json = parseJson("{\"classes\": [ { \"class_name\": \"Device\", \"properties\": [ { \"name\": \"p\", \"expected_value\": [] } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::emptyExpectedValue));
}

void test::validateInput_t22_tooManyExpectedValues() {
    QJsonObject json;
    QJsonObject cls; cls["class_name"] = "Device";
    QJsonObject prop; prop["name"] = "p";
    QJsonArray vals;
    for(int i = 0; i < 102; ++i) vals.append(i);
    prop["expected_value"] = vals;
    QJsonArray props; props.append(prop);
    cls["properties"] = props;
    QJsonArray classes; classes.append(cls);
    json["classes"] = classes;

    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::tooManyExpectedValues));
}

void test::validateInput_t23_expectedValueNotANumber() {
    QJsonObject json = parseJson("{\"classes\": [ { \"class_name\": \"Device\", \"properties\": [ { \"name\": \"p\", \"expected_value\": [1, \"two\", 3] } ] } ] }");
    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validateInput(json, errors, parsedClasses);
    QVERIFY(hasError(errors, ErrorType::invalidValueType));
}

// ============================================================================
// РЕАЛИЗАЦИЯ ТЕСТОВ: Приложение 2. buildClassHierarchy
// ============================================================================

void test::buildClassHierarchy_t1_simpleInheritance() {
    ClassHierarchy hierarchy;
    QVector<Class> dummyParsedClasses;

    ClassNode* nodeA = new ClassNode("Device");
    nodeA->properties.insert(PropertyRule{"power", PropertyRuleType::HasProperty, {}, {}});

    ClassNode* nodeB = new ClassNode("SmartDevice");
    nodeB->properties.insert(PropertyRule{"power", PropertyRuleType::HasProperty, {}, {}});
    nodeB->properties.insert(PropertyRule{"wifi", PropertyRuleType::HasProperty, {}, {}});

    hierarchy.classes.insert(nodeA);
    hierarchy.classes.insert(nodeB);

    buildClassHierarchy(hierarchy, dummyParsedClasses);

    QVERIFY(hasEdge(hierarchy, "Device", "SmartDevice"));
}

void test::buildClassHierarchy_t2_multilevelHierarchy() {
    ClassHierarchy hierarchy;
    QVector<Class> dummyParsedClasses;

    ClassNode* nodeA = new ClassNode("A");
    nodeA->properties.insert(PropertyRule{"x", PropertyRuleType::HasProperty, {}, {}});

    ClassNode* nodeB = new ClassNode("B");
    nodeB->properties.insert(PropertyRule{"x", PropertyRuleType::HasProperty, {}, {}});
    nodeB->properties.insert(PropertyRule{"y", PropertyRuleType::HasProperty, {}, {}});

    ClassNode* nodeC = new ClassNode("C");
    nodeC->properties.insert(PropertyRule{"x", PropertyRuleType::HasProperty, {}, {}});
    nodeC->properties.insert(PropertyRule{"y", PropertyRuleType::HasProperty, {}, {}});
    nodeC->properties.insert(PropertyRule{"z", PropertyRuleType::HasProperty, {}, {}});

    hierarchy.classes.insert(nodeA);
    hierarchy.classes.insert(nodeB);
    hierarchy.classes.insert(nodeC);

    buildClassHierarchy(hierarchy, dummyParsedClasses);

    QVERIFY(hasEdge(hierarchy, "A", "B"));
    QVERIFY(hasEdge(hierarchy, "B", "C"));
    QVERIFY(hasEdge(hierarchy, "A", "C"));
}

void test::buildClassHierarchy_t3_independentClasses() {
    ClassHierarchy hierarchy;
    QVector<Class> dummyParsedClasses;

    ClassNode* nodeA = new ClassNode("A");
    nodeA->properties.insert(PropertyRule{"x", PropertyRuleType::HasProperty, {}, {}});

    ClassNode* nodeB = new ClassNode("B");
    nodeB->properties.insert(PropertyRule{"y", PropertyRuleType::HasProperty, {}, {}});

    hierarchy.classes.insert(nodeA);
    hierarchy.classes.insert(nodeB);

    buildClassHierarchy(hierarchy, dummyParsedClasses);

    QVERIFY(hierarchy.edges.isEmpty());
}

void test::buildClassHierarchy_t4_diamondHierarchy() {
    ClassHierarchy hierarchy;
    QVector<Class> dummyParsedClasses;

    ClassNode* nodeA = new ClassNode("A");
    nodeA->properties.insert(PropertyRule{"x", PropertyRuleType::HasProperty, {}, {}});

    ClassNode* nodeB = new ClassNode("B");
    nodeB->properties.insert(PropertyRule{"x", PropertyRuleType::HasProperty, {}, {}});
    nodeB->properties.insert(PropertyRule{"y", PropertyRuleType::HasProperty, {}, {}});

    ClassNode* nodeC = new ClassNode("C");
    nodeC->properties.insert(PropertyRule{"x", PropertyRuleType::HasProperty, {}, {}});
    nodeC->properties.insert(PropertyRule{"z", PropertyRuleType::HasProperty, {}, {}});

    ClassNode* nodeD = new ClassNode("D");
    nodeD->properties.insert(PropertyRule{"x", PropertyRuleType::HasProperty, {}, {}});
    nodeD->properties.insert(PropertyRule{"y", PropertyRuleType::HasProperty, {}, {}});
    nodeD->properties.insert(PropertyRule{"z", PropertyRuleType::HasProperty, {}, {}});

    hierarchy.classes.insert(nodeA);
    hierarchy.classes.insert(nodeB);
    hierarchy.classes.insert(nodeC);
    hierarchy.classes.insert(nodeD);

    buildClassHierarchy(hierarchy, dummyParsedClasses);

    QVERIFY(hasEdge(hierarchy, "A", "B"));
    QVERIFY(hasEdge(hierarchy, "A", "C"));
    QVERIFY(hasEdge(hierarchy, "A", "D"));
    QVERIFY(hasEdge(hierarchy, "B", "D"));
    QVERIFY(hasEdge(hierarchy, "C", "D"));
}

void test::buildClassHierarchy_t5_emptyInput() {
    ClassHierarchy hierarchy;
    QVector<Class> dummyParsedClasses;
    buildClassHierarchy(hierarchy, dummyParsedClasses);
    QVERIFY(hierarchy.classes.isEmpty());
    QVERIFY(hierarchy.edges.isEmpty());
}

void test::buildClassHierarchy_t6_cyclicDependency() {
    ClassHierarchy hierarchy;
    QVector<Class> dummyParsedClasses;

    ClassNode* nodeA = new ClassNode("A");
    nodeA->properties.insert(PropertyRule{"x", PropertyRuleType::HasProperty, {}, {}});

    ClassNode* nodeB = new ClassNode("B");
    nodeB->properties.insert(PropertyRule{"x", PropertyRuleType::HasProperty, {}, {}});

    hierarchy.classes.insert(nodeA);
    hierarchy.classes.insert(nodeB);

    buildClassHierarchy(hierarchy, dummyParsedClasses);
    QVERIFY(hasEdge(hierarchy, "A", "B"));
    QVERIFY(hasEdge(hierarchy, "B", "A"));
}

void test::buildClassHierarchy_t7_fullGraph() {
    ClassHierarchy hierarchy;
    QVector<Class> dummyParsedClasses;

    ClassNode* nodeA = new ClassNode("A");
    ClassNode* nodeB = new ClassNode("B");
    ClassNode* nodeC = new ClassNode("C");

    hierarchy.classes.insert(nodeA);
    hierarchy.classes.insert(nodeB);
    hierarchy.classes.insert(nodeC);

    buildClassHierarchy(hierarchy, dummyParsedClasses);
    QVERIFY(hasEdge(hierarchy, "A", "B"));
    QVERIFY(hasEdge(hierarchy, "A", "C"));
    QVERIFY(hasEdge(hierarchy, "B", "A"));
    QVERIFY(hasEdge(hierarchy, "B", "C"));
    QVERIFY(hasEdge(hierarchy, "C", "A"));
    QVERIFY(hasEdge(hierarchy, "C", "B"));
}

// ============================================================================
// РЕАЛИЗАЦИЯ ТЕСТОВ: Приложение 3. removeTransitiveEdges
// ============================================================================

void test::removeTransitiveEdges_t1_removal() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("A"));
    hierarchy.classes.insert(new ClassNode("B"));
    hierarchy.classes.insert(new ClassNode("C"));

    hierarchy.edges["A"].append("B");
    hierarchy.edges["B"].append("C");
    hierarchy.edges["A"].append("C");

    removeTransitiveEdges(hierarchy);
    QVERIFY(hasEdge(hierarchy, "A", "B"));
    QVERIFY(hasEdge(hierarchy, "B", "C"));
    QCOMPARE(hasEdge(hierarchy, "A", "C"), false);
}

void test::removeTransitiveEdges_t2_noTransitivity() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("A"));
    hierarchy.classes.insert(new ClassNode("B"));
    hierarchy.edges["A"].append("B");

    removeTransitiveEdges(hierarchy);
    QVERIFY(hasEdge(hierarchy, "A", "B"));
    QCOMPARE(hierarchy.edges.size(), 1);
}

void test::removeTransitiveEdges_t3_multipleEdges() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("A"));
    hierarchy.classes.insert(new ClassNode("B"));
    hierarchy.classes.insert(new ClassNode("C"));
    hierarchy.classes.insert(new ClassNode("D"));

    hierarchy.edges["A"].append("B"); hierarchy.edges["B"].append("C"); hierarchy.edges["C"].append("D");
    hierarchy.edges["A"].append("C"); hierarchy.edges["A"].append("D"); hierarchy.edges["B"].append("D");

    removeTransitiveEdges(hierarchy);
    QVERIFY(hasEdge(hierarchy, "A", "B"));
    QVERIFY(hasEdge(hierarchy, "B", "C"));
    QVERIFY(hasEdge(hierarchy, "C", "D"));
    QCOMPARE(hasEdge(hierarchy, "A", "C"), false);
    QCOMPARE(hasEdge(hierarchy, "A", "D"), false);
    QCOMPARE(hasEdge(hierarchy, "B", "D"), false);
}

void test::removeTransitiveEdges_t4_emptyGraph() {
    ClassHierarchy hierarchy;
    removeTransitiveEdges(hierarchy);
    QVERIFY(hierarchy.edges.isEmpty());
}

void test::removeTransitiveEdges_t5_isolatedNodes() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("A"));
    hierarchy.classes.insert(new ClassNode("B"));
    hierarchy.classes.insert(new ClassNode("C"));

    hierarchy.edges["A"] = QVector<QString>();
    hierarchy.edges["B"] = QVector<QString>();
    hierarchy.edges["C"] = QVector<QString>();

    removeTransitiveEdges(hierarchy);
    QVERIFY(hierarchy.edges["A"].isEmpty());
    QVERIFY(hierarchy.edges["B"].isEmpty());
    QVERIFY(hierarchy.edges["C"].isEmpty());
}

void test::removeTransitiveEdges_t6_longLinearChain() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("A"));
    hierarchy.classes.insert(new ClassNode("B"));
    hierarchy.classes.insert(new ClassNode("C"));
    hierarchy.classes.insert(new ClassNode("D"));
    hierarchy.classes.insert(new ClassNode("E"));

    hierarchy.edges["A"].append("B"); hierarchy.edges["B"].append("C");
    hierarchy.edges["C"].append("D"); hierarchy.edges["D"].append("E");

    hierarchy.edges["A"].append("C"); hierarchy.edges["A"].append("D"); hierarchy.edges["A"].append("E");
    hierarchy.edges["B"].append("D"); hierarchy.edges["B"].append("E");
    hierarchy.edges["C"].append("E");

    removeTransitiveEdges(hierarchy);
    QVERIFY(hasEdge(hierarchy, "A", "B"));
    QVERIFY(hasEdge(hierarchy, "B", "C"));
    QVERIFY(hasEdge(hierarchy, "C", "D"));
    QVERIFY(hasEdge(hierarchy, "D", "E"));

    QCOMPARE(hasEdge(hierarchy, "A", "C"), false);
    QCOMPARE(hasEdge(hierarchy, "A", "D"), false);
    QCOMPARE(hasEdge(hierarchy, "A", "E"), false);
    QCOMPARE(hasEdge(hierarchy, "B", "D"), false);
    QCOMPARE(hasEdge(hierarchy, "B", "E"), false);
    QCOMPARE(hasEdge(hierarchy, "C", "E"), false);
}

void test::removeTransitiveEdges_t7_loopSize2() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("A"));
    hierarchy.classes.insert(new ClassNode("B"));

    hierarchy.edges["A"].append("B");
    hierarchy.edges["B"].append("A");

    removeTransitiveEdges(hierarchy);
    QVERIFY(hasEdge(hierarchy, "A", "B"));
    QVERIFY(hasEdge(hierarchy, "B", "A"));
}

void test::removeTransitiveEdges_t8_loopSize3() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("A"));
    hierarchy.classes.insert(new ClassNode("B"));
    hierarchy.classes.insert(new ClassNode("C"));

    hierarchy.edges["A"].append("B");
    hierarchy.edges["B"].append("C");
    hierarchy.edges["C"].append("A");

    removeTransitiveEdges(hierarchy);
    QVERIFY(hasEdge(hierarchy, "A", "B"));
    QVERIFY(hasEdge(hierarchy, "B", "C"));
    QVERIFY(hasEdge(hierarchy, "C", "A"));
}

void test::removeTransitiveEdges_t9_selfLoop() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("A"));
    hierarchy.edges["A"].append("A");

    removeTransitiveEdges(hierarchy);
    QCOMPARE(hierarchy.classes.size(), 1);
}

void test::removeTransitiveEdges_t10_starTopology() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("Root"));
    hierarchy.classes.insert(new ClassNode("Child1"));
    hierarchy.classes.insert(new ClassNode("Child2"));
    hierarchy.classes.insert(new ClassNode("Child3"));

    hierarchy.edges["Root"].append("Child1");
    hierarchy.edges["Root"].append("Child2");
    hierarchy.edges["Root"].append("Child3");

    removeTransitiveEdges(hierarchy);
    QVERIFY(hasEdge(hierarchy, "Root", "Child1"));
    QVERIFY(hasEdge(hierarchy, "Root", "Child2"));
    QVERIFY(hasEdge(hierarchy, "Root", "Child3"));
}

void test::removeTransitiveEdges_t11_bipartiteGraph() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("Parent1"));
    hierarchy.classes.insert(new ClassNode("Parent2"));
    hierarchy.classes.insert(new ClassNode("Child1"));
    hierarchy.classes.insert(new ClassNode("Child2"));

    hierarchy.edges["Parent1"].append("Child1");
    hierarchy.edges["Parent1"].append("Child2");
    hierarchy.edges["Parent2"].append("Child1");
    hierarchy.edges["Parent2"].append("Child2");

    removeTransitiveEdges(hierarchy);
    QVERIFY(hasEdge(hierarchy, "Parent1", "Child1"));
    QVERIFY(hasEdge(hierarchy, "Parent1", "Child2"));
    QVERIFY(hasEdge(hierarchy, "Parent2", "Child1"));
    QVERIFY(hasEdge(hierarchy, "Parent2", "Child2"));
}

void test::removeTransitiveEdges_t12_duplicateEdgesInVector() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("A"));
    hierarchy.classes.insert(new ClassNode("B"));

    hierarchy.edges["A"].append("B");
    hierarchy.edges["A"].append("B");

    removeTransitiveEdges(hierarchy);
    QVERIFY(hasEdge(hierarchy, "A", "B"));
}

void test::removeTransitiveEdges_t13_complexFalseTransitivity() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("A"));
    hierarchy.classes.insert(new ClassNode("B"));
    hierarchy.classes.insert(new ClassNode("C"));
    hierarchy.classes.insert(new ClassNode("D"));

    hierarchy.edges["A"].append("B");
    hierarchy.edges["B"].append("C");
    hierarchy.edges["A"].append("D");

    removeTransitiveEdges(hierarchy);
    QVERIFY(hasEdge(hierarchy, "A", "B"));
    QVERIFY(hasEdge(hierarchy, "B", "C"));
    QVERIFY(hasEdge(hierarchy, "A", "D"));
    QCOMPARE(hasEdge(hierarchy, "A", "C"), false);
}

void test::removeTransitiveEdges_t14_disconnectedSubgraphs() {
    ClassHierarchy hierarchy;
    hierarchy.classes.insert(new ClassNode("A1"));
    hierarchy.classes.insert(new ClassNode("B1"));
    hierarchy.classes.insert(new ClassNode("C1"));
    hierarchy.classes.insert(new ClassNode("A2"));
    hierarchy.classes.insert(new ClassNode("B2"));

    hierarchy.edges["A1"].append("B1"); hierarchy.edges["B1"].append("C1"); hierarchy.edges["A1"].append("C1");
    hierarchy.edges["A2"].append("B2");

    removeTransitiveEdges(hierarchy);
    QVERIFY(hasEdge(hierarchy, "A1", "B1"));
    QVERIFY(hasEdge(hierarchy, "B1", "C1"));
    QCOMPARE(hasEdge(hierarchy, "A1", "C1"), false);
    QVERIFY(hasEdge(hierarchy, "A2", "B2"));
}

// ============================================================================
// РЕАЛИЗАЦИЯ ТЕСТОВ: Приложение 4. isSubset
// ============================================================================

void test::isSubset_t1_fullMatch() {
    ClassNode nodeA("DeviceA");
    nodeA.properties.insert(PropertyRule{"power", PropertyRuleType::HasProperty, {}, {}});
    nodeA.properties.insert(PropertyRule{"wifi", PropertyRuleType::HasProperty, {}, {}});

    ClassNode nodeB("DeviceB");
    nodeB.properties.insert(PropertyRule{"power", PropertyRuleType::HasProperty, {}, {}});
    nodeB.properties.insert(PropertyRule{"wifi", PropertyRuleType::HasProperty, {}, {}});

    QCOMPARE(isSubset(nodeA, nodeB), true);
}

void test::isSubset_t2_baseInDerived() {
    ClassNode nodeA("Device");
    nodeA.properties.insert(PropertyRule{"power", PropertyRuleType::HasProperty, {}, {}});

    ClassNode nodeB("SmartDevice");
    nodeB.properties.insert(PropertyRule{"power", PropertyRuleType::HasProperty, {}, {}});
    nodeB.properties.insert(PropertyRule{"wifi", PropertyRuleType::HasProperty, {}, {}});
    nodeB.properties.insert(PropertyRule{"bluetooth", PropertyRuleType::HasProperty, {}, {}});

    QCOMPARE(isSubset(nodeA, nodeB), true);
}

void test::isSubset_t3_notAllIncluded() {
    ClassNode nodeA("Device");
    nodeA.properties.insert(PropertyRule{"power", PropertyRuleType::HasProperty, {}, {}});
    nodeA.properties.insert(PropertyRule{"wifi", PropertyRuleType::HasProperty, {}, {}});

    ClassNode nodeB("SimpleDevice");
    nodeB.properties.insert(PropertyRule{"power", PropertyRuleType::HasProperty, {}, {}});

    QCOMPARE(isSubset(nodeA, nodeB), false);
}

void test::isSubset_t4_completelyDifferent() {
    ClassNode nodeA("Animal");
    nodeA.properties.insert(PropertyRule{"weight", PropertyRuleType::HasProperty, {}, {}});

    ClassNode nodeB("Transport");
    nodeB.properties.insert(PropertyRule{"speed", PropertyRuleType::HasProperty, {}, {}});

    QCOMPARE(isSubset(nodeA, nodeB), false);
}

void test::isSubset_t5_valueCountInheritance() {
    ClassNode nodeA("Triangle");
    nodeA.properties.insert(PropertyRule{"angles", PropertyRuleType::HasPropertyWithCount, {3}, {}});

    ClassNode nodeB("ColoredTriangle");
    nodeB.properties.insert(PropertyRule{"angles", PropertyRuleType::HasPropertyWithCount, {3}, {}});
    nodeB.properties.insert(PropertyRule{"color", PropertyRuleType::HasProperty, {}, {}});

    QCOMPARE(isSubset(nodeA, nodeB), true);
}

void test::isSubset_t6_valueCountMismatch() {
    ClassNode nodeA("Triangle");
    nodeA.properties.insert(PropertyRule{"angles", PropertyRuleType::HasPropertyWithCount, {3}, {}});

    ClassNode nodeB("Rectangle");
    nodeB.properties.insert(PropertyRule{"angles", PropertyRuleType::HasPropertyWithCount, {4}, {}});

    QCOMPARE(isSubset(nodeA, nodeB), false);
}

void test::isSubset_t7_expectedValueInheritance() {
    ClassNode nodeA("LegacyTerminal");
    nodeA.properties.insert(PropertyRule{"com_port", PropertyRuleType::HasPropertyWithValue, {}, {232}});

    ClassNode nodeB("AdvancedTerminal");
    nodeB.properties.insert(PropertyRule{"com_port", PropertyRuleType::HasPropertyWithValue, {}, {232}});
    nodeB.properties.insert(PropertyRule{"display", PropertyRuleType::HasProperty, {}, {}});

    QCOMPARE(isSubset(nodeA, nodeB), true);
}

void test::isSubset_t8_expectedValueMismatch() {
    ClassNode nodeA("LegacyTerminal");
    nodeA.properties.insert(PropertyRule{"com_port", PropertyRuleType::HasPropertyWithValue, {}, {232}});

    ClassNode nodeB("UsbDevice");
    nodeB.properties.insert(PropertyRule{"com_port", PropertyRuleType::HasPropertyWithValue, {}, {111}});

    QCOMPARE(isSubset(nodeA, nodeB), false);
}

// ============================================================================
// РЕАЛИЗАЦИЯ ТЕСТОВ: Приложение 5. matchProperty
// ============================================================================

void test::matchProperty_t1_simplePropertyMatch() {
    PropertyRule rule{"power_source", PropertyRuleType::HasProperty, {}, {}};
    PropertyRule candidate{"power_source", PropertyRuleType::HasProperty, {}, {}};
    QCOMPARE(matchProperty(rule, candidate), true);
}

void test::matchProperty_t2_propertyNameMismatch() {
    PropertyRule rule{"speed", PropertyRuleType::HasProperty, {}, {}};
    PropertyRule candidate{"weight", PropertyRuleType::HasProperty, {}, {}};
    QCOMPARE(matchProperty(rule, candidate), false);
}

void test::matchProperty_t3_valueCountMatch() {
    PropertyRule rule{"angles", PropertyRuleType::HasPropertyWithCount, {3}, {}};
    PropertyRule candidate{"angles", PropertyRuleType::HasPropertyWithCount, {3}, {}};
    QCOMPARE(matchProperty(rule, candidate), true);
}

void test::matchProperty_t4_valueCountMismatch() {
    PropertyRule rule{"angles", PropertyRuleType::HasPropertyWithCount, {3}, {}};
    PropertyRule candidate{"angles", PropertyRuleType::HasPropertyWithCount, {4}, {}};
    QCOMPARE(matchProperty(rule, candidate), false);
}

void test::matchProperty_t5_expectedValueMatch() {
    PropertyRule rule{"com_port", PropertyRuleType::HasPropertyWithValue, {}, {232}};
    PropertyRule candidate{"com_port", PropertyRuleType::HasPropertyWithValue, {}, {232}};
    QCOMPARE(matchProperty(rule, candidate), true);
}

void test::matchProperty_t6_expectedValueMismatch() {
    PropertyRule rule{"com_port", PropertyRuleType::HasPropertyWithValue, {}, {232}};
    PropertyRule candidate{"com_port", PropertyRuleType::HasPropertyWithValue, {}, {111}};
    QCOMPARE(matchProperty(rule, candidate), false);
}

void test::matchProperty_t7_expectedValueArrayMatch() {
    PropertyRule rule{"color_channels", PropertyRuleType::HasPropertyWithValues, {}, {255, 255, 255}};
    PropertyRule candidate{"color_channels", PropertyRuleType::HasPropertyWithValues, {}, {255, 255, 255}};
    QCOMPARE(matchProperty(rule, candidate), true);
}

void test::matchProperty_t8_expectedValueArrayMismatch() {
    PropertyRule rule{"color_channels", PropertyRuleType::HasPropertyWithValues, {}, {255, 255, 255}};
    PropertyRule candidate{"color_channels", PropertyRuleType::HasPropertyWithValues, {}, {255, 0, 0}};
    QCOMPARE(matchProperty(rule, candidate), false);
}

// Макрос Qt, создающий функцию main() для консольного запуска тестов
QTEST_APPLESS_MAIN(test)

#include "tst_test.moc"
