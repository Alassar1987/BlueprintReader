#include "Extractors/BPR_Extractor_InterfaceBP.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "EdGraph/EdGraphPin.h"

BPR_Extractor_InterfaceBP::BPR_Extractor_InterfaceBP()
{
}

BPR_Extractor_InterfaceBP::~BPR_Extractor_InterfaceBP()
{
}

void BPR_Extractor_InterfaceBP::ProcessInterfaceBP(
    UObject* Object,
    FBPR_ExtractedData& OutData
)

{
#if WITH_EDITOR
	FString Result;

	UBlueprint* BP = Cast<UBlueprint>(Object);
	if (!BP || BP->BlueprintType != BPTYPE_Interface)
	{
		OutData.Structure = FText::FromString("Object is not a Blueprint Interface.");
		return;
	}

	Result += TEXT("# Blueprint Interface\n\n");

	Result += FString::Printf(TEXT("- **Name:** %s\n"), *BP->GetName());
	Result += FString::Printf(TEXT("- **Generated Class:** %s\n"), 
		* (BP->GeneratedClass ? BP->GeneratedClass->GetName() : FString("None")));

	Result += TEXT("\n");

	// Adding a list of interface functions
	AppendInterfaceFunctions(BP, Result);

	OutData.Structure = FText::FromString(Result);
#endif
}

//
void BPR_Extractor_InterfaceBP::AppendInterfaceFunctions(UBlueprint* BP, FString& OutText)
{
    if (!BP)
        return;

    OutText += TEXT("## Interface Functions\n\n");

    for (UEdGraph* Graph : BP->FunctionGraphs)
    {
        if (!Graph)
            continue;

        FString FunctionName = Graph->GetName();
        OutText += FString::Printf(TEXT("### %s\n"), *FunctionName);

        UK2Node_FunctionEntry* EntryNode = nullptr;
        UK2Node_FunctionResult* ResultNode = nullptr;

        // Ищем Entry и Result ноды
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (!EntryNode)
                EntryNode = Cast<UK2Node_FunctionEntry>(Node);
            if (!ResultNode)
                ResultNode = Cast<UK2Node_FunctionResult>(Node);
            
            if (EntryNode && ResultNode)
                break;
        }

        // Processing input parameters (from FunctionEntry)
        if (EntryNode)
        {
            for (UEdGraphPin* Pin : EntryNode->Pins)
            {
                if (!Pin || Pin->bHidden)
                    continue;

                if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
                    continue;

                if (Pin->Direction != EGPD_Output)
                    continue;

                FString ParamName = Pin->PinName.ToString();
                FString ParamType = Pin->PinType.PinCategory.ToString();
                
                if (Pin->PinType.PinSubCategoryObject.IsValid())
                {
                    ParamType = Pin->PinType.PinSubCategoryObject->GetName();
                }

                OutText += FString::Printf(TEXT("- **Input** %s : %s\n"), *ParamName, *ParamType);
            }
        }

        // Processing output parameters (from FunctionResult)
        if (ResultNode)
        {
            for (UEdGraphPin* Pin : ResultNode->Pins)
            {
                if (!Pin || Pin->bHidden)
                    continue;

                if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
                    continue;

                // On FunctionResult the output parameters have Direction == Input 
                // (data is “included” in Result from the graph)
                if (Pin->Direction != EGPD_Input)
                    continue;

                FString ParamName = Pin->PinName.ToString();
                FString ParamType = Pin->PinType.PinCategory.ToString();
                
                if (Pin->PinType.PinSubCategoryObject.IsValid())
                {
                    ParamType = Pin->PinType.PinSubCategoryObject->GetName();
                }

                OutText += FString::Printf(TEXT("- **Output** %s : %s\n"), *ParamName, *ParamType);
            }
        }

        OutText += TEXT("\n");
    }

    if (BP->FunctionGraphs.Num() == 0)
    {
        OutText += TEXT("_No functions defined in this interface._\n\n");
    }
}