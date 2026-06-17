#ifndef HIERARCHY_H
#define HIERARCHY_H

#include "structures.h"

/*!
 * \brief Класс hierarchy отвечает за построение, оптимизацию и визуализацию графа иерархии классов.
 * * Данный класс реализует логику выявления отношений вложенности (подмножеств) между классами
 * на основе их правил-свойств, удаляет транзитивные связи для упрощения графа и генерирует
 * финальное представление структуры в формате DOT (Graphviz).
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
     * \param[in] hierarchy Иерархия классов для экспорта.
     * \return Текстовое описание графа на языке DOT.
     */
    QString generateDot(const ClassHierarchy& hierarchy);
};

/*!
     * \brief Строит ориентированный граф иерархии на основе распарсенных классов.
     * \param[out] hierarchy Объект иерархии, в который будут записаны построенные вершины и ребра.
     * \param[in] parsedClasses Список валидных классов, полученных после этапа синтаксического анализа.
     */
void buildClassHierarchy(ClassHierarchy& hierarchy, const QVector<Class>& parsedClasses);

/*!
     * \brief Удаляет транзитивные ребра из графа (транзитивное сокращение).
     * * Метод убирает избыточные связи. Например, если существуют ребра A -> B, B -> C и A -> C,
     * то ребро A -> C будет удалено, так как отношение выводится через вершину B.
     * * \param[in,out] hierarchy Иерархия классов, граф которой подлежит оптимизации.
     */
void removeTransitiveEdges(ClassHierarchy& hierarchy);

/*!
     * \brief Проверяет, является ли класс \a A строгим или нестрогим подмножеством класса \a B.
     * \param[in] A Потенциальный дочерний класс (узкоспециализированный).
     * \param[in] B Потенциальный родительский класс (базовый/более общий).
     * \return \c true, если все свойства \a B покрываются совместимыми свойствами \a A, иначе \c false.
     */
bool isSubset(const ClassNode& A, const ClassNode& B);

/*!
     * \brief Проверяет совместимость конкретного правила свойства с проверяемым кандидатом.
     * \param[in] rule Ограничение (правило) из эталонного или родительского класса.
     * \param[in] candidate Свойство из проверяемого дочернего класса.
     * \return \c true, если значения \a candidate удовлетворяют условиям \a rule, иначе \c false.
     */
bool matchProperty(const PropertyRule& rule, const PropertyRule& candidate);

#endif // HIERARCHY_H
