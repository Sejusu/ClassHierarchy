@echo off
chcp 65001 > nul

echo Поиск исполняемого файла buildClassHierarchy.exe...

:: 1. Сначала проверяем, вдруг exe лежит в той же папке, что и батник
if exist .\buildClassHierarchy.exe (
    set EXE_PATH=.\buildClassHierarchy.exe
    goto :FOUND
)

:: 2. Если нет, ищем в текущей папке и всех подпапках
for /f "delims=" %%i in ('dir /b /s .\buildClassHierarchy.exe 2^>nul') do (
    set EXE_PATH=%%i
    goto :FOUND
)

:: 3. Если и там нет, ищем на один уровень выше (в параллельных папках сборки Qt Creator)
for /f "delims=" %%i in ('dir /b /s ..\buildClassHierarchy.exe 2^>nul') do (
    set EXE_PATH=%%i
    goto :FOUND
)

:FOUND
if "%EXE_PATH%"=="" (
    echo [ОШИБКА] Не удалось автоматически найти файл buildClassHierarchy.exe.
    echo Пожалуйста, скомпилируйте проект в Qt Creator или укажите путь вручную!
    pause
    exit /b
)

echo Успешно найден исполняемый файл: %EXE_PATH%
set TEST_DIR=.\functional_tests
set LOG_FILE=%TEST_DIR%\tests_report.log

:: =========================================================================
:: АВТОМАТИЧЕСКАЯ НАСТРОЙКА ОКРУЖЕНИЯ QT
:: =========================================================================
set QT_BIN_DIR=C:\Qt\6.9.2\mingw_64\bin
set MINGW_BIN_DIR=C:\Qt\Tools\mingw1310_64\bin

if not exist "%MINGW_BIN_DIR%" set MINGW_BIN_DIR=C:\Qt\Tools\mingw1120_64\bin

set PATH=%QT_BIN_DIR%;%MINGW_BIN_DIR%;%PATH%
:: =========================================================================

if not exist %TEST_DIR% mkdir %TEST_DIR%
echo =================================================== > %LOG_FILE%
echo     ПОЛНЫЙ ИСЧЕРПЫВАЮЩИЙ ОТЧЕТ ПО СКВОЗНЫМ ТЕСТАМ    >> %LOG_FILE%
echo =================================================== >> %LOG_FILE%

echo Запуск комплексного стресс-тестирования программного модуля...
echo ---------------------------------------------------

:: =========================================================================
:: ГРУППА 1: СИСТЕМНЫЕ И ФАЙЛОВЫЕ ОШИБКИ (Тесты 1-3)
:: =========================================================================

echo ТЕСТ 1: Запуск без параметров
%EXE_PATH% > %TEST_DIR%\out_t1.txt 2> %TEST_DIR%\err_t1.txt
if %ERRORLEVEL% EQU 1 (
    echo Тест 1. Использование параметров: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 1. Использование параметров: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 2: Входной файл отсутствует (inputFileNotExist)
%EXE_PATH% %TEST_DIR%\missing_file.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t2.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t2.txt' -Pattern 'не существует или недоступен' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 2. inputFileNotExist: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 2. inputFileNotExist: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 3: Входной файл пуст (emptyInputFile)
copy /y nul %TEST_DIR%\empty.json > nul
%EXE_PATH% %TEST_DIR%\empty.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t3.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t3.txt' -Pattern 'пуст' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 3. emptyInputFile: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 3. emptyInputFile: СБОЙ >> %LOG_FILE%
)


:: =========================================================================
:: ГРУППА 2: СИНТАКСИС И СТРУКТУРА JSON (Тесты 4-6)
:: =========================================================================

