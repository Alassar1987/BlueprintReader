#include "Extractors/BPR_Extractor_Actor.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "UObject/UnrealType.h"
#include "EdGraph/EdGraph.h"
#include "K2Node_Event.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CallDelegate.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Switch.h"
#include "K2Node_Tunnel.h"

//==============================================================================
//  Конструктор / Деструктор
//==============================================================================
BPR_Extractor_Actor::BPR_Extractor_Actor() {}
BPR_Extractor_Actor::~BPR_Extractor_Actor() {}


//==============================================================================
//  ProcessBlueprint — главная точка входа
//==============================================================================
void BPR_Extractor_Actor::ProcessBlueprint(UObject* SelectedObject, FText& OutText)
{
    if (!SelectedObject)
    {
        LogError(TEXT("SelectedObject is null!"));
        OutText = FText::FromString("Error: SelectedObject is null.");
        return;
    }

    UBlueprint* Blueprint = Cast<UBlueprint>(SelectedObject);
    if (!Blueprint)
    {
        LogWarning(TEXT("Selected object is not a Blueprint."));
        OutText = FText::FromString("Warning: Selected object is not a Blueprint.");
        return;
    }

    FString TmpText = FString::Printf(TEXT("# BPR Export for: %s\n\n"), *Blueprint->GetName());

    AppendVariables(Blueprint, TmpText);
    AppendGraphs(Blueprint, TmpText);

    OutText = FText::FromString(TmpText);
}


//==============================================================================
//  Логирование
//==============================================================================
void BPR_Extractor_Actor::LogMessage(const FString& Msg) { UE_LOG(LogTemp, Log, TEXT("[BPR] %s"), *Msg); }
void BPR_Extractor_Actor::LogWarning(const FString& Msg) { UE_LOG(LogTemp, Warning, TEXT("[BPR_Extractor] %s"), *Msg); }
void BPR_Extractor_Actor::LogError(const FString& Msg) { UE_LOG(LogTemp, Error, TEXT("[BPR_Extractor] %s"), *Msg); }


//==============================================================================
//  Переменные Blueprint
//==============================================================================
void BPR_Extractor_Actor::AppendVariables(UBlueprint* Blueprint, FString& OutText)
{
    if (!Blueprint || !Blueprint->GeneratedClass) return;

    OutText += TEXT("\n## Variables\n");

    UClass* Class = Blueprint->GeneratedClass;
    for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;
        if (!Property) continue;

        FString PropName = Property->GetName();
        FString PropType = Property->GetCPPType();
        FString DefaultValue = GetPropertyDefaultValue(Property, Class->GetDefaultObject());

        OutText += FString::Printf(TEXT("- **%s**: `%s` (Default: `%s`)\n"),
                                   *PropName, *PropType, *DefaultValue);
    }
}

FString BPR_Extractor_Actor::GetPropertyDefaultValue(FProperty* Property, UObject* Object)
{
    if (!Property || !Object) return TEXT("None");

    FString ValueStr;
    Property->ExportText_Direct(ValueStr,
        Property->ContainerPtrToValuePtr<void>(Object),
        Property->ContainerPtrToValuePtr<void>(Object),
        Object, 0);

    return ValueStr.IsEmpty() ? TEXT("None") : ValueStr;
}


//==============================================================================
//  Графы Blueprint
//==============================================================================
void BPR_Extractor_Actor::AppendGraphs(UBlueprint* Blueprint, FString& OutText)
{
    if (!Blueprint) return;

    OutText += TEXT("\n## Graphs\n");

    for (UEdGraph* Graph : Blueprint->UbergraphPages)
        AppendGraphSequence(Graph, OutText);
    for (UEdGraph* Graph : Blueprint->FunctionGraphs)
        AppendGraphSequence(Graph, OutText);
    for (UEdGraph* Graph : Blueprint->MacroGraphs)
        AppendGraphSequence(Graph, OutText);
}

void BPR_Extractor_Actor::ProcessEventGraph(UEdGraph* Graph, FString& OutText) { /* аналогично */ }
void BPR_Extractor_Actor::ProcessFunctionGraph(UEdGraph* Graph, FString& OutText) { /* аналогично */ }
void BPR_Extractor_Actor::ProcessMacroGraph(UEdGraph* Graph, FString& OutText) { /* аналогично */ }
FString BPR_Extractor_Actor::GetFunctionSignature(UEdGraph* Graph) { return TEXT("stub"); }


//==============================================================================
//  Последовательность узлов
//==============================================================================
void BPR_Extractor_Actor::AppendGraphSequence(UEdGraph* Graph, FString& OutText)
{
    if (!Graph) return;

    OutText += FString::Printf(TEXT("\n### Sequence: %s\n"), *Graph->GetName());

    TSet<UEdGraphNode*> Visited;
    UEdGraphNode* StartNode = nullptr;

    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (Cast<UK2Node_Event>(Node) || Cast<UK2Node_FunctionEntry>(Node))
        {
            StartNode = Node;
            break;
        }
    }

    if (!StartNode && Graph->Nodes.Num() > 0)
        StartNode = Graph->Nodes[0];

    if (StartNode)
        ProcessNodeSequence(StartNode, 1, Visited, OutText);
}

void BPR_Extractor_Actor::ProcessNodeSequence(
    UEdGraphNode* Node, int32 IndentLevel, TSet<UEdGraphNode*>& Visited, FString& OutText)
{
    if (!Node || Visited.Contains(Node))
    {
        if (Node)
            OutText += FString::Printf(TEXT("%*s[Loop Detected: %s]\n"), IndentLevel * 2, TEXT(""), *Node->GetName());
        return;
    }

    Visited.Add(Node);
    OutText += FString::Printf(TEXT("%*d. %s\n"), IndentLevel * 2, Visited.Num(), *GetReadableNodeName(Node));

    for (UEdGraphPin* Pin : Node->Pins)
    {
        if (Pin && Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
            for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
                if (LinkedPin && LinkedPin->GetOwningNode())
                    ProcessNodeSequence(LinkedPin->GetOwningNode(), IndentLevel + 1, Visited, OutText);
    }
}


//==============================================================================
//  Вспомогательные функции
//==============================================================================
FString BPR_Extractor_Actor::GetReadableNodeName(UEdGraphNode* Node)
{
    if (!Node) return TEXT("None");

    if (UK2Node_CustomEvent* CustomEvent = Cast<UK2Node_CustomEvent>(Node))
        if (CustomEvent->CustomFunctionName != NAME_None)
            return CustomEvent->CustomFunctionName.ToString();

    if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node))
    {
        FName EventName = EventNode->EventReference.GetMemberName();
        if (EventName != NAME_None) return EventName.ToString();
    }

    if (UK2Node_CallFunction* CallFunc = Cast<UK2Node_CallFunction>(Node))
        return FString::Printf(TEXT("CallFunction: %s"), *CallFunc->FunctionReference.GetMemberName().ToString());

    if (Cast<UK2Node_CallDelegate>(Node)) return TEXT("CallDelegate");
    if (Cast<UK2Node_IfThenElse>(Node)) return TEXT("Branch");
    if (Cast<UK2Node_Switch>(Node)) return TEXT("Switch");
    if (Cast<UK2Node_Tunnel>(Node)) return TEXT("Tunnel");
    if (Cast<UK2Node_FunctionEntry>(Node)) return TEXT("FunctionEntry");
    if (Cast<UK2Node_FunctionResult>(Node)) return TEXT("FunctionResult");

    return Node->GetName();
}
