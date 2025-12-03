#include "Extractors/BPR_Extractor_ActorComponent.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "UObject/UnrealType.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CallDelegate.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Switch.h"
#include "K2Node_Tunnel.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Net/UnrealNetwork.h"

BPR_Extractor_ActorComponent::BPR_Extractor_ActorComponent() {}
BPR_Extractor_ActorComponent::~BPR_Extractor_ActorComponent() {}

//---------------------------------------------
// Главная точка входа
//---------------------------------------------
void BPR_Extractor_ActorComponent::ProcessComponent(UObject* SelectedObject, FBPR_ExtractedData& OutData)
{
    if (!SelectedObject)
    {
        LogError(TEXT("SelectedObject is null!"));
        OutData.Structure = FText::FromString("Error: SelectedObject is null.");
        OutData.Graph = FText::FromString("Error: SelectedObject is null.");
        return;
    }

    UBlueprint* Blueprint = Cast<UBlueprint>(SelectedObject);
    if (!Blueprint)
    {
        LogWarning(TEXT("Selected object is not a Blueprint asset."));
        OutData.Structure = FText::FromString("Warning: Selected object is not a Blueprint.");
        OutData.Graph = FText::FromString("Warning: Selected object is not a Blueprint.");
        return;
    }

    FString TmpStructure = FString::Printf(TEXT("# Blueprint Structure: %s\n\n"), *Blueprint->GetName());
    FString TmpGraph = FString::Printf(TEXT("# Graphs Export for Component: %s\n\n"), *Blueprint->GetName());

    AppendBlueprintInfo(Blueprint, TmpStructure);
    AppendVariables(Blueprint, TmpStructure);
    AppendReplicationInfo(Blueprint->GeneratedClass, TmpStructure);
    AppendGraphs(Blueprint, TmpGraph);

    OutData.Structure = FText::FromString(TmpStructure);
    OutData.Graph = FText::FromString(TmpGraph);
}

//---------------------------------------------
// Логирование
//---------------------------------------------
void BPR_Extractor_ActorComponent::LogMessage(const FString& Msg) { UE_LOG(LogTemp, Log, TEXT("[BPR_Extractor_ActorComponent] %s"), *Msg); }
void BPR_Extractor_ActorComponent::LogWarning(const FString& Msg) { UE_LOG(LogTemp, Warning, TEXT("[BPR_Extractor_ActorComponent] %s"), *Msg); }
void BPR_Extractor_ActorComponent::LogError(const FString& Msg) { UE_LOG(LogTemp, Error, TEXT("[BPR_Extractor_ActorComponent] %s"), *Msg); }

//---------------------------------------------
// Общая информация о Blueprint
//---------------------------------------------
void BPR_Extractor_ActorComponent::AppendBlueprintInfo(UBlueprint* Blueprint, FString& OutText)
{
    if (!Blueprint || !Blueprint->ParentClass) return;

    FString ParentClassName = Blueprint->ParentClass->GetName();
    OutText += FString::Printf(TEXT("Parent Class: %s (Native)\n"), *ParentClassName);
    OutText += TEXT("Replication: ");

    if (UClass* GenClass = Blueprint->GeneratedClass)
    {
        if (UActorComponent* CDO = Cast<UActorComponent>(GenClass->GetDefaultObject()))
        {
            OutText += CDO->GetIsReplicated() ? TEXT("Replicating\n\n") : TEXT("Not replicating\n\n");
        }
        else
        {
            OutText += TEXT("N/A (not a component)\n\n");
        }
    }
}

