#pragma once

#include "CoreMinimal.h"
#include "Core/BPR_Core.h"

// Forward declaration достаточна
class UActorComponent;
class UBlueprint;
class UEdGraph;
class UEdGraphNode;
class UEdGraphPin;


/**
 * Экстрактор для Actor Blueprint
 */

class BLUEPRINTREADER_API BPR_Extractor_Actor
{

public:
    BPR_Extractor_Actor();
    ~BPR_Extractor_Actor();

    void ProcessActor(UObject* SelectedObject, FBPR_ExtractedData& OutData);

private:
    // ======================
    // Logging
    // ======================
    void LogMessage(const FString& Msg);
    void LogWarning(const FString& Msg);
    void LogError(const FString& Msg);

    // ======================
    // Class info / Blueprint info
    // ======================
    void AppendClassInfo(UBlueprint* Blueprint, FString& OutText);
    void AppendBlueprintInfo(UBlueprint* Blueprint, FString& OutText);
    void AppendReplicationInfo(UClass* Class, FString& OutText);

    // ======================
    // Variables
    // ======================
    void AppendVariables(UBlueprint* Blueprint, FString& OutText);
    FString GetPropertyDefaultValue(FProperty* Property, UObject* Object);
    FString GetPropertyValueAsString(FProperty* Property, UObject* Object);
    FString GetPropertyTypeDetailed(FProperty* Property);
    bool IsUserVariable(FProperty* Property);
    void AppendStructFields(FStructProperty* StructProp, FString& OutText, int32 Indent = 0);

    // ======================
    // Components & Tags
    // ======================
    void AppendActorComponents(UBlueprint* Blueprint, FString& OutText);
    void AppendActorTags(UBlueprint* Blueprint, FString& OutText);
    FString FormatComponentInfo(UActorComponent* Component);

    // ======================
    // Graphs
    // ======================
    void AppendGraphs(UBlueprint* Blueprint, FString& OutText);
    void AppendGraphSequence(UEdGraph* Graph, FString& OutExecText, FString& OutDataText);
    void ProcessNodeSequence(UEdGraphNode* Node, int32 IndentLevel, TSet<UEdGraphNode*>& Visited, FString& OutExecText, FString& OutDataText);

    FString GetFunctionSignature(UEdGraph* Graph);
    FString GetMacroSignature(UEdGraph* Graph);

    FString GetReadableNodeName(UEdGraphNode* Node);
    bool IsComputationalNode(UEdGraphNode* Node);
    bool HasExecInput(UEdGraphNode* Node);

    FString GetPinDetails(UEdGraphPin* Pin);
    FString GetPinDisplayName(UEdGraphPin* Pin);
    FString CleanName(const FString& RawName);
};
