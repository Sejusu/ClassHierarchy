#include "hierarchy.h"
#include <QStringList>

/*!
 * \brief Конструктор по умолчанию класса hierarchy.
 */
hierarchy::hierarchy() {}

/*!
 * \brief Внутренняя функция для проверки совпадения правил одного свойства.
 * * Сопоставляет тип правила, имя свойства, а также его внутреннее наполнение
 * (количество элементов или точные списки значений) в зависимости от строгости правила.
 * * \param[in] rule Базовое правило (ограничение родительского класса).
 * \param[in] candidate Проверяемое правило (свойство потенциального наследника).
 * \return \c true, если правила идентичны/совместимы, иначе \c false.
 */
bool matchProperty(const PropertyRule& rule, const PropertyRule& candidate) {
    if (rule.name != candidate.name) return false;
    if (rule.ruleType != candidate.ruleType) return false;

    if (rule.ruleType == PropertyRuleType::HasPropertyWithCount) {
        return rule.valueCount == candidate.valueCount;
    }
    if (rule.ruleType == PropertyRuleType::HasPropertyWithValue || rule.ruleType == PropertyRuleType::HasPropertyWithValues) {
        return rule.expectedValues == candidate.expectedValues;
    }
    return true;
}

/*!
 * \brief Внутренняя функция проверки отношения «базовый-производный» между классами.
 * * Класс \a A считается базовым (подмножеством) для \a B, если все свойства,
 * описанные в \a A, присутствуют в \a B и полностью удовлетворяют критериям функции matchProperty.
 * * \param[in] A Потенциальный базовый класс.
 * \param[in] B Потенциальный дочерний класс.
 * \return \c true, если класс \a A является подмножеством класса \a B, иначе \c false.
 */
bool isSubset(const ClassNode& A, const ClassNode& B) {
    if (A.properties.isEmpty()) return true;
    for (const PropertyRule& ruleA : A.properties) {
        bool found = false;
        for (const PropertyRule& ruleB : B.properties) {
            if (matchProperty(ruleA, ruleB)) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

/*!
 * \brief Реализация построения графа иерархии классов.
 * * Конвертирует первичные структуры \c Class во внутреннее представление вершин \c ClassNode,
 * выполняет попарное сравнение всех классов через отношение \c isSubset и формирует
 * первоначальную таблицу смежности связей (включая избыточные ребра).
 * * \param[out] hierarchy Объект иерархии для сохранения вершин и ребер.
 * \param[in] parsedClasses Вектор распарсенных и валидированных классов.
 */
void buildClassHierarchy(ClassHierarchy& hierarchy, const QVector<Class>& parsedClasses) {
    for (const Class& cls : parsedClasses) {
        ClassNode* node = new ClassNode(cls.name);
        for (const Property& prop : cls.properties) {
            PropertyRule rule;
            rule.name = prop.name;
            rule.ruleType = prop.ruleType;
            rule.valueCount = prop.valueCount.toList();
            rule.expectedValues = prop.expectedValues.toList();
            node->properties.insert(rule);
        }
        hierarchy.classes.insert(node);
    }

    for (ClassNode* A : hierarchy.classes) {
        for (ClassNode* B : hierarchy.classes) {
            if (A->className != B->className && isSubset(*A, *B)) {
                hierarchy.edges[A->className].append(B->className);
            }
        }
    }
}

/*!
 * \brief Реализация алгоритма транзитивного сокращения графа.
 * * Обходит граф и исключает избыточные ребра прямой вложенности, если связь между
 * вершинами может быть установлена транзитивно через промежуточные узлы (цепочки наследования).
 * Предотвращает поломку структуры при наличии циклических зависимостей (взаимного равенства подмножеств).
 * * \param[in,out] hierarchy Оптимизируемый объект иерархии.
 */
void removeTransitiveEdges(ClassHierarchy& hierarchy) {
    QMap<QString, QList<QString>> optimizedEdges = hierarchy.edges;

    for (ClassNode* nodeA : hierarchy.classes) {
        QString A = nodeA->className;
        if (!hierarchy.edges.contains(A)) continue;

        QList<QString> currentChildren = hierarchy.edges[A];

        for (const QString& B : currentChildren) {
            if (A == B) continue;

            if (hierarchy.edges.contains(B)) {
                const QList<QString>& bChildren = hierarchy.edges[B];

                for (const QString& C : bChildren) {
                    if (C == A || C == B) continue;

                    if (currentChildren.contains(C)) {
                        bool isCyclicAC = hierarchy.edges.contains(C) && hierarchy.edges[C].contains(A);
                        bool isCyclicAB = hierarchy.edges.contains(B) && hierarchy.edges[B].contains(A);

                        if (!isCyclicAC && !isCyclicAB) {
                            optimizedEdges[A].removeOne(C);
                        }
                    }
                }
            }
        }
    }

    hierarchy.edges = optimizedEdges;
}

/*!
 * \brief Формирует выходное представление графа в синтаксисе языка описания DOT.
 * \param[in] hierarchy Объект иерархии.
 * \return Строка в формате DOT для визуализации (Graphviz).
 */
QString hierarchy::generateDot(const ClassHierarchy& hierarchy) {
    QString dot = "digraph G {\n";
    for (auto it = hierarchy.edges.constBegin(); it != hierarchy.edges.constEnd(); ++it) {
        QString parent = it.key();
        for (const QString& child : it.value()) {
            dot += QString("    \"%1\" -> \"%2\";\n").arg(parent, child);
        }
    }
    dot += "}\n";
    return dot;
}
