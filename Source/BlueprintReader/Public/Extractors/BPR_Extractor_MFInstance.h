#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialFunctionInstance.h"

class BLUEPRINTREADER_API BPR_Extractor_MFInstance
{
public:
	BPR_Extractor_MFInstance() = default;
	~BPR_Extractor_MFInstance() = default;

public:
	/** Основной метод */
	void ProcessMFInstance(UObject* Object, FText& OutText);

private:
	void AppendBasicInfo(UMaterialFunctionInstance* Instance, FString& OutText);
	void AppendOverrides(UMaterialFunctionInstance* Instance, FString& OutText);
};
