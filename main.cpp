#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QDebug>
#include "structures.h"
#include "validator.cpp"
#include "hierarchy.cpp"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    if (argc < 3) {
        QTextStream errorStream(stderr);
        errorStream << "Использование: classHierarchy.exe <входной_файл.json> <выходной_файл.dot>" << Qt::endl;
        return 1;
    }

    QString inputFilePath = argv[1];
    QString outputFilePath = argv[2];

    QFile inputFile(inputFilePath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        Error err(ErrorType::inputFileNotExist);
        QTextStream out(stdout);
        out << err.generateErrorMessage() << Qt::endl;
        return 1;
    }

    QByteArray data = inputFile.readAll();
    inputFile.close();

    if (data.isEmpty()) {
        Error err(ErrorType::emptyInputFile);
        QTextStream out(stdout);
        out << err.generateErrorMessage() << Qt::endl;
        return 1;
    }

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        Error err(ErrorType::jsonSyntaxError);
        QTextStream out(stdout);
        out << err.generateErrorMessage() << Qt::endl;
        return 1;
    }

    QSet<Error> errors;
    QVector<Class> parsedClasses;

    if (!validateInput(jsonDoc.object(), errors, parsedClasses)) {
        QTextStream out(stdout);
        for (const Error& err : errors) {
            out << err.generateErrorMessage() << Qt::endl;
        }
        return 1;
    }

    ClassHierarchy hierarchy;
    buildClassHierarchy(hierarchy, parsedClasses);
    removeTransitiveEdges(hierarchy);

    QString dotContent = generateDot(hierarchy);

    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Error err(ErrorType::outputFileCreateFail);
        QTextStream out(stdout);
        out << err.generateErrorMessage() << Qt::endl;
        return 1;
    }

    QTextStream out(&outputFile);
    out << dotContent;
    outputFile.close();

    return 0;
}
