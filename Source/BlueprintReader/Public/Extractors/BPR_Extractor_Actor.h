#pragma once

//==============================================================================
//  BPR_Extractor_Actor
//==============================================================================
//
//  Экстрактор для Blueprint на основе AActor.
//  Сбор переменных, компонентов, графов и последовательностей узлов.
//  Возвращает результат в FBPR_ExtractedData (Structure и Graph).
//
//==============================================================================

#include "CoreMinimal.h"
#include "Core/BPR_Core.h" // FBPR_ExtractedData

class UBlueprint;
class UEdGraph;
class UEdGraphNode;

class BLUEPRINTREADER_API BPR_Extractor_Actor
{
public:
    BPR_Extractor_Actor();
    ~BPR_Extractor_Actor();

    /** Главная точка входа: обработка выбранного ассета */
    void ProcessActor(UObject* SelectedObject, FBPR_ExtractedData& OutData);

private:
    // Логирование
    void LogMessage(const FString& Msg);
    void LogWarning(const FString& Msg);
    void LogError(const FString& Msg);

    // Переменные Blueprint
    void AppendVariables(UBlueprint* Blueprint, FString& OutText);
    FString GetPropertyDefaultValue(FProperty* Property, UObject* Object);

    // Actor-specific: компоненты, теги, класс-инфа
    void AppendActorComponents(UBlueprint* Blueprint, FString& OutText);
    void AppendActorTags(UBlueprint* Blueprint, FString& OutText);
    void AppendClassInfo(UBlueprint* Blueprint, FString& OutText);

    // Обработка графов
    void AppendGraphs(UBlueprint* Blueprint, FString& OutText);
    void ProcessEventGraph(UEdGraph* Graph, FString& OutText);
    void ProcessFunctionGraph(UEdGraph* Graph, FString& OutText);
    void ProcessMacroGraph(UEdGraph* Graph, FString& OutText);
    FString GetFunctionSignature(UEdGraph* Graph);

    // Последовательность узлов
    void AppendGraphSequence(UEdGraph* Graph, FString& OutText);
    void ProcessNodeSequence(UEdGraphNode* Node, int32 IndentLevel, TSet<UEdGraphNode*>& Visited, FString& OutText);

    // Вспомогательные
    FString GetReadableNodeName(UEdGraphNode* Node);
};
