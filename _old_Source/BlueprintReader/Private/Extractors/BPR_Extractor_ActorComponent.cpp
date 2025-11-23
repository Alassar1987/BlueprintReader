#include "Extractors/BPR_Extractor_ActorComponent.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "UObject/UnrealType.h"
#include "EdGraph/EdGraph.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CallDelegate.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Switch.h"
#include "K2Node_Tunnel.h"
#include "K2Node_MacroInstance.h"


BPR_Extractor_ActorComponent::BPR_Extractor_ActorComponent() {}
BPR_Extractor_ActorComponent::~BPR_Extractor_ActorComponent() {}

//---------------------------------------------
// Главная точка входа
//---------------------------------------------
void BPR_Extractor_ActorComponent::ProcessComponent(UObject* SelectedObject, FText& OutText)
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
        LogWarning(TEXT("Selected object is not a Blueprint asset."));
        OutText = FText::FromString("Warning: Selected object is not a Blueprint.");
        return;
    }

    FString TmpText = FString::Printf(TEXT("# BP2MD Export for Component: %s\n\n"), *Blueprint->GetName());

    AppendVariables(Blueprint, TmpText);
    AppendGraphs(Blueprint, TmpText);

    OutText = FText::FromString(TmpText);
}

//---------------------------------------------
// Логирование
//---------------------------------------------
void BPR_Extractor_ActorComponent::LogMessage(const FString& Msg) { UE_LOG(LogTemp, Log, TEXT("[BPRBPR_Extractor_ActorComponent] %s"), *Msg); }
void BPR_Extractor_ActorComponent::LogWarning(const FString& Msg) { UE_LOG(LogTemp, Warning, TEXT("[BPRBPR_Extractor_ActorComponent] %s"), *Msg); }
void BPR_Extractor_ActorComponent::LogError(const FString& Msg) { UE_LOG(LogTemp, Error, TEXT("[BPRBPR_Extractor_ActorComponent] %s"), *Msg); }

//---------------------------------------------
// Переменные
//---------------------------------------------
void BPR_Extractor_ActorComponent::AppendVariables(UBlueprint* Blueprint, FString& OutText)
{
    if (!Blueprint || !Blueprint->GeneratedClass) return;

    OutText += TEXT("\n## Variables (Detailed)\n");

    UClass* Class = Blueprint->GeneratedClass;
    UObject* CDO = Class->GetDefaultObject();

    for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;
        if (!Property) continue;

        FString PropName   = Property->GetName();
        FString PropType   = Property->GetCPPType();
        FString DefaultVal = GetPropertyDefaultValue(Property, CDO);

        // ---------------------------
        // 1. Описание (ToolTip)
        // ---------------------------
        FString Description = Property->GetMetaData(TEXT("ToolTip"));
        if (Description.IsEmpty())
            Description = Property->GetMetaData(TEXT("DisplayName"));

        // ---------------------------
        // 2. Категория
        // ---------------------------
        FString Category = Property->GetMetaData(TEXT("Category"));
        if (Category.IsEmpty())
            Category = TEXT("None");

        // ---------------------------
        // 3. Флаги редактируемости
        // ---------------------------
        FString Flags;
        if (Property->HasAnyPropertyFlags(CPF_Edit))               Flags += TEXT("Edit ");
        if (Property->HasAnyPropertyFlags(CPF_BlueprintVisible))   Flags += TEXT("BlueprintVisible ");
        if (Property->HasAnyPropertyFlags(CPF_BlueprintReadOnly))  Flags += TEXT("BlueprintReadOnly ");
        if (Property->HasAnyPropertyFlags(CPF_BlueprintAssignable))Flags += TEXT("Assignable ");
        Flags = Flags.IsEmpty() ? TEXT("None") : Flags.TrimEnd();

        // ---------------------------
        // 4. Если переменная — компонент
        //    → проверяем физику, коллизию, mobility
        // ---------------------------
        FString ComponentFlags;

        if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Property))
        {
            UObject* ValueObj = ObjProp->GetObjectPropertyValue_InContainer(CDO);

            if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(ValueObj))
            {
                ComponentFlags += FString::Printf(
                    TEXT("Collision: %s, "),
                    *UEnum::GetValueAsString(PrimComp->GetCollisionEnabled())
                );

                ComponentFlags += FString::Printf(
                    TEXT("SimPhysics: %s, "),
                    PrimComp->IsSimulatingPhysics() ? TEXT("true") : TEXT("false")
                );

                ComponentFlags += FString::Printf(
                    TEXT("Mobility: %s"),
                    *UEnum::GetValueAsString(PrimComp->Mobility)
                );
            }
        }

        if (ComponentFlags.IsEmpty())
            ComponentFlags = TEXT("None");

        // ---------------------------
        // Формируем Markdown
        // ---------------------------
        OutText += FString::Printf(
            TEXT("- **%s**: `%s`\n"),
            *PropName, *PropType
        );

        OutText += FString::Printf(TEXT("  - Default: `%s`\n"), *DefaultVal);
        OutText += FString::Printf(TEXT("  - Category: `%s`\n"), *Category);
        OutText += FString::Printf(TEXT("  - Flags: `%s`\n"), *Flags);

        if (!Description.IsEmpty())
            OutText += FString::Printf(TEXT("  - Description: \"%s\"\n"), *Description);

        OutText += FString::Printf(TEXT("  - Component Settings: %s\n"), *ComponentFlags);

        OutText += TEXT("\n");
    }
}


