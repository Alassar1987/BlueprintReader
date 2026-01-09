// Copyright (c) 2026 Racoon Coder. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BPR_Core.h"

// Forward declarations
class UActorComponent;
class UBlueprint;
class UEdGraph;
class UEdGraphNode;
class UEdGraphPin;

/**
 * Экстрактор для Actor Blueprint.
 * Собирает информацию о классе, переменных, компонентах, тегах и графах.
 * Результат возвращается в Core через FText.
 */
class BLUEPRINTREADER_API BPR_Extractor_Actor
{
public:
    BPR_Extractor_Actor();
    ~BPR_Extractor_Actor();

    // -------------------------------
    // Главная точка входа
    // -------------------------------
    /** Обрабатывает выбранный объект (Blueprint), формирует структуру и граф */
    void ProcessActor(UObject* SelectedObject, FBPR_ExtractedData& OutData);

private:
    // -------------------------------
    // Логирование
    // -------------------------------
    void LogMessage(const FString& Msg);
    void LogWarning(const FString& Msg);
    void LogError(const FString& Msg);

    // -------------------------------
    // Работа со структурой Blueprint
    // -------------------------------
    /** Общая информация о классе (имя, родитель, placeable) */
    void AppendClassInfo(UBlueprint* Blueprint, FString& OutText);
    
    /** Общая информация о Blueprint */
    void AppendBlueprintInfo(UBlueprint* Blueprint, FString& OutText);
    
    /** Информация о реплицируемых переменных */
    void AppendReplicationInfo(UClass* Class, FString& OutText);

    // -------------------------------
    // Переменные
    // -------------------------------
    /** Сбор пользовательских переменных в табличном формате */
    void AppendVariables(UBlueprint* Blueprint, FString& OutText);
    
    /** Возвращает дефолтное значение свойства */
    FString GetPropertyDefaultValue(FProperty* Property, UObject* Object);
    
    /** Возвращает детализированный тип свойства */
    FString GetPropertyTypeDetailed(FProperty* Property);
    
    /** Обрабатывает поля структурных свойств и выводит их информацию */
    void AppendStructFields(FStructProperty* StructProp, FString& OutText, int32 Indent = 0);
    
    /** Определяет, является ли свойство пользовательской переменной */
    bool IsUserVariable(FProperty* Property);

    // -------------------------------
    // Компоненты и теги
    // -------------------------------
    /** Сбор информации о компонентах актора */
    void AppendActorComponents(UBlueprint* Blueprint, FString& OutText);
    
    /** Сбор тегов актора */
    void AppendActorTags(UBlueprint* Blueprint, FString& OutText);
    
    /** Форматирует информацию о компоненте */
    FString FormatComponentInfo(UActorComponent* Component);

    // -------------------------------
    // Работа с графами
    // -------------------------------
    /** Основная точка обхода всех графов Blueprint */
    void AppendGraphs(UBlueprint* Blueprint, FString& OutText);
    
    /** Обход последовательности узлов графа (exec + data flow) */
    void AppendGraphSequence(UEdGraph* Graph, FString& OutExecText, FString& OutDataText);
    
    /** Рекурсивная обработка узлов графа */
    void ProcessNodeSequence(
        UEdGraphNode* Node, 
        int32 IndentLevel, 
        TSet<UEdGraphNode*>& Visited, 
        FString& OutExecText, 
        FString& OutDataText);

    /** Сбор подписи функции по входным/выходным пинам */
    FString GetFunctionSignature(UEdGraph* Graph);
    
    /** Сбор подписи макроса по Tunnel-узлам */
    FString GetMacroSignature(UEdGraph* Graph);

    // -------------------------------
    // Вспомогательные методы для узлов и пинов
    // -------------------------------
    /** Формирует читаемое имя узла */
    FString GetReadableNodeName(UEdGraphNode* Node);
    
    /** Детали пина (значение или ссылка) */
    FString GetPinDetails(UEdGraphPin* Pin);
    
    /** Читабельное имя пина */
    FString GetPinDisplayName(UEdGraphPin* Pin);
    
    /** Очищает имя от хвоста GUID */
    FString CleanName(const FString& RawName);
    
    /** Проверяет, является ли нода вычислительной (data-flow) */
    bool IsComputationalNode(UEdGraphNode* Node);
    
    /** Проверяет, есть ли у ноды Exec-вход */
    bool HasExecInput(UEdGraphNode* Node);
};