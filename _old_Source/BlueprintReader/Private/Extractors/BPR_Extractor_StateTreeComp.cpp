//Часть кода закомменчена. Сейчас невозможо реализовать, поскольку данные protected

#include "Extractors/BPR_Extractor_StateTreeComp.h"

#include "StateTreeSchema.h"
#include "Components/StateTreeComponentSchema.h"
#include "Components/StateTreeAIComponentSchema.h"
#include "StateTreeExecutionTypes.h"
#include "StateTree.h"

BPR_Extractor_StateTreeComp::BPR_Extractor_StateTreeComp() {}
BPR_Extractor_StateTreeComp::~BPR_Extractor_StateTreeComp() {}

void BPR_Extractor_StateTreeComp::ProcessStateTreeSchema(UObject* Object, FText& OutText)
{
#if WITH_EDITOR

	FString Result;

	UStateTreeComponentSchema* Schema = Cast<UStateTreeComponentSchema>(Object);
	if (!Schema)
	{
		OutText = FText::FromString("Object is not a StateTree Component Schema.");
		return;
	}

	Result += TEXT("# StateTree Component Schema\n\n");

	AppendSchemaHeader(Schema, Result);
	AppendContextData(Schema, Result);
	AppendOwnerInfo(Schema, Result);

	// Если это AI Schema — добавляем блок AI данных
	if (UStateTreeAIComponentSchema* AISchema = Cast<UStateTreeAIComponentSchema>(Schema))
	{
		AppendAIInfo(AISchema, Result);
	}

	OutText = FText::FromString(Result);

#endif
}

//=====================================================================
// HEADER
//=====================================================================
void BPR_Extractor_StateTreeComp::AppendSchemaHeader(UStateTreeComponentSchema* Schema, FString& OutText)
{
	OutText += FString::Printf(TEXT("- **Name:** %s\n"), *Schema->GetName());
	OutText += FString::Printf(TEXT("- **Class:** %s\n\n"), *Schema->GetClass()->GetName());
}

//=====================================================================
// CONTEXT OBJECTS
//=====================================================================
void BPR_Extractor_StateTreeComp::AppendContextData(UStateTreeComponentSchema* Schema, FString& OutText)
{
	// TConstArrayView<FStateTreeExternalDataDesc> Data = Schema->GetContextDataDescs();
	//
	// OutText += TEXT("## Context Objects\n\n");
	//
	// if (Data.Num() == 0)
	// {
	// 	OutText += TEXT("_No context objects defined._\n\n");
	// 	return;
	// }
	//
	// for (const FStateTreeExternalDataDesc& Desc : Data)
	// {
	// 	FString Name = Desc.Name.ToString();
	// 	FString StructName = Desc.Struct ? Desc.Struct->GetName() : TEXT("None");
	// 	FString Requirement = (Desc.Requirement == EStateTreeExternalDataRequirement::Required) ? TEXT("Required") : TEXT("Optional");
	//
	// 	OutText += FString::Printf(TEXT("- %s : %s (%s)\n"), *Name, *StructName, *Requirement);
	// }
	//
	// OutText += TEXT("\n");
}

//=====================================================================
// OWNER INFO
//=====================================================================
void BPR_Extractor_StateTreeComp::AppendOwnerInfo(UStateTreeComponentSchema* Schema, FString& OutText)
{
	// OutText += TEXT("## Owner (Extracted from Context Data)\n\n");
	//
	// TConstArrayView<FStateTreeExternalDataDesc> Data = Schema->GetContextDataDescs();
	//
	// const UClass* ActorClass = nullptr;
	// const UClass* ComponentClass = nullptr;
	//
	// for (const FStateTreeExternalDataDesc& Desc : Data)
	// {
	// 	if (!Desc.Struct) continue;
	//
	// 	if (Desc.Struct->IsChildOf(AActor::StaticClass()))
	// 		ActorClass = Desc.Struct->GetClass();
	//
	// 	if (Desc.Struct->IsChildOf(UActorComponent::StaticClass()))
	// 		ComponentClass = Desc.Struct->GetClass();
	// }
	//
	// OutText += FString::Printf(TEXT("- Actor Class: %s\n"), ActorClass ? *ActorClass->GetName() : TEXT("None"));
	// OutText += FString::Printf(TEXT("- Component Class: %s\n\n"), ComponentClass ? *ComponentClass->GetName() : TEXT("None"));
}

//=====================================================================
// AI SCHEMA EXTRA INFO
//=====================================================================
void BPR_Extractor_StateTreeComp::AppendAIInfo(UStateTreeAIComponentSchema* AISchema, FString& OutText)
{
	// OutText += TEXT("## AI Extensions\n\n");
	//
	// TConstArrayView<FStateTreeExternalDataDesc> Data = AISchema->GetContextDataDescs();
	//
	// const UClass* ControllerClass = nullptr;
	// const UClass* PawnClass = nullptr;
	//
	// for (const FStateTreeExternalDataDesc& Desc : Data)
	// {
	// 	if (!Desc.Struct) continue;
	//
	// 	if (Desc.Struct->IsChildOf(APawn::StaticClass()))
	// 		PawnClass = Desc.Struct->GetClass();
	//
	// 	if (Desc.Struct->IsChildOf(AController::StaticClass()))
	// 		ControllerClass = Desc.Struct->GetClass();
	// }
	//
	// OutText += FString::Printf(TEXT("- Controller Class: %s\n"), ControllerClass ? *ControllerClass->GetName() : TEXT("None"));
	// OutText += FString::Printf(TEXT("- Pawn Class: %s\n\n"), PawnClass ? *PawnClass->GetName() : TEXT("None"));
}