//---------------------------------------------
// Переменные
//---------------------------------------------
void BPR_Extractor_ActorComponent::AppendVariables(UBlueprint* Blueprint, FString& OutText)
{
    if (!Blueprint || !Blueprint->GeneratedClass) return;

    OutText += TEXT("## Custom Variables\n");
    OutText += TEXT("| Name | Type | Default Value | Flags | Description |\n");
    OutText += TEXT("|------|------|---------------|-------|-------------|\n");

    UClass* Class = Blueprint->GeneratedClass;
    UObject* CDO = Class->GetDefaultObject();

    for (TFieldIterator<FProperty> PropIt(Class, EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;
        if (!Property || !IsUserVariable(Property)) continue;

        FString PropName = Property->GetName();
        FString PropType = GetPropertyTypeDetailed(Property);
        FString DefaultVal = GetPropertyDefaultValue(Property, CDO);
        FString Description;
        if (Property->HasAllPropertyFlags(CPF_Edit))
        {
            // Try to get tooltip if available (best-effort)
            // Some properties may not have tooltips — keep description empty in that case.
            Description = Property->GetToolTipText().ToString();
        }
        else
        {
            Description = Property->GetToolTipText().ToString();
        }

        FString Flags;
        if (Property->HasAnyPropertyFlags(CPF_Edit)) Flags += TEXT("Edit ");
        if (Property->HasAnyPropertyFlags(CPF_BlueprintVisible)) Flags += TEXT("BlueprintVisible ");
        if (Property->HasAnyPropertyFlags(CPF_BlueprintReadOnly)) Flags += TEXT("BlueprintReadOnly ");
        Flags = Flags.TrimEnd();

        OutText += FString::Printf(TEXT("| %s | %s | %s | %s | %s |\n"),
            *PropName, *PropType, *DefaultVal, *Flags, *Description);

        if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
        {
            OutText += TEXT("\n### Struct: ") + PropName + TEXT("\n");
            AppendStructFields(StructProp, OutText);
        }
    }

    OutText += TEXT("\n");
}

FString BPR_Extractor_ActorComponent::GetPropertyTypeDetailed(FProperty* Property)
{
    if (!Property) return TEXT("Unknown");

    FString Type = Property->GetCPPType();

    if (FMapProperty* MapProp = CastField<FMapProperty>(Property))
    {
        FString KeyType = MapProp->KeyProp ? MapProp->KeyProp->GetCPPType() : TEXT("Unknown");
        FString ValueType = MapProp->ValueProp ? MapProp->ValueProp->GetCPPType() : TEXT("Unknown");
        Type = FString::Printf(TEXT("TMap<%s, %s>"), *KeyType, *ValueType);
    }
    else if (FSetProperty* SetProp = CastField<FSetProperty>(Property))
    {
        FString ElemType = SetProp->ElementProp ? SetProp->ElementProp->GetCPPType() : TEXT("Unknown");
        Type = FString::Printf(TEXT("TSet<%s>"), *ElemType);
    }
    else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
    {
        FString ElemType = ArrayProp->Inner ? ArrayProp->Inner->GetCPPType() : TEXT("Unknown");
        Type = FString::Printf(TEXT("TArray<%s>"), *ElemType);
    }
    else if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
    {
        if (EnumProp->GetEnum())
            Type = EnumProp->GetEnum()->GetName();
    }

    return Type;
}

void BPR_Extractor_ActorComponent::AppendStructFields(FStructProperty* StructProp, FString& OutText, int32 Indent)
{
    if (!StructProp) return;

    UScriptStruct* Struct = StructProp->Struct;
    if (!Struct) return;

    OutText += TEXT("| Field | Type | Description |\n");
    OutText += TEXT("|-------|------|-------------|\n");

    for (TFieldIterator<FProperty> FieldIt(Struct); FieldIt; ++FieldIt)
    {
        FProperty* Field = *FieldIt;
        FString FieldName = Field->GetName();
        FString FieldType = GetPropertyTypeDetailed(Field);
        FString Desc = Field->GetToolTipText().ToString();

        OutText += FString::Printf(TEXT("| %s | %s | %s |\n"), *FieldName, *FieldType, *Desc);
    }

    OutText += TEXT("\n");
}

FString BPR_Extractor_ActorComponent::GetPropertyDefaultValue(FProperty* Property, UObject* Object)
{
    if (!Property || !Object) return TEXT("None");

    FString ValueStr;
    // Safe export: ExportText_Direct returns the textual default
    Property->ExportText_Direct(ValueStr, Property->ContainerPtrToValuePtr<void>(Object), nullptr, Object, PPF_PropertyWindow);
    return ValueStr.IsEmpty() ? TEXT("None") : ValueStr;
}

bool BPR_Extractor_ActorComponent::IsUserVariable(FProperty* Property)
{
    if (!Property) return false;
    if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated)) return false;
    FString Name = Property->GetName();
    if (Name.StartsWith(TEXT("UberGraphFrame")) || Name.StartsWith(TEXT("PrimaryComponentTick")) || Name.StartsWith(TEXT("bReplicates"))) return false;
    return true;
}

