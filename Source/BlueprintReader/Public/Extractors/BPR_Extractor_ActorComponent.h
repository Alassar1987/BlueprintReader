#pragma once

#include "CoreMinimal.h"
#include "Core/BPR_Core.h"

class UActorComponent;
class UBlueprint;
class UEdGraph;
class UEdGraphNode;
class UEdGraphPin;

/**
 * Экстрактор для ActorComponent Blueprint.
 * Собирает переменные, функции и макросы компонента.
 * Результат возвращается в Core через FText.
 */
class BLUEPRINTREADER_API BPR_Extractor_ActorComponent
{
public:
	BPR_Extractor_ActorComponent();
	~BPR_Extractor_ActorComponent();

	/** Главная точка входа: обработка выбранного ассета */
	void ProcessComponent(UObject* SelectedObject, FBPR_ExtractedData& OutData);

private:
	// Логирование
	void LogMessage(const FString& Msg);
	void LogWarning(const FString& Msg);
	void LogError(const FString& Msg);

	// Основная обработка Blueprint
	void AppendBlueprintInfo(UBlueprint* Blueprint, FString& OutText);
	void AppendVariables(UBlueprint* Blueprint, FString& OutText);
	FString GetPropertyDefaultValue(FProperty* Property, UObject* Object);
	FString GetPropertyTypeDetailed(FProperty* Property);
	void AppendStructFields(FStructProperty* StructProp, FString& OutText, int32 Indent = 0);
	void AppendReplicationInfo(UClass* Class, FString& OutText);
	void AppendGraphs(UBlueprint* Blueprint, FString& OutText);
	
	void AppendGraphSequence(
	UEdGraph* Graph, 
	FString& OutExecText,
	FString& OutDataText
	);
	
	void ProcessNodeSequence(
	UEdGraphNode* Node, 
	int32 IndentLevel, 
	TSet<UEdGraphNode*>& Visited, 
	FString& OutExecText,
	FString& OutDataText
	);

	FString GetFunctionSignature(UEdGraph* Graph);
	FString GetMacroSignature(UEdGraph* Graph);
	FString GetReadableNodeName(UEdGraphNode* Node);
	FString GetPinDetails(UEdGraphPin* Pin);
	
	FString GetPinDisplayName(UEdGraphPin* Pin);
	FString CleanName(const FString& RawName);

	
	bool IsUserVariable(FProperty* Property);
};