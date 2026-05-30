#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QJsonObject>
#include <QJsonArray>
#include <QSet>
#include "structures.h"

class validator
{
public:
    validator();
    bool validateInput(const QJsonObject& json, QSet<Error>& errors, QVector<Class>& parsedClasses);
};

#endif // VALIDATOR_H