//---------------------------------------------
// Репликация
//---------------------------------------------
void BPR_Extractor_ActorComponent::AppendReplicationInfo(UClass* Class, FString& OutText)
{
    if (!Class) return;

    OutText += TEXT("## Replicated Variables\n");

    bool HasReplicated = false;
    for (TFieldIterator<FProperty> PropIt(Class, EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;
        if (Property->HasAnyPropertyFlags(CPF_Net))
        {
            FString PropName = Property->GetName();
            FString RepNotify = Property->GetMetaData(TEXT("ReplicatedUsing"));
            FString Condition = Property->GetMetaData(TEXT("RepCondition"));

            OutText += FString::Printf(TEXT("- %s (Replicated"), *PropName);
            if (!RepNotify.IsEmpty()) OutText += FString::Printf(TEXT(", Using: %s"), *RepNotify);
            if (!Condition.IsEmpty()) OutText += FString::Printf(TEXT(", Condition: %s"), *Condition);
            OutText += TEXT(")\n");

            HasReplicated = true;
        }
    }

    if (!HasReplicated) OutText += TEXT("- None\n\n");
}

//---------------------------------------------
// Графы
//---------------------------------------------
void BPR_Extractor_ActorComponent::AppendGraphs(UBlueprint* Blueprint, FString& OutText)
{
    if (!Blueprint) return;

    OutText += TEXT("\n## Graphs\n");

    // Event Graphs и UberGraph
    for (UEdGraph* Graph : Blueprint->UbergraphPages)
    {
        if (!Graph) continue;
        OutText += FString::Printf(TEXT("### Event Graph: %s\n"), *Graph->GetName());
        AppendGraphSequence(Graph, OutText);
    }

    // Function Graphs
    for (UEdGraph* Graph : Blueprint->FunctionGraphs)
    {
        if (!Graph) continue;
        FString Signature = GetFunctionSignature(Graph);
        OutText += FString::Printf(TEXT("### Function: %s\n- Signature: %s\n"), *Graph->GetName(), *Signature);
        AppendGraphSequence(Graph, OutText);
    }

    // Macro Graphs
    for (UEdGraph* Graph : Blueprint->MacroGraphs)
    {
        if (!Graph) continue;
        FString Signature = GetMacroSignature(Graph);
        OutText += FString::Printf(TEXT("### Macro: %s\n- Signature: %s\n"), *Graph->GetName(), *Signature);
        AppendGraphSequence(Graph, OutText);
    }
}

static UK2Node_FunctionEntry* FindFunctionEntryNodeInGraph(UEdGraph* Graph)
{
    if (!Graph) return nullptr;

    // Всегда ищем вручную — это сейчас единственный надёжный способ
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (auto* Entry = Cast<UK2Node_FunctionEntry>(Node))
        {
            return Entry;
        }
    }

    return nullptr;
}


static UK2Node_FunctionResult* FindFunctionResultNodeInGraph(UEdGraph* Graph)
{
    if (!Graph) return nullptr;

    // Manual search for result node (no stable helper in some engine versions)
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (UK2Node_FunctionResult* Casted = Cast<UK2Node_FunctionResult>(Node))
        {
            return Casted;
        }
    }
    return nullptr;
}

FString BPR_Extractor_ActorComponent::GetFunctionSignature(UEdGraph* Graph)
{
    if (!Graph) return TEXT("None");

    UK2Node_FunctionEntry* Entry = FindFunctionEntryNodeInGraph(Graph);
    UK2Node_FunctionResult* Result = FindFunctionResultNodeInGraph(Graph);

    TArray<FString> Inputs, Outputs;
    if (Entry)
    {
        for (UEdGraphPin* Pin : Entry->Pins)
        {
            if (!Pin) continue;
            if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
            {
                Inputs.Add(FString::Printf(TEXT("%s: %s"), *Pin->PinName.ToString(), *Pin->PinType.PinCategory.ToString()));
            }
        }
    }

    if (Result)
    {
        for (UEdGraphPin* Pin : Result->Pins)
        {
            if (!Pin) continue;
            if (Pin->Direction == EGPD_Input && Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
            {
                Outputs.Add(FString::Printf(TEXT("%s: %s"), *Pin->PinName.ToString(), *Pin->PinType.PinCategory.ToString()));
            }
        }
    }

    return FString::Printf(TEXT("Inputs: (%s) Outputs: (%s)"), *FString::Join(Inputs, TEXT(", ")), *FString::Join(Outputs, TEXT(", ")));
}

FString BPR_Extractor_ActorComponent::GetMacroSignature(UEdGraph* Graph)
{
    if (!Graph) return TEXT("None");

    TArray<FString> Inputs, Outputs;
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (UK2Node_Tunnel* Tunnel = Cast<UK2Node_Tunnel>(Node))
        {
            for (UEdGraphPin* Pin : Tunnel->Pins)
            {
                if (!Pin) continue;
                FString PinDesc = FString::Printf(TEXT("%s: %s"), *Pin->PinName.ToString(), *Pin->PinType.PinCategory.ToString());
                if (Pin->Direction == EGPD_Input) Inputs.Add(PinDesc);
                else Outputs.Add(PinDesc);
            }
        }
    }
    return FString::Printf(TEXT("Inputs: (%s) Outputs: (%s)"), *FString::Join(Inputs, TEXT(", ")), *FString::Join(Outputs, TEXT(", ")));
}

void BPR_Extractor_ActorComponent::AppendGraphSequence(UEdGraph* Graph, FString& OutText)
{
    if (!Graph || Graph->Nodes.Num() == 0) return;

    TSet<UEdGraphNode*> Visited;
    UEdGraphNode* StartNode = FindFunctionEntryNodeInGraph(Graph);
    if (!StartNode)
    {
        // Fallback: first node (for event graphs there may be EventEntry or others)
        StartNode = Graph->Nodes.Num() > 0 ? Graph->Nodes[0] : nullptr;
    }

    if (StartNode)
    {
        ProcessNodeSequence(StartNode, 0, Visited, OutText);
    }
}