FString BPR_Extractor_ActorComponent::GetPropertyDefaultValue(FProperty* Property, UObject* Object)
{
    if (!Property || !Object) return TEXT("None");

    FString ValueStr;
    Property->ExportText_Direct(ValueStr, Property->ContainerPtrToValuePtr<void>(Object),
        Property->ContainerPtrToValuePtr<void>(Object), Object, 0);

    return ValueStr.IsEmpty() ? TEXT("None") : ValueStr;
}

//---------------------------------------------
// Графы
//---------------------------------------------
void BPR_Extractor_ActorComponent::AppendGraphs(UBlueprint* Blueprint, FString& OutText)
{
    if (!Blueprint) return;

    OutText += TEXT("\n## Graphs\n");

    for (UEdGraph* Graph : Blueprint->FunctionGraphs)
        AppendGraphSequence(Graph, OutText);

    for (UEdGraph* Graph : Blueprint->MacroGraphs)
        AppendGraphSequence(Graph, OutText);
}

void BPR_Extractor_ActorComponent::ProcessFunctionGraph(UEdGraph* Graph, FString& OutText)
{
    if (!Graph) return;

    int32 NodeCount = Graph->Nodes.Num();
    FString Signature = GetFunctionSignature(Graph);

    OutText += FString::Printf(TEXT("### Function Graph: %s\n- Nodes: %d\n- Signature: %s\n\n"),
        *Graph->GetName(), NodeCount, *Signature);
}

void BPR_Extractor_ActorComponent::ProcessMacroGraph(UEdGraph* Graph, FString& OutText)
{
    if (!Graph) return;

    int32 NodeCount = Graph->Nodes.Num();
    TArray<FString> Inputs, Outputs;

    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (UK2Node_MacroInstance* MacroNode = Cast<UK2Node_MacroInstance>(Node))
        {
            for (UEdGraphPin* Pin : MacroNode->Pins)
            {
                FString PinDesc = FString::Printf(TEXT("%s: %s"), *Pin->PinName.ToString(), *Pin->PinType.PinCategory.ToString());
                if (Pin->Direction == EGPD_Output)
                    Inputs.Add(PinDesc);
                else if (Pin->Direction == EGPD_Input)
                    Outputs.Add(PinDesc);
            }
        }
    }

    OutText += FString::Printf(TEXT("### Macro Graph: %s\n- Nodes: %d\n- Inputs: %s\n- Outputs: %s\n\n"),
        *Graph->GetName(), NodeCount, *FString::Join(Inputs, TEXT(", ")), *FString::Join(Outputs, TEXT(", ")));
}

FString BPR_Extractor_ActorComponent::GetFunctionSignature(UEdGraph* Graph)
{
    if (!Graph) return TEXT("None");

    UK2Node_FunctionEntry* EntryNode = nullptr;
    UK2Node_FunctionResult* ResultNode = nullptr;

    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (!EntryNode) EntryNode = Cast<UK2Node_FunctionEntry>(Node);
        if (!ResultNode) ResultNode = Cast<UK2Node_FunctionResult>(Node);
        if (EntryNode && ResultNode) break;
    }

    TArray<FString> Inputs, Outputs;

    if (EntryNode)
        for (UEdGraphPin* Pin : EntryNode->Pins)
            if (Pin) Inputs.Add(FString::Printf(TEXT("%s: %s"), *Pin->PinName.ToString(), *Pin->PinType.PinCategory.ToString()));

    if (ResultNode)
        for (UEdGraphPin* Pin : ResultNode->Pins)
            if (Pin) Outputs.Add(FString::Printf(TEXT("%s: %s"), *Pin->PinName.ToString(), *Pin->PinType.PinCategory.ToString()));

    return FString::Printf(TEXT("(%s) -> (%s)"), *FString::Join(Inputs, TEXT(", ")), *FString::Join(Outputs, TEXT(", ")));
}

//---------------------------------------------
// Рекурсивная последовательность
//---------------------------------------------
void BPR_Extractor_ActorComponent::AppendGraphSequence(UEdGraph* Graph, FString& OutText)
{
    if (!Graph) return;

    OutText += FString::Printf(TEXT("\n### Sequence: %s\n"), *Graph->GetName());

    TSet<UEdGraphNode*> Visited;
    if (Graph->Nodes.Num() > 0)
        ProcessNodeSequence(Graph->Nodes[0], 1, Visited, OutText);
}

void BPR_Extractor_ActorComponent::ProcessNodeSequence(UEdGraphNode* Node, int32 IndentLevel, TSet<UEdGraphNode*>& Visited, FString& OutText)
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

//---------------------------------------------
// Вспомогательные
//---------------------------------------------
FString BPR_Extractor_ActorComponent::GetReadableNodeName(UEdGraphNode* Node)
{
    if (!Node) return TEXT("None");

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
