#include "hierarchy.h"
#include <QStringList>

hierarchy::hierarchy() {}

// Функция проверки совпадения правил одного свойства
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

// Функция проверки, является ли класс A базовым для класса B (на подмножество свойств)
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

// Первоначальное построение графа (все связи тип-подтип)
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

// Оптимизированное удаление избыточных (транзитивных) ребер
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
