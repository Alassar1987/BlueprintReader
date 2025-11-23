#pragma once

#include "CoreMinimal.h"

class UStateTreeComponentSchema;
class UStateTreeAIComponentSchema;

class BLUEPRINTREADER_API BPR_Extractor_StateTreeComp
{
public:
	BPR_Extractor_StateTreeComp();
	~BPR_Extractor_StateTreeComp();

	void ProcessStateTreeSchema(UObject* Object, FText& OutText);

private:
	void AppendSchemaHeader(UStateTreeComponentSchema* Schema, FString& OutText);
	void AppendContextData(UStateTreeComponentSchema* Schema, FString& OutText);
	void AppendOwnerInfo(UStateTreeComponentSchema* Schema, FString& OutText);
	void AppendAIInfo(UStateTreeAIComponentSchema* AISchema, FString& OutText);
};
