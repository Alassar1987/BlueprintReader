#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstance.h"

/**
 * Экстрактор для MaterialInstance
 * Собирает параметры MaterialInstance в Markdown.
 */
class BLUEPRINTREADER_API BPR_Extractor_MaterialInstance
{
public:
	BPR_Extractor_MaterialInstance();
	~BPR_Extractor_MaterialInstance();

	/** Главная точка входа */
	void ProcessMaterialInstance(UObject* SelectedObject, FText& OutText);

private:
	void LogMessage(const FString& Msg);
	void LogWarning(const FString& Msg);
	void LogError(const FString& Msg);

	/** Обрабатываем параметры */
	void AppendMaterialInstanceParameters(UMaterialInstance* MI, FString& OutText);
};
