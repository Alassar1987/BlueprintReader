#pragma once

#include "CoreMinimal.h"
#include "Core/BPR_Core.h" // для FBPR_ExtractedData

class BLUEPRINTREADER_API BPR_Extractor_Enum
{
public:
	BPR_Extractor_Enum();
	~BPR_Extractor_Enum();

	/** Основной метод: превращает Enum в FBPR_ExtractedData */
	void ProcessEnum(UObject* Object, FBPR_ExtractedData& OutData);

private:
	/** Сборка списка элементов Enum */
	void AppendEnumEntries(UEnum* Enum, FString& OutText);
};
