// Copyright (c) 2026 Racoon Coder. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BPR_Core.h"

class UMaterialFunction;
class UMaterialFunctionInstance;
class UMaterialExpression;
class UMaterialExpressionFunctionInput;
class UMaterialExpressionFunctionOutput;

/**
 * Экстрактор для Material Function и Material Function Instance.
 * Преобразует Function Graph в текстовое представление:
 *  - сигнатуру функции (Inputs / Outputs)
 *  - параметры и дефолтные значения
 *  - граф вычислений (Expression DAG)
 */
class BLUEPRINTREADER_API BPR_Extractor_MaterialFunction
{
public:

    BPR_Extractor_MaterialFunction();
    ~BPR_Extractor_MaterialFunction();

    // -------------------------------
    // Главная точка входа
    // -------------------------------
    /** Обрабатывает выбранный объект (MaterialFunction или MaterialFunctionInstance) */
    void ProcessMaterialFunction(UObject* SelectedObject, FBPR_ExtractedData& OutData);

private:

    // -------------------------------
    // Логирование
    // -------------------------------
    void LogMessage(const FString& Msg);
    void LogWarning(const FString& Msg);
    void LogError(const FString& Msg);

    // -------------------------------
    // Структура Material Function (декларативная часть)
    // -------------------------------
    /** Общая информация о Material Function */
    void AppendFunctionInfo(UMaterialFunction* Function, FString& OutText);

    /** Входные параметры функции (Function Inputs) */
    void AppendFunctionInputs(UMaterialFunction* Function, FString& OutText);

    /** Выходы функции (Function Outputs) */
    void AppendFunctionOutputs(UMaterialFunction* Function, FString& OutText);

    /** Информация о Function Instance и родительской функции */
    void AppendFunctionInstanceInfo(UMaterialFunctionInstance* Instance, FString& OutText);

    /** Переопределённые значения Function Instance */
    void AppendFunctionInstanceOverrides(UMaterialFunctionInstance* Instance, FString& OutText);

    // -------------------------------
    // Граф Material Function (вычислительная часть)
    // -------------------------------
    /** Точка входа обхода графа — все Function Outputs */
    void AppendFunctionGraph(UMaterialFunction* Function, FString& OutText);

    /** Обрабатывает конкретный Function Output */
    void AppendFunctionOutput(
        const FString& OutputName,
        const TArray<UMaterialExpression*>& DirectExpressions,
        FString& OutText,
        TMap<UMaterialExpression*, int32>& NodeIds,
        TMap<int32, FString>& NodeTexts,
        int32& NextId
    );

    /** Рекурсивный обход Expression-графа (data-flow) */
    void ProcessExpressionDAG(
        UMaterialExpression* Expression,
        TMap<UMaterialExpression*, int32>& NodeIds,
        TMap<int32, FString>& NodeTexts,
        int32& NextId
    );

    // -------------------------------
    // Expression / Input helpers
    // -------------------------------
    /** Читабельное имя Expression (тип + ключевые параметры) */
    FString GetReadableExpressionName(UMaterialExpression* Expression);

    /** Детализация входных пинов Expression */
    FString GetExpressionInputs(UMaterialExpression* Expression, int32 IndentLevel);

    /** Значение входа: связь или дефолт */
    FString GetInputValueDescription(const struct FExpressionInput& Input);

    /** Проверка: есть ли у Expression входящие связи */
    bool HasAnyInputs(UMaterialExpression* Expression);

    FString GetReadableNodeName(UMaterialExpression* Expr, int32 NodeId);

    // -------------------------------
    // Вспомогательные методы
    // -------------------------------
    /** Очистка имён от технических суффиксов */
    FString CleanName(const FString& RawName);

    /** Отступы для визуального отображения графа */
    FString MakeIndent(int32 Level);

    /** Проверка: является ли Expression технически прозрачным */
    bool IsTransparentExpression(UMaterialExpression* Expr);

    /** Проверка: является ли Expression логическим источником данных */
    bool IsLogicalSourceExpression(UMaterialExpression* Expr);

    /** Возвращает первый НЕ-прозрачный expression вверх по цепочке */
    UMaterialExpression* ResolveExpression(UMaterialExpression* Expr);
};
