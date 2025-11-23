#pragma once

#include "CoreMinimal.h"

class UActorComponent;
class UBlueprint;
class UEdGraph;
class UEdGraphNode;

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
	void ProcessComponent(UObject* SelectedObject, FText& OutText);

private:
	// Логирование
	void LogMessage(const FString& Msg);
	void LogWarning(const FString& Msg);
	void LogError(const FString& Msg);

	// Основная обработка Blueprint
	void AppendVariables(UBlueprint* Blueprint, FString& OutText);
	FString GetPropertyDefaultValue(FProperty* Property, UObject* Object);
	void AppendGraphs(UBlueprint* Blueprint, FString& OutText);
	void ProcessFunctionGraph(UEdGraph* Graph, FString& OutText);
	void ProcessMacroGraph(UEdGraph* Graph, FString& OutText);
	FString GetFunctionSignature(UEdGraph* Graph);
	void AppendGraphSequence(UEdGraph* Graph, FString& OutText);
	void ProcessNodeSequence(UEdGraphNode* Node, int32 IndentLevel, TSet<UEdGraphNode*>& Visited, FString& OutText);

	// Вспомогательные функции
	FString GetReadableNodeName(UEdGraphNode* Node);
};
