#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTemporaryFile>
#include <QSet>

#include "structures.h"

bool validateInput(const QJsonObject& json, QSet<Error>& errors, QVector<Class>& parsedClasses);
bool matchProperty(const PropertyRule& rule, const PropertyRule& candidate);
bool isSubset(const ClassNode& A, const ClassNode& B);
void buildClassHierarchy(ClassHierarchy& hierarchy, const QVector<Class>& parsedClasses);
void removeTransitiveEdges(ClassHierarchy& hierarchy);


class test : public QObject
{
    Q_OBJECT

public:
    test();
    ~test();

private slots:

    // Тесты для функции validateInput

    void validateInput_t1_emptyJson();
    void validateInput_t2_missingClasses();
    void validateInput_t3_emptyClassesArray();
    void validateInput_t4_missingClassName();
    void validateInput_t5_missingProperties();
    void validateInput_t6_emptyProperties();
    void validateInput_t7_missingPropertyName();
    void validateInput_t8_ambiguousRule();
    void validateInput_t9_extraField();
    void validateInput_t10_invalidCharactersInClassName();
    void validateInput_t11_tooManyClasses();
    void validateInput_t12_invalidClassNameType();
    void validateInput_t13_emptyClassNameLength();
    void validateInput_t14_exceededClassNameLength();
    void validateInput_t15_invalidPropertiesType();
    void validateInput_t16_tooManyProperties();
    void validateInput_t17_invalidPropertyNameType();
    void validateInput_t18_emptyPropertyNameLength();
    void validateInput_t19_invalidValueCountType();
    void validateInput_t20_valueCountOutOfRange();
    void validateInput_t21_emptyExpectedValue();
    void validateInput_t22_tooManyExpectedValues();
    void validateInput_t23_expectedValueNotANumber();

    // Тесты для функции buildClassHierarchy

    void buildClassHierarchy_t1_simpleInheritance();
    void buildClassHierarchy_t2_multilevelHierarchy();
    void buildClassHierarchy_t3_independentClasses();
    void buildClassHierarchy_t4_diamondHierarchy();

    // Тесты для функции removeTransitiveEdges

    void removeTransitiveEdges_t1_removal();
    void removeTransitiveEdges_t2_noTransitivity();
    void removeTransitiveEdges_t3_multipleEdges();

    // Тесты для функции isSubset

    void isSubset_t1_fullMatch();
    void isSubset_t2_baseInDerived();
    void isSubset_t3_notAllIncluded();
    void isSubset_t4_completelyDifferent();
    void isSubset_t5_valueCountInheritance();
    void isSubset_t6_valueCountMismatch();
    void isSubset_t7_expectedValueInheritance();
    void isSubset_t8_expectedValueMismatch();

    // Тесты для функции matchProperty

    void matchProperty_t1_simplePropertyMatch();
    void matchProperty_t2_propertyNameMismatch();
    void matchProperty_t3_valueCountMatch();
    void matchProperty_t4_valueCountMismatch();
    void matchProperty_t5_expectedValueMatch();
    void matchProperty_t6_expectedValueMismatch();
    void matchProperty_t7_expectedValueArrayMatch();
    void matchProperty_t8_expectedValueArrayMismatch();
};

test::test() {}

test::~test() {}

static QJsonObject parseJson(const QString& text) {
    return QJsonDocument::fromJson(text.toUtf8()).object();
}

// Вспомогательная функция для проверки типа ошибки внутри вашего QSet<Error>
static bool hasError(const QSet<Error>& errors, ErrorType type) {
    for (const auto& err : errors) {
        if (err.type == type) return true;
    }
    return false;
}

// Вспомогательный метод для быстрого поиска связи в вашем графе QMap<QString, QList<QString>>
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
    QVector<Class> dummyParsedClasses; // Вторым аргументом передаем вектор (можно пустой, если логика построения идет чисто по nodes)

    ClassNode* nodeA = new ClassNode("Device");
    nodeA->properties.insert(PropertyRule{"power", PropertyRuleType::HasProperty, {}, {}});

    ClassNode* nodeB = new ClassNode("SmartDevice");
    nodeB->properties.insert(PropertyRule{"power", PropertyRuleType::HasProperty, {}, {}});
    nodeB->properties.insert(PropertyRule{"wifi", PropertyRuleType::HasProperty, {}, {}});

    hierarchy.classes.insert(nodeA);
    hierarchy.classes.insert(nodeB);

    // Теперь передаем оба параметра, как требует сигнатура
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

QTEST_APPLESS_MAIN(test)

#include "tst_test.moc"
