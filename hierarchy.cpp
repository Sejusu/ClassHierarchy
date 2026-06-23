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
    // Если название правила не совпадает с н.п.кандидата или тип правила не совпадает с т.п.кандидата
    if (rule.name != candidate.name || rule.ruleType != candidate.ruleType) {
        return false;
    }
    // Если правило - количество свойств, проверить равны ли valueCount у эталона и кандидата
    if (rule.ruleType == PropertyRuleType::HasPropertyWithCount) {
        return rule.valueCount == candidate.valueCount;
    }
    // Если правило - заданное(ые) число(а), проверить равны ли ожидаемые числа у эталона и кандидата
    if (rule.ruleType == PropertyRuleType::HasPropertyWithValue || rule.ruleType == PropertyRuleType::HasPropertyWithValues) {
        return rule.expectedValues == candidate.expectedValues;
    }
    // Вернуть true
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
    if (A.properties.isEmpty() || B.properties.isEmpty()) return false;
    for (const PropertyRule& ruleA : A.properties) {
        bool found = false;
        // Поиск эквивалентного правила соответствия в потенциальном базовом классе Б
        for (const PropertyRule& ruleB : B.properties) {
            if (matchProperty(ruleA, ruleB)) {
                found = true;
                break;
            }
        }
        // Если хотя бы одно правило из Б не покрыто логикой класса А, отношение подмножества ложно
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
    QSet<QString> addedClassNames;

    for (const Class& cls : parsedClasses) {
        if (addedClassNames.contains(cls.name)) continue;
        addedClassNames.insert(cls.name);

        ClassNode* node = new ClassNode(cls.name);
        // Для каждого свойства
        for (const Property& prop : cls.properties) {
            // Добавить правило вывода
            PropertyRule rule;
            rule.name = prop.name;
            rule.ruleType = prop.ruleType;
            rule.valueCount = prop.valueCount.toList();
            rule.expectedValues = prop.expectedValues.toList();
            node->properties.insert(rule);
        }
        // Добавить в иерархию узел класса
        hierarchy.classes.insert(node);

        if (!hierarchy.edges.contains(cls.name)) {
            hierarchy.edges[cls.name] = QList<QString>();
        }

    }

    // Полный обход пар вершин для выявления отношений вложенности свойств
    for (ClassNode* A : hierarchy.classes) {
        for (ClassNode* B : hierarchy.classes) {
            if (A->className == B->className) continue;
            if (A->properties.isEmpty() || B->properties.isEmpty()) continue;
            if (A->properties.size() >= B->properties.size()) continue;

            if (isSubset(*A, *B)) {
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
    // Создание рабочей копии графа ребер
    QMap<QString, QList<QString>> optimizedEdges = hierarchy.edges;

    // Для каждого класса А
    for (ClassNode* nodeA : hierarchy.classes) {
        QString A = nodeA->className;

        // Объявить список потомков
        QList<QString> currentChildren = hierarchy.edges[A];

        // Проверка каждого прямого потомка B на наличие собственных связей
        for (const QString& B : currentChildren) {
            if (A == B) continue;

            // Если у класса Б есть свои наследники
            if (hierarchy.edges.contains(B)) {
                const QList<QString>& bChildren = hierarchy.edges[B];

                // Обход наследников «второго уровня»
                for (const QString& C : bChildren) {
                    if (C == A || C == B) continue;

                    // Если класс С достижим напрямую из А, прямая связь (A -> C) удаляется
                    if (currentChildren.contains(C)) {
                        bool isCyclicAC = hierarchy.edges.contains(C) && hierarchy.edges[C].contains(A);
                        bool isCyclicAB = hierarchy.edges.contains(B) && hierarchy.edges[B].contains(A);

                        // Удаление избыточного ребра разрешено только при отсутствии циклов
                        if (!isCyclicAC && !isCyclicAB) {
                            optimizedEdges[A].removeOne(C);
                        }
                    }
                }
            }
        }
    }

    // Перенос оптимизированной структуры ребер обратно в итоговый граф
    hierarchy.edges = optimizedEdges;
}

/*!
 * \brief Формирует выходное представление графа в синтаксисе языка описания DOT.
 * \param[in] hierarchy Объект иерархии.
 * \return Строка в формате DOT для визуализации (Graphviz).
 */
/*!
 * \brief Формирует выходное представление графа в синтаксисе языка описания DOT.
 * \param[in] hierarchy Объект иерархии.
 * \return Строка в формате DOT для визуализации (Graphviz).
 */
QString hierarchy::generateDot(const ClassHierarchy& hierarchy) {
    // Добавить строку "digraph G {"
    QString dot = "digraph G {\n";

    // Объявить множество неизолированных классов
    QSet<QString> connectedClasses;
    // Для каждой связи
    for (auto it = hierarchy.edges.constBegin(); it != hierarchy.edges.constEnd(); ++it) {
        // Для каждого потомка
        for (const QString& child : it.value()) {
            // Добавить родителя и потомка
            connectedClasses.insert(it.key());
            connectedClasses.insert(child);
        }
    }
    // Для каждого класса
    for (ClassNode* node : hierarchy.classes) {
        // Если неизолированные классы не содержат имя класса
        if (!connectedClasses.contains(node->className)) {
            // Добавить название класса
            dot += QString("    \"%1\";\n").arg(node->className);
        }
    }
    // Для каждой связи
    for (auto it = hierarchy.edges.constBegin(); it != hierarchy.edges.constEnd(); ++it) {
        // Для каждого потомка
        for (const QString& child : it.value()) {
            // Добавить строку "родитель->потомок"
            dot += QString("    \"%1\" -> \"%2\";\n").arg(it.key(), child);
        }
    }
    // Добавить строку "}"
    dot += "}\n";
    // Вернуть dot
    return dot;
}
