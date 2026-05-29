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
