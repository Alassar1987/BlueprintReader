#pragma once

#include "CoreMinimal.h"
#include "Core/BPR_Core.h" // FBPR_ExtractedData

/**
 * Экстрактор для структур (UScriptStruct)
 */
class BLUEPRINTREADER_API BPR_Extractor_Structure
{
public:
	BPR_Extractor_Structure();
	~BPR_Extractor_Structure();

	/** Основной метод извлечения данных из структуры */
	void ProcessStructure(UObject* Object, FBPR_ExtractedData& OutData);

private:
	/** Помогает формировать текст Markdown для конкретного UScriptStruct */
	void AppendStructInfo(UScriptStruct* Struct, FString& OutText);
};