void BPR_Extractor_ActorComponent::ProcessNodeSequence(UEdGraphNode* Node, int32 IndentLevel, TSet<UEdGraphNode*>& Visited, FString& OutText)
{
    if (!Node || Visited.Contains(Node))
    {
        if (Node) OutText += FString::Printf(TEXT("%*s[Loop Detected: %s]\n"), IndentLevel * 2, TEXT(""), *Node->GetName());
        return;
    }

    Visited.Add(Node);

    FString NodeName = GetReadableNodeName(Node);
    FString Comment = Node->NodeComment;
    if (!Comment.IsEmpty()) NodeName += FString::Printf(TEXT(" // %s"), *Comment);

    OutText += FString::Printf(TEXT("%*s- %s\n"), IndentLevel * 2, TEXT(""), *NodeName);

    // Обходим все exec-выходы
    for (UEdGraphPin* Pin : Node->Pins)
    {
        if (!Pin) continue;
        if (Pin->Direction != EGPD_Output || Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec) continue;

        FString Label = Pin->PinFriendlyName.ToString();
        if (Label.IsEmpty()) Label = Pin->PinName.ToString();
        if (Label.IsEmpty()) Label = TEXT("Exec");

        for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
        {
            if (!LinkedPin || !LinkedPin->GetOwningNode()) continue;

            OutText += FString::Printf(TEXT("%*s  [%s] →\n"), IndentLevel * 2, TEXT(""), *Label);
            ProcessNodeSequence(LinkedPin->GetOwningNode(), IndentLevel + 1, Visited, OutText);
        }
    }
}

FString BPR_Extractor_ActorComponent::GetReadableNodeName(UEdGraphNode* Node)
{
    if (!Node) return TEXT("None");

    FString BaseName = Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString();

    if (UK2Node_CallFunction* CallFunc = Cast<UK2Node_CallFunction>(Node))
    {
        FString Params;
        for (UEdGraphPin* Pin : CallFunc->Pins)
        {
            if (!Pin) continue;
            if (Pin->Direction == EGPD_Input && Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
            {
                Params += FString::Printf(TEXT("%s=%s "), *Pin->PinName.ToString(), *GetPinDetails(Pin));
            }
        }
        return FString::Printf(TEXT("CallFunction: %s(%s)"), *CallFunc->GetFunctionName().ToString(), *Params.TrimEnd());
    }

    if (UK2Node_VariableSet* VarSet = Cast<UK2Node_VariableSet>(Node))
    {
        FString VarName = VarSet->VariableReference.GetMemberName().ToString();
        return FString::Printf(TEXT("VariableSet: %s = <value>"), *VarName);
    }

    if (UK2Node_VariableGet* VarGet = Cast<UK2Node_VariableGet>(Node))
    {
        FString VarName = VarGet->VariableReference.GetMemberName().ToString();
        return FString::Printf(TEXT("VariableGet: %s"), *VarName);
    }

    if (UK2Node_IfThenElse* Branch = Cast<UK2Node_IfThenElse>(Node))
    {
        FString Condition = GetPinDetails(Branch->GetConditionPin());
        return FString::Printf(TEXT("Branch (Condition: %s)"), *Condition);
    }

    if (UK2Node_Switch* Switch = Cast<UK2Node_Switch>(Node))
    {
        FString Selection = GetPinDetails(Switch->GetSelectionPin());
        return FString::Printf(TEXT("Switch (Selection: %s)"), *Selection);
    }

    if (Cast<UK2Node_CallDelegate>(Node)) return TEXT("CallDelegate");
    if (Cast<UK2Node_Tunnel>(Node)) return TEXT("Tunnel");
    if (Cast<UK2Node_FunctionEntry>(Node)) return TEXT("FunctionEntry");
    if (Cast<UK2Node_FunctionResult>(Node)) return TEXT("FunctionResult");

    return BaseName.IsEmpty() ? Node->GetName() : BaseName;
}

FString BPR_Extractor_ActorComponent::GetPinDetails(UEdGraphPin* Pin)
{
    if (!Pin) return TEXT("None");

    if (!Pin->DefaultValue.IsEmpty()) return Pin->DefaultValue;
    if (Pin->LinkedTo.Num() > 0 && Pin->LinkedTo[0] && Pin->LinkedTo[0]->GetOwningNode())
    {
        return FString::Printf(TEXT("<linked from %s>"), *Pin->LinkedTo[0]->GetOwningNode()->GetName());
    }
    return TEXT("default");
}
