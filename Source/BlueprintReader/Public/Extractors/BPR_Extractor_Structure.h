#pragma once

#include "CoreMinimal.h"

/**
 * Экстрактор для структур (UScriptStruct)
 */
class BLUEPRINTREADER_API BPR_Extractor_Structure
{
public:
	BPR_Extractor_Structure();
	~BPR_Extractor_Structure();

	/** Основной метод извлечения данных из структуры */
	void ProcessStructure(UObject* Object, FText& OutText);

private:
	/** Помогает формировать текст Markdown для конкретного UScriptStruct */
	void AppendStructInfo(UScriptStruct* Struct, FString& OutText);
};