echo ТЕСТ 4: Синтаксическая ошибка JSON (jsonSyntaxError)
echo { "classes": [ { "class_name": "A" ] } > %TEST_DIR%\syntax_err.json
%EXE_PATH% %TEST_DIR%\syntax_err.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t4.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t4.txt' -Pattern 'синтаксиса' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 4. jsonSyntaxError: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 4. jsonSyntaxError: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 5: Отсутствует или пуст массив классов (emptyClassesArray)
echo { "classes": [] } > %TEST_DIR%\empty_arr.json
%EXE_PATH% %TEST_DIR%\empty_arr.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t5.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t5.txt' -Pattern 'classes' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 5. emptyClassesArray: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 5. emptyClassesArray: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 6: Лишнее поле верхнего уровня (extraField)
echo { "classes": [ { "class_name": "A", "properties": [{"name": "p"}] }], "garbage": 123 } > %TEST_DIR%\extra_f.json
%EXE_PATH% %TEST_DIR%\extra_f.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t6.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t6.txt' -Pattern 'лишнее' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 6. extraField верхнего уровня: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 6. extraField верхнего уровня: СБОЙ >> %LOG_FILE%
)


:: =========================================================================
:: ГРУППА 3: ОШИБКИ И ОГРАНИЧЕНИЯ НА УРОВНЕ КЛАССА (Тесты 7-11)
:: =========================================================================

echo ТЕСТ 7: Отсутствует имя класса (missingClassName)
echo { "classes": [ { "properties": [{"name": "p"}] } ] } > %TEST_DIR%\missing_name.json
%EXE_PATH% %TEST_DIR%\missing_name.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t7.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t7.txt' -Pattern 'class_name' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 7. missingClassName: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 7. missingClassName: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 8: Отсутствует массив свойств (missingProperties)
echo { "classes": [ { "class_name": "A" } ] } > %TEST_DIR%\missing_props.json
%EXE_PATH% %TEST_DIR%\missing_props.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t8.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t8.txt' -Pattern 'properties' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 8. missingProperties: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 8. missingProperties: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 9: Массив свойств пуст (emptyProperties)
echo { "classes": [ { "class_name": "A", "properties": [] } ] } > %TEST_DIR%\empty_props.json
%EXE_PATH% %TEST_DIR%\empty_props.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t9.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t9.txt' -Pattern 'пуст' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 9. emptyProperties: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 9. emptyProperties: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 10: Превышена длина имени класса (invalidClassNameLength)
powershell -Command "$longName = 'B' * 256; echo \"{ `\"classes`\": [ { `\"class_name`\": `\"$longName`\", `\"properties`\": [ {`\"name`\": `\"p`\"} ] } ] }\"" > %TEST_DIR%\long_classname.json
%EXE_PATH% %TEST_DIR%\long_classname.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t10.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t10.txt' -Pattern 'длина имени класса' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 10. invalidClassNameLength: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 10. invalidClassNameLength: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 11: Недопустимые символы в имени класса (invalidCharacters)
echo { "classes": [ { "class_name": "Class@Bad", "properties": [ { "name": "p" } ] } ] } > %TEST_DIR%\bad_chars.json
%EXE_PATH% %TEST_DIR%\bad_chars.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t11.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t11.txt' -Pattern 'символ' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 11. invalidCharacters: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 11. invalidCharacters: СБОЙ >> %LOG_FILE%
)


:: =========================================================================
:: ГРУППА 4: ОШИБКИ И ОГРАНИЧЕНИЯ НА УРОВНЕ СВОЙСТВ (Тесты 12-19)
:: =========================================================================

