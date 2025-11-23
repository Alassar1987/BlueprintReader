#pragma once

#include "CoreMinimal.h"

/**
 * Экстрактор для Blueprint Interface
 */
class BLUEPRINTREADER_API BPR_Extractor_InterfaceBP
{
public:
	BPR_Extractor_InterfaceBP();
	~BPR_Extractor_InterfaceBP();

	/** Основной метод извлечения данных */
	void ProcessInterfaceBP(UObject* Object, FText& OutText);

private:

	/** Вспомогательный метод для сбора информации о функциях интерфейса */
	void AppendInterfaceFunctions(UBlueprint* BP, FString& OutText);
};
