#pragma once

#include "CoreMinimal.h"
#include "Core/BPR_Core.h"

/**
 * Extractor for structures (UScriptStruct)
 */
class BLUEPRINTREADER_API BPR_Extractor_Structure
{
public:
	BPR_Extractor_Structure();
	~BPR_Extractor_Structure();

	/** Basic method of extracting data from a structure */
	void ProcessStructure(UObject* Object, FBPR_ExtractedData& OutData);

private:
	/** Helps generate Markdown text for a specific UScriptStruct */
	void AppendStructInfo(UScriptStruct* Struct, FString& OutText);
};