echo ТЕСТ 12: Отсутствует имя свойства (missingPropertyName)
echo { "classes": [ { "class_name": "A", "properties": [ { "value_count": [1] } ] } ] } > %TEST_DIR%\missing_propname.json
%EXE_PATH% %TEST_DIR%\missing_propname.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t12.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t12.txt' -Pattern 'отсутствует обязательное поле' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Test 12. missingPropertyName: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Test 12. missingPropertyName: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 13: Логическое противоречие условий (ambiguousRule)
echo { "classes": [ { "class_name": "A", "properties": [ { "name": "p", "value_count": [1], "expected_value": [5] } ] } ] } > %TEST_DIR%\ambiguous.json
%EXE_PATH% %TEST_DIR%\ambiguous.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t13.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t13.txt' -Pattern 'противоречив' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 13. ambiguousRule: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 13. ambiguousRule: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 14: Неверный тип value_count (invalidValueCountType)
echo { "classes": [ { "class_name": "A", "properties": [ { "name": "p", "value_count": 5 } ] } ] } > %TEST_DIR%\bad_vcount_type.json
%EXE_PATH% %TEST_DIR%\bad_vcount_type.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t14.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t14.txt' -Pattern 'value_count' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 14. invalidValueCountType: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 14. invalidValueCountType: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 15: Неверный тип данных внутри expected_value (invalidValueType)
echo { "classes": [ { "class_name": "A", "properties": [ { "name": "p", "expected_value": ["str"] } ] } ] } > %TEST_DIR%\bad_val_type.json
%EXE_PATH% %TEST_DIR%\bad_val_type.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t15.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t15.txt' -Pattern 'expected_value' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 15. invalidValueType: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 15. invalidValueType: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 16: Значение выходит за верхний предел (invalidValueRange кусок 1)
echo { "classes": [ { "class_name": "A", "properties": [ { "name": "p", "expected_value": [1001] } ] } ] } > %TEST_DIR%\out_range_high.json
%EXE_PATH% %TEST_DIR%\out_range_high.json %TEST_DIR%\out.dot > %TEST_DIR%\out_range_high.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_range_high.txt' -Pattern 'диапазон' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 16. invalidValueRange Верхний: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 16. invalidValueRange Верхний: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 17: Значение выходит за нижний предел (invalidValueRange кусок 2)
echo { "classes": [ { "class_name": "A", "properties": [ { "name": "p", "expected_value": [0] } ] } ] } > %TEST_DIR%\out_range_low.json
%EXE_PATH% %TEST_DIR%\out_range_low.json %TEST_DIR%\out.dot > %TEST_DIR%\out_range_low.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_range_low.txt' -Pattern 'диапазон' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 17. invalidValueRange Нижний: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 17. invalidValueRange Нижний: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 18: Массив expected_value присутствует, но пуст (emptyExpectedValue)
echo { "classes": [ { "class_name": "A", "properties": [ { "name": "p", "expected_value": [] } ] } ] } > %TEST_DIR%\empty_expected.json
%EXE_PATH% %TEST_DIR%\empty_expected.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t18.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t18.txt' -Pattern 'expected_value' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 18. emptyExpectedValue: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 18. emptyExpectedValue: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 19: Слишком много элементов в expected_value (tooManyExpectedValues)
powershell -Command "$vals = 1..101 -join ','; echo \"{ `\"classes`\": [ { `\"class_name`\": `\"A`\", `\"properties`\": [ { `\"name`\": `\"p`\", `\"expected_value`\": [$vals] } ] } ] }\"" > %TEST_DIR%\too_many_vals.json
%EXE_PATH% %TEST_DIR%\too_many_vals.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t19.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t19.txt' -Pattern 'превышает лимит' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 19. tooManyExpectedValues: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 19. tooManyExpectedValues: СБОЙ >> %LOG_FILE%
)


:: =========================================================================
:: ГРУППА 5: СТРЕСС-ТЕСТЫ И ПРЕДЕЛЬНЫЕ ОБЪЕМЫ (Тесты 20-21)
:: =========================================================================

echo ТЕСТ 20: Превышен лимит количества классов (tooManyClasses)
powershell -Command "$arr = @(); for($i=1; $i -le 101; $i++){ $arr += \"{`\"class_name`\":`\"C$i`\",`\"properties`\":[{`\"name`\":`\"id`\"}]}\" }; $body = $arr -join ','; echo \"{`\"classes`\":[$body]}\"" > %TEST_DIR%\too_many_classes.json
%EXE_PATH% %TEST_DIR%\too_many_classes.json %TEST_DIR%\out.dot > %TEST_DIR%\out_t20.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\out_t20.txt' -Pattern 'Превышено максимальное количество' -CaseSensitive:$false -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 20. tooManyClasses: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 20. tooManyClasses: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 21: Длинный сложный JSON-файл (80 классов)
powershell -Command "$arr = @(); for($i=1; $i -le 80; $i++){ $prev = $i - 1; if($i -eq 1){ $arr += \"{`\"class_name`\":`\"C$i`\",`\"properties`\":[{`\"name`\":`\"p`\"}]}\" } else { $arr += \"{`\"class_name`\":`\"C$i`\",`\"properties`\":[{`\"name`\":`\"p`\"},{`\"name`\":`\"p$prev`\"}]}\" } }; $body = $arr -join ','; echo \"{`\"classes`\":[$body]}\"" > %TEST_DIR%\stress_valid.json
%EXE_PATH% %TEST_DIR%\stress_valid.json %TEST_DIR%\stress_output.dot > %TEST_DIR%\out_t21.txt
if %ERRORLEVEL% EQU 0 (
    powershell -Command "if (Select-String -Path '%TEST_DIR%\stress_output.dot' -Pattern 'C1' -Encoding utf8) { exit 0 } else { exit 1 }"
    if %ERRORLEVEL% EQU 0 (
        echo Тест 21. Длинный JSON: ПРОЙДЕН >> %LOG_FILE%
    ) else (
        echo Тест 21. Длинный JSON: СБОЙ структуры графа >> %LOG_FILE%
    )
) else (
    echo Тест 21. Длинный JSON: СБОЙ выполнения >> %LOG_FILE%
)


