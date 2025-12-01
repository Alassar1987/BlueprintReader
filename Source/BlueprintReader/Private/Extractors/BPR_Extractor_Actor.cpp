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
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "UObject/Package.h"

BPR_Extractor_Actor::BPR_Extractor_Actor() {}
BPR_Extractor_Actor::~BPR_Extractor_Actor() {}

//==============================================================================
//  ProcessActor — главная точка входа
//==============================================================================
void BPR_Extractor_Actor::ProcessActor(UObject* SelectedObject, FBPR_ExtractedData& OutData)
{
    if (!SelectedObject)
    {
        LogError(TEXT("SelectedObject is null!"));
        OutData.Structure = FText::FromString("Error: SelectedObject is null.");
        OutData.Graph     = FText::FromString("Error: SelectedObject is null.");
        return;
    }

    UBlueprint* Blueprint = Cast<UBlueprint>(SelectedObject);
    if (!Blueprint)
    {
        LogWarning(TEXT("Selected object is not a Blueprint asset."));
        OutData.Structure = FText::FromString("Warning: Selected object is not a Blueprint.");
        OutData.Graph     = FText::FromString("Warning: Selected object is not a Blueprint.");
        return;
    }

    FString TmpStructure = FString::Printf(TEXT("# Structure Export for Actor: %s\n\n"), *Blueprint->GetName());
    FString TmpGraph     = FString::Printf(TEXT("# Graphs Export for Actor: %s\n\n"), *Blueprint->GetName());

    // Сбор общей информации о классе
    AppendClassInfo(Blueprint, TmpStructure);

    // Переменные
    AppendVariables(Blueprint, TmpStructure);

    // Компоненты актора (CDO)
    AppendActorComponents(Blueprint, TmpStructure);

    // Теги
    AppendActorTags(Blueprint, TmpStructure);

    // Графы
    AppendGraphs(Blueprint, TmpGraph);

    OutData.Structure = FText::FromString(TmpStructure);
    OutData.Graph     = FText::FromString(TmpGraph);

    LogMessage(FString::Printf(TEXT("Actor extraction finished for %s"), *Blueprint->GetName()));
}

//==============================================================================
//  Логирование
//==============================================================================
void BPR_Extractor_Actor::LogMessage(const FString& Msg) { UE_LOG(LogTemp, Log, TEXT("[BPR_Extractor_Actor] %s"), *Msg); }
void BPR_Extractor_Actor::LogWarning(const FString& Msg) { UE_LOG(LogTemp, Warning, TEXT("[BPR_Extractor_Actor] %s"), *Msg); }
void BPR_Extractor_Actor::LogError(const FString& Msg) { UE_LOG(LogTemp, Error, TEXT("[BPR_Extractor_Actor] %s"), *Msg); }

//==============================================================================
//  Class info
//==============================================================================
void BPR_Extractor_Actor::AppendClassInfo(UBlueprint* Blueprint, FString& OutText)
{
    if (!Blueprint || !Blueprint->GeneratedClass) return;

    UClass* Class = Blueprint->GeneratedClass;

    OutText += TEXT("## Class Info\n");

    FString Parent = Class->GetSuperClass() ? Class->GetSuperClass()->GetName() : TEXT("None");
    FString NativeParent = Class->GetSuperClass() ? Class->GetSuperClass()->GetPathName() : TEXT("None");

    OutText += FString::Printf(TEXT("- Class: %s\n"), *Class->GetName());
    OutText += FString::Printf(TEXT("- ParentClass: %s\n"), *Parent);
    OutText += FString::Printf(TEXT("- NativeParentPath: %s\n"), *NativeParent);

    // Blueprint type / placeable info
    bool bIsPlaceable = (Class->ClassFlags & CLASS_NotPlaceable) == 0;
    OutText += FString::Printf(TEXT("- Placeable: %s\n"), bIsPlaceable ? TEXT("True") : TEXT("False"));

    OutText += TEXT("\n");
}

