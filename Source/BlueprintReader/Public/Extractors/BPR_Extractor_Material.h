#pragma once

#include "CoreMinimal.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "Core/BPR_Core.h"

/**
 * Экстрактор для Material и MaterialInstance
 * Собирает свойства материала и список Expression-нод в Markdown.
 */
class BLUEPRINTREADER_API BPR_Extractor_Material
{
public:
	BPR_Extractor_Material();
	~BPR_Extractor_Material();

	/** Главная точка входа */
	void ProcessMaterial(UObject* SelectedObject, FBPR_ExtractedData& OutData);

private:
	void LogMessage(const FString& Msg);
	void LogWarning(const FString& Msg);
	void LogError(const FString& Msg);

	/** Обрабатываем конкретный материал */
	void AppendMaterialProperties(UMaterial* Material, FString& OutText);
	void AppendMaterialExpressions(UMaterial* Material, FString& OutText);

	/** Преобразуем Expression в читаемое описание */
	FString GetExpressionDescription(UMaterialExpression* Expression);
};
