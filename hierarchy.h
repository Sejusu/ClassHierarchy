#ifndef HIERARCHY_H
#define HIERARCHY_H

#include "structures.h"

/**
 * @file hierarchy.h
 * @brief Заголовочный файл функций и классов иерархий
 */

/*!
 * \brief Класс hierarchy отвечает за построение, оптимизацию и визуализацию графа иерархии классов.
 *
 * Данный класс реализует логику выявления отношений вложенности (подмножеств) между классами
 * на основе их правил-свойств, удаляет транзитивные связи для упрощения графа и генерирует
 * финальное представление структуры в формате DOT (Graphviz).
 *
 * Основные этапы работы:
 * - \ref buildClassHierarchy - построение начального графа иерархии
 * - \ref removeTransitiveEdges - удаление транзитивных связей
 * - \ref generateDot - генерация DOT-представления
 *
 * \see buildClassHierarchy
 * \see removeTransitiveEdges
 * \see generateDot
 */
class hierarchy
{
public:
    /*!
     * \brief Конструктор по умолчанию класса hierarchy.
     */
    hierarchy();

    /*!
     * \brief Генерирует строку в формате DOT для последующей визуализации графа с помощью Graphviz.
     *
     * Используется после вызова \ref buildClassHierarchy и \ref removeTransitiveEdges.
     *
     * \param[in] hierarchy Иерархия классов для экспорта.
     * \return Текстовое описание графа на языке DOT.
     *
     * \see buildClassHierarchy
     * \see removeTransitiveEdges
     */
    QString generateDot(const ClassHierarchy& hierarchy);
};

/*!
 * \brief Строит ориентированный граф иерархии на основе распарсенных классов.
 *
 * Для каждой пары классов проверяется отношение подмножества с помощью \ref isSubset.
 * Если класс A является подмножеством класса B, добавляется ребро A -> B.
 *
 * \param[out] hierarchy Объект иерархии, в который будут записаны построенные вершины и ребра.
 * \param[in] parsedClasses Список валидных классов, полученных после этапа синтаксического анализа.
 *
 * \see isSubset
 * \see removeTransitiveEdges
 * \see hierarchy::generateDot
 */
void buildClassHierarchy(ClassHierarchy& hierarchy, const QVector<Class>& parsedClasses);

/*!
 * \brief Удаляет транзитивные ребра из графа (транзитивное сокращение).
 *
 * Метод убирает избыточные связи. Например, если существуют ребра A -> B, B -> C и A -> C,
 * то ребро A -> C будет удалено, так как отношение выводится через вершину B.
 *
 * Алгоритм использует:
 * - \ref isSubset для проверки отношений между классами
 * - Оптимизацию графа для устранения транзитивности
 *
 * \param[in,out] hierarchy Иерархия классов, граф которой подлежит оптимизации.
 *
 * \see buildClassHierarchy
 * \see isSubset
 * \see hierarchy::generateDot
 */
void removeTransitiveEdges(ClassHierarchy& hierarchy);

/*!
 * \brief Проверяет, является ли класс \a A строгим или нестрогим подмножеством класса \a B.
 *
 * Использует \ref matchProperty для сравнения каждого свойства из класса B
 * с соответствующим свойством в классе A.
 *
 * \param[in] A Потенциальный родительский класс (базовый, с меньшим числом свойств).
 * \param[in] B Потенциальный дочерний класс (производный, с большим числом свойств).
 * \return \c true, если все свойства \a A покрываются совместимыми свойствами \a B, иначе \c false.
 *
 * \see matchProperty
 * \see buildClassHierarchy
 * \see removeTransitiveEdges
 */
bool isSubset(const ClassNode& A, const ClassNode& B);

/*!
 * \brief Проверяет совместимость конкретного правила свойства с проверяемым кандидатом.
 *
 * Сравнивает имена свойств и проверяет условия в зависимости от типа правила:
 * - Тип 1 (HasProperty): только проверка имени
 * - Тип 2 (HasPropertyWithCount): проверка имени и количества значений
 * - Тип 3 (HasPropertyWithValue): проверка имени и наличия значения
 * - Тип 4 (HasPropertyWithValues): проверка имени и наличия всех значений
 *
 * Используется внутри \ref isSubset для проверки отдельных свойств.
 *
 * \param[in] rule Ограничение (правило) из эталонного или родительского класса.
 * \param[in] candidate Свойство из проверяемого дочернего класса.
 * \return \c true, если значения \a candidate удовлетворяют условиям \a rule, иначе \c false.
 *
 * \see isSubset
 */
bool matchProperty(const PropertyRule& rule, const PropertyRule& candidate);

#endif // HIERARCHY_H