//==============================================================================
//  Переменные Blueprint (перенят стиль из ComponentExtractor)
//==============================================================================
void BPR_Extractor_Actor::AppendVariables(UBlueprint* Blueprint, FString& OutText)
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

        // Description (ToolTip or DisplayName)
        FString Description = Property->GetMetaData(TEXT("ToolTip"));
        if (Description.IsEmpty())
            Description = Property->GetMetaData(TEXT("DisplayName"));

        // Category
        FString Category = Property->GetMetaData(TEXT("Category"));
        if (Category.IsEmpty())
            Category = TEXT("None");

        // Flags
        FString Flags;
        if (Property->HasAnyPropertyFlags(CPF_Edit))               Flags += TEXT("Edit ");
        if (Property->HasAnyPropertyFlags(CPF_BlueprintVisible))   Flags += TEXT("BlueprintVisible ");
        if (Property->HasAnyPropertyFlags(CPF_BlueprintReadOnly))  Flags += TEXT("BlueprintReadOnly ");
        if (Property->HasAnyPropertyFlags(CPF_BlueprintAssignable))Flags += TEXT("Assignable ");
        Flags = Flags.IsEmpty() ? TEXT("None") : Flags.TrimEnd();

        // Component-specific info (если property указывает на компонент-объект)
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

        // Markdown output
        OutText += FString::Printf(TEXT("- **%s**: `%s`\n"), *PropName, *PropType);
        OutText += FString::Printf(TEXT("  - Default: `%s`\n"), *DefaultVal);
        OutText += FString::Printf(TEXT("  - Category: `%s`\n"), *Category);
        OutText += FString::Printf(TEXT("  - Flags: `%s`\n"), *Flags);
        if (!Description.IsEmpty())
            OutText += FString::Printf(TEXT("  - Description: \"%s\"\n"), *Description);
        OutText += FString::Printf(TEXT("  - Component Settings: %s\n"), *ComponentFlags);
        OutText += TEXT("\n");
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
//  Actor Components (CDO-based)
//==============================================================================
void BPR_Extractor_Actor::AppendActorComponents(UBlueprint* Blueprint, FString& OutText)
{
    if (!Blueprint || !Blueprint->GeneratedClass) return;

    OutText += TEXT("\n## Actor Components\n");

    UClass* Class = Blueprint->GeneratedClass;
    UObject* CDO = Class->GetDefaultObject();

    if (AActor* ActorCDO = Cast<AActor>(CDO))
    {
        TArray<UActorComponent*> Comps;
        ActorCDO->GetComponents(Comps);

        for (UActorComponent* Comp : Comps)
        {
            if (!Comp) continue;

            FString CompName = Comp->GetName();
            FString CompClass = Comp->GetClass()->GetName();
            OutText += FString::Printf(TEXT("- **%s**: `%s`\n"), *CompName, *CompClass);

            // Если это SceneComponent / PrimitiveComponent — выводим attach, mobility, collision, physics
            if (USceneComponent* SceneComp = Cast<USceneComponent>(Comp))
            {
                FString AttachParent = TEXT("None");
                if (SceneComp->GetAttachParent())
                    AttachParent = SceneComp->GetAttachParent()->GetName();

                OutText += FString::Printf(TEXT("  - AttachParent: %s\n"), *AttachParent);

                if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(SceneComp))
                {
                    OutText += FString::Printf(TEXT("  - Collision: %s\n"),
                        *UEnum::GetValueAsString(Prim->GetCollisionEnabled()));
                    OutText += FString::Printf(TEXT("  - SimPhysics: %s\n"),
                        Prim->IsSimulatingPhysics() ? TEXT("true") : TEXT("false"));
                    OutText += FString::Printf(TEXT("  - Mobility: %s\n"),
                        *UEnum::GetValueAsString(Prim->Mobility));
                }
            }

            OutText += TEXT("\n");
        }
    }
    else
    {
        OutText += TEXT("No Actor CDO available to enumerate components.\n\n");
    }
}

//==============================================================================
//  Tags
//==============================================================================
void BPR_Extractor_Actor::AppendActorTags(UBlueprint* Blueprint, FString& OutText)
{
    if (!Blueprint || !Blueprint->GeneratedClass) return;

    UClass* Class = Blueprint->GeneratedClass;
    UObject* CDO = Class->GetDefaultObject();

    if (AActor* ActorCDO = Cast<AActor>(CDO))
    {
        OutText += TEXT("\n## Tags\n");
        for (const FName& Tag : ActorCDO->Tags)
        {
            OutText += FString::Printf(TEXT("- %s\n"), *Tag.ToString());
        }
        OutText += TEXT("\n");
    }
}

//==============================================================================
//  Graphs (Ubergraph / Functions / Macros)
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

void BPR_Extractor_Actor::ProcessEventGraph(UEdGraph* Graph, FString& OutText)
{
    // можно расширить параллельно с компонентным экстрактором
    if (!Graph) return;

    int32 NodeCount = Graph->Nodes.Num();
    OutText += FString::Printf(TEXT("### Event Graph: %s\n- Nodes: %d\n\n"), *Graph->GetName(), NodeCount);
}

void BPR_Extractor_Actor::ProcessFunctionGraph(UEdGraph* Graph, FString& OutText)
{
    if (!Graph) return;

    int32 NodeCount = Graph->Nodes.Num();
    FString Signature = GetFunctionSignature(Graph);

    OutText += FString::Printf(TEXT("### Function Graph: %s\n- Nodes: %d\n- Signature: %s\n\n"),
        *Graph->GetName(), NodeCount, *Signature);
}

void BPR_Extractor_Actor::ProcessMacroGraph(UEdGraph* Graph, FString& OutText)
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

FString BPR_Extractor_Actor::GetFunctionSignature(UEdGraph* Graph)
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

//==============================================================================
//  Рекурсивная последовательность
//==============================================================================
void BPR_Extractor_Actor::AppendGraphSequence(UEdGraph* Graph, FString& OutText)
{
    if (!Graph) return;

    OutText += FString::Printf(TEXT("\n### Sequence: %s\n"), *Graph->GetName());

    TSet<UEdGraphNode*> Visited;
    UEdGraphNode* StartNode = nullptr;

    // Найдём подходящую стартовую ноду: Event или FunctionEntry
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

void BPR_Extractor_Actor::ProcessNodeSequence(UEdGraphNode* Node, int32 IndentLevel, TSet<UEdGraphNode*>& Visited, FString& OutText)
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
//  Вспомогательные
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