:: =========================================================================
:: ГРУППА 6: РАЗЛИЧНЫЕ ВИДЫ ИЕРАРХИЙ И АЛГОРИТМЫ ГРАФОВ (Тесты 22-25)
:: =========================================================================

echo ТЕСТ 22: Базовая иерархия (Одиночное наследование)
echo { "classes": [ { "class_name": "Base", "properties": [ { "name": "id" } ] }, { "class_name": "Derived", "properties": [ { "name": "id" }, { "name": "val" } ] } ] } > %TEST_DIR%\tree.json
%EXE_PATH% %TEST_DIR%\tree.json %TEST_DIR%\tree.dot > %TEST_DIR%\out_t22.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\tree.dot' -Pattern 'Base' -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 22. Одиночное наследование: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 22. Одиночное наследование: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 23: Множественное наследование (Ромб)
echo { "classes": [ { "class_name": "A", "properties": [{"name": "x"}] }, { "class_name": "B", "properties": [{"name": "x"},{"name": "y"}] }, { "class_name": "C", "properties": [{"name": "x"},{"name": "z"}] }, { "class_name": "D", "properties": [{"name": "x"},{"name": "y"},{"name": "z"},{"name": "w"}] } ] } > %TEST_DIR%\diamond.json
%EXE_PATH% %TEST_DIR%\diamond.json %TEST_DIR%\diamond.dot > %TEST_DIR%\out_t23.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\diamond.dot' -Pattern 'A' -Encoding utf8) { exit 0 } else { exit 1 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 23. Множественное наследование: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 23. Множественное наследование: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 24: Проверка удаления транзитивных ребер (Редукция графа)
echo { "classes": [ { "class_name": "A", "properties": [{"name": "1"}] }, { "class_name": "B", "properties": [{"name": "1"},{"name": "2"}] }, { "class_name": "C", "properties": [{"name": "1"},{"name": "2"},{"name": "3"}] } ] } > %TEST_DIR%\transitive.json
%EXE_PATH% %TEST_DIR%\transitive.json %TEST_DIR%\transitive.dot > %TEST_DIR%\out_t24.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\transitive.dot' -Pattern 'A -> C' -Encoding utf8) { exit 1 } else { exit 0 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 24. Редукция транзитивных связей: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 24. Редукция транзитивных связей: СБОЙ >> %LOG_FILE%
)

echo ТЕСТ 25: Изолированные несвязанные классы
echo { "classes": [ { "class_name": "A", "properties": [{"name": "x"}] }, { "class_name": "B", "properties": [{"name": "y"}] } ] } > %TEST_DIR%\isolated.json
%EXE_PATH% %TEST_DIR%\isolated.json %TEST_DIR%\isolated.dot > %TEST_DIR%\out_t25.txt
powershell -Command "if (Select-String -Path '%TEST_DIR%\isolated.dot' -Pattern '->' -Encoding utf8) { exit 1 } else { exit 0 }"
if %ERRORLEVEL% EQU 0 (
    echo Тест 25. Изолированные классы: ПРОЙДЕН >> %LOG_FILE%
) else (
    echo Тест 25. Изолированные классы: СБОЙ >> %LOG_FILE%
)

echo ---------------------------------------------------
echo Тестирование полностью завершено. Итоговый лог:
echo.
type %LOG_FILE%
pause