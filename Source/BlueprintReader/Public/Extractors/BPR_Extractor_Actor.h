#pragma once

//==============================================================================
//  BPR_Extractor_Actor
//==============================================================================
//
//  Экстрактор для Blueprint на основе AActor.
//  Цель:
//  - собрать переменные, функции, макросы, последовательности узлов;
//  - сформировать текстовый вывод для Core/окна плагина.
//
//==============================================================================

#include "CoreMinimal.h"

class UBlueprint;
class UEdGraph;
class UEdGraphNode;

class BLUEPRINTREADER_API BPR_Extractor_Actor
{
public:

    BPR_Extractor_Actor();
    ~BPR_Extractor_Actor();


    //--------------------------------------------------------------------------
    //  Основной метод
    //--------------------------------------------------------------------------

    /** Обрабатывает выбранный Blueprint и формирует Markdown-текст */
    void ProcessBlueprint(UObject* SelectedObject, FText& OutText);


private:

    //--------------------------------------------------------------------------
    //  Логирование
    //--------------------------------------------------------------------------

    void LogMessage(const FString& Msg);
    void LogWarning(const FString& Msg);
    void LogError(const FString& Msg);


    //--------------------------------------------------------------------------
    //  Переменные Blueprint
    //--------------------------------------------------------------------------

    void AppendVariables(UBlueprint* Blueprint, FString& OutText);
    FString GetPropertyDefaultValue(FProperty* Property, UObject* Object);


    //--------------------------------------------------------------------------
    //  Обработка графов
    //--------------------------------------------------------------------------

    void AppendGraphs(UBlueprint* Blueprint, FString& OutText);
    void ProcessEventGraph(UEdGraph* Graph, FString& OutText);
    void ProcessFunctionGraph(UEdGraph* Graph, FString& OutText);
    void ProcessMacroGraph(UEdGraph* Graph, FString& OutText);
    FString GetFunctionSignature(UEdGraph* Graph);


    //--------------------------------------------------------------------------
    //  Последовательность узлов
    //--------------------------------------------------------------------------

    void AppendGraphSequence(UEdGraph* Graph, FString& OutText);
    void ProcessNodeSequence(UEdGraphNode* Node, int32 IndentLevel,
                             TSet<UEdGraphNode*>& Visited, FString& OutText);


    //--------------------------------------------------------------------------
    //  Вспомогательные функции
    //--------------------------------------------------------------------------

    FString GetReadableNodeName(UEdGraphNode* Node);
};
