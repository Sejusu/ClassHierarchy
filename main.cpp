#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QDebug>
#include "structures.h"
#include "validator.h"
#include "hierarchy.h"

/*!
 * \brief Точка входа в консольное приложение.
 * * Обеспечивает разбор аргументов командной строки, чтение входного конфигурационного JSON-файла,
 * маршрутизацию процесса валидации, запуск построения иерархии, минимизацию дублирующих связей
 * графа и сохранение итоговой структуры в файл разметки DOT.
 * * \param[in] argc Количество аргументов командной строки.
 * \param[in] argv Массив указателей на строки аргументов.
 * \return Код завершения программы: \c 0 — успешное выполнение, \c 1 — критическая ошибка.
 */
int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // Проверка наличия обязательных аргументов командной строки
    if (argc < 3) {
        QTextStream errorStream(stderr);
        errorStream << "Использование: classHierarchy.exe <входной_файл.json> <выходной_файл.dot>" << Qt::endl;
        return 1;
    }

    QString inputFilePath = argv[1];
    QString outputFilePath = argv[2];

    // Попытка открытия и чтения исходного файла конфигурации
    QFile inputFile(inputFilePath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        Error err(ErrorType::inputFileNotExist, "", "", 0, 0, inputFilePath);
        QTextStream out(stdout);
        out << err.generateErrorMessage() << Qt::endl;
        return 1;
    }

    QByteArray data = inputFile.readAll();
    inputFile.close();

    // Валидация на пустой файл
    if (data.isEmpty()) {
        Error err(ErrorType::emptyInputFile, "", "", 0, 0, inputFilePath);
        QTextStream out(stdout);
        out << err.generateErrorMessage() << Qt::endl;
        return 1;
    }

    // Проверка синтаксической корректности JSON-документа
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        Error err(ErrorType::jsonSyntaxError, "", "", 0, 0, inputFilePath);
        QTextStream out(stdout);
        out << err.generateErrorMessage() << Qt::endl;
        return 1;
    }

    QSet<Error> errors;
    QVector<Class> parsedClasses;
    validator jsonValidator;

    // Запуск семантической и структурной проверки данных
    if (!jsonValidator.validateInput(jsonDoc.object(), errors, parsedClasses)) {
        QTextStream out(stdout);
        for (const Error& err : errors) {
            out << err.generateErrorMessage() << Qt::endl;
        }
        return 1;
    }

    // Построение и оптимизация графа
    hierarchy hierarchyManager;
    ClassHierarchy hierarchy;
    buildClassHierarchy(hierarchy, parsedClasses);
    removeTransitiveEdges(hierarchy);

    // Экспорт результатов в формат Graphviz DOT
    QString dotContent = hierarchyManager.generateDot(hierarchy);

    // Запись результирующего графа в файл
    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Error err(ErrorType::outputFileCreateFail, "", "", 0, 0, outputFilePath);
        QTextStream out(stdout);
        out << err.generateErrorMessage() << Qt::endl;
        return 1;
    }

    QTextStream out(&outputFile);
    out << dotContent;
    outputFile.close();

    return 0;
}
