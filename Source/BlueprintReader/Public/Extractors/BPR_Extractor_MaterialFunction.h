#pragma once

#include "CoreMinimal.h"
#include "Core/BPR_Core.h"

class UMaterialFunctionInterface;
class UMaterialFunction;
class UMaterialFunctionInstance;
class UMaterialExpression;

/**
 * Экстрактор для Material Function и Material Function Instance
 */
class BLUEPRINTREADER_API BPR_Extractor_MaterialFunction
{
public:
	BPR_Extractor_MaterialFunction();
	~BPR_Extractor_MaterialFunction();

	/** Главный метод: превращает Material Function в Markdown */
	void ProcessMaterialFunction(UObject* SelectedObject, FBPR_ExtractedData& OutData);

private:

	// Проверки типов
	bool IsMaterialFunction(UObject* Obj) const;
	bool IsMaterialFunctionInstance(UObject* Obj) const;

	// Основные обработчики
	void ExtractMaterialFunction(UMaterialFunction* Function, FString& OutText);
	void ExtractMaterialFunctionInstance(UMaterialFunctionInstance* Instance, FString& OutText);

	// Подсекции
	void AppendFunctionInputs(UMaterialFunction* Function, FString& OutText);
	void AppendFunctionOutputs(UMaterialFunction* Function, FString& OutText);
	void AppendFunctionExpressions(UMaterialFunction* Function, FString& OutText);

	void AppendInstanceOverrides(UMaterialFunctionInstance* Instance, FString& OutText);

	// Логирование
	void LogMessage(const FString& Msg);
	void LogWarning(const FString& Msg);
	void LogError(const FString& Msg);
};
