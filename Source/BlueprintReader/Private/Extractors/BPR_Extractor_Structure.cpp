#include "Extractors/BPR_Extractor_Structure.h"
#include "UObject/UnrealType.h"
#include "UObject/StructOnScope.h"

BPR_Extractor_Structure::BPR_Extractor_Structure()
{
}

BPR_Extractor_Structure::~BPR_Extractor_Structure()
{
}

void BPR_Extractor_Structure::ProcessStructure(UObject* Object, FText& OutText)
{
#if WITH_EDITOR
	FString Result;

	if (!Object)
	{
		OutText = FText::FromString("Object is null.");
		return;
	}

	// Если это структура внутри Blueprint (например, через UStructProperty), можно пробовать получить UScriptStruct
	if (UScriptStruct* StructObj = Cast<UScriptStruct>(Object))
	{
		AppendStructInfo(StructObj, Result);
	}
	else
	{
		if (UClass* Class = Object->GetClass())
		{
			UStruct* SuperStruct = Class->GetSuperStruct(); // переименовали
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

	OutText = FText::FromString(Result);
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

		FString PropName = Property->GetNameCPP();
		FString PropType = Property->GetClass()->GetName(); // например, FIntProperty, FFloatProperty

		OutText += FString::Printf(TEXT("- **%s** : %s\n"), *PropName, *PropType);
	}

	OutText += TEXT("\n");
}
