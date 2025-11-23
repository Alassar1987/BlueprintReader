#pragma once

#include "CoreMinimal.h"

class BLUEPRINTREADER_API BPR_Extractor_Enum
{
public:
	BPR_Extractor_Enum();
	~BPR_Extractor_Enum();

	/** Основной метод: превращает Enum в Markdown */
	void ProcessEnum(UObject* Object, FText& OutText);

private:

	/** Вспомогательный метод для сборки списка элементов */
	void AppendEnumEntries(UEnum* Enum, FString& OutText);
};
