#ifndef HIERARCHY_H
#define HIERARCHY_H

#include "structures.h"

class hierarchy
{
public:
    hierarchy();

    void buildClassHierarchy(ClassHierarchy& hierarchy, const QVector<Class>& parsedClasses);
    void removeTransitiveEdges(ClassHierarchy& hierarchy);
    QString generateDot(const ClassHierarchy& hierarchy);
    bool isSubset(const ClassNode& A, const ClassNode& B);
    bool matchProperty(const PropertyRule& rule, const PropertyRule& candidate);
};

#endif // HIERARCHY_H
