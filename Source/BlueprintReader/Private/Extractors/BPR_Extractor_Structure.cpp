#include "Extractors/BPR_Extractor_Structure.h"
#include "UObject/UnrealType.h"
#include "UObject/StructOnScope.h"

BPR_Extractor_Structure::BPR_Extractor_Structure()
{
}

BPR_Extractor_Structure::~BPR_Extractor_Structure()
{
}

void BPR_Extractor_Structure::ProcessStructure(UObject* Object, FBPR_ExtractedData& OutData)
{
#if WITH_EDITOR
	FString Result;

	if (!Object)
	{
		OutData.Structure = FText::FromString(TEXT("Object is null."));
		OutData.Graph     = FText::FromString(TEXT("N/A"));
		return;
	}

	// Если объект уже UScriptStruct — используем его
	if (UScriptStruct* StructObj = Cast<UScriptStruct>(Object))
	{
		AppendStructInfo(StructObj, Result);
	}
	else
	{
		// В других случаях попробуем найти структуру в metadata класса (если это нужно)
		// старый код пытался брать SuperStruct от класса — оставим ту же логику
		if (UClass* Class = Object->GetClass())
		{
			UStruct* SuperStruct = Class->GetSuperStruct();
			if (SuperStruct)
			{
				AppendStructInfo(Cast<UScriptStruct>(SuperStruct), Result);
			}
		}
	}

	if (Result.IsEmpty())
	{
		Result = TEXT("No structure info available.");
	}

	OutData.Structure = FText::FromString(Result);
	OutData.Graph     = FText::FromString(TEXT("N/A")); // У структур графов нет
#else
	OutData.Structure = FText::FromString(TEXT("Structure extraction available only in editor builds."));
	OutData.Graph     = FText::FromString(TEXT("N/A"));
#endif
}

void BPR_Extractor_Structure::AppendStructInfo(UScriptStruct* Struct, FString& OutText)
{
	if (!Struct)
		return;

	OutText += FString::Printf(TEXT("# Structure: %s\n\n"), *Struct->GetName());

	OutText += TEXT("## Fields\n\n");

	for (TFieldIterator<FProperty> It(Struct); It; ++It)
	{
		FProperty* Property = *It;
		if (!Property)
			continue;

		// Имя и тип остаются как в старой реализации
		FString PropName = Property->GetNameCPP();
		FString PropType = Property->GetClass()->GetName(); // например, FIntProperty, FFloatProperty

		OutText += FString::Printf(TEXT("- **%s** : %s\n"), *PropName, *PropType);
	}

	OutText += TEXT("\n");
}
