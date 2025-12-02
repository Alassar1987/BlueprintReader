#include "Extractors/BPR_Extractor_InterfaceBP.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_FunctionEntry.h"
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

	// Добавляем список функций интерфейса
	AppendInterfaceFunctions(BP, Result);

	OutData.Structure = FText::FromString(Result);
#endif
}

// Есть баг - перепутаны пины на ввод и вывод. Надо разбираться почему
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
        OutText += FString::Printf(TEXT("- %s\n"), *FunctionName);

        // Ищем ноду FunctionEntry
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UK2Node_FunctionEntry* EntryNode = Cast<UK2Node_FunctionEntry>(Node))
            {
                // Один проход по пинам
                for (UEdGraphPin* Pin : EntryNode->Pins)
                {
                    if (!Pin || Pin->bHidden)
                        continue;

                    FString ParamName = Pin->PinName.ToString();
                    FString ParamType = Pin->PinType.PinCategory.ToString();

                    if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
                    {
                        // Exec-пины всегда как Output then
                        if (Pin->Direction == EGPD_Output)
                        {
                            OutText += TEXT("    - Output then : exec\n");
                        }
                        // Игнорируем входящий Exec
                    }
                    else
                    {
                        // Проверяем направление и ссылочность
                        if (Pin->Direction == EGPD_Output && Pin->PinType.bIsReference)
                        {
                            // Output параметр
                            OutText += FString::Printf(TEXT("    - Output %s : %s\n"), *ParamName, *ParamType);
                        }
                        else
                        {
                            // Input параметр
                            OutText += FString::Printf(TEXT("    - Input %s : %s\n"), *ParamName, *ParamType);
                        }
                    }
                }
            }
        }

        OutText += TEXT("\n");
    }

    if (BP->FunctionGraphs.Num() == 0)
    {
        OutText += TEXT("_No functions defined in this interface._\n\n");
    }
}



