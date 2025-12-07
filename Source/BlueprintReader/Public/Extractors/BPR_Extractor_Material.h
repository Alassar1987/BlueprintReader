#pragma once

#include "CoreMinimal.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialExpression.h"
#include "Core/BPR_Core.h"

/**
 * Экстрактор для Material и MaterialInstance.
 * Собирает свойства материала, параметры и граф выражений.
 * Результат возвращается в Core через FText.
 */
class BLUEPRINTREADER_API BPR_Extractor_Material
{
public:
    BPR_Extractor_Material();
    ~BPR_Extractor_Material();

    // -------------------------------
    // Главная точка входа
    // -------------------------------
    /** Обрабатывает выбранный объект (Material или MaterialInstance) */
    void ProcessMaterial(UObject* SelectedObject, FBPR_ExtractedData& OutData);

private:
    // -------------------------------
    // Логирование
    // -------------------------------
    void LogMessage(const FString& Msg);
    void LogWarning(const FString& Msg);
    void LogError(const FString& Msg);

    // -------------------------------
    // Обработка UMaterial
    // -------------------------------
    /** Обработка базового материала */
    void ProcessMaterialAsset(UMaterial* Material, FString& OutStructure, FString& OutGraph);
    
    /** Свойства материала (BlendMode, ShadingModel, TwoSided) */
    void AppendMaterialProperties(UMaterial* Material, FString& OutText);
    
    /** Параметры материала в табличном формате */
    void AppendMaterialParameters(UMaterial* Material, FString& OutText);
    
    /** Expression-ноды материала */
    void AppendMaterialExpressions(UMaterial* Material, FString& OutText);

    // -------------------------------
    // Обработка MaterialInstance
    // -------------------------------
    /** Обработка инстанса материала */
    void ProcessMaterialInstanceAsset(UMaterialInstance* MatInst, FString& OutStructure, FString& OutGraph);
    
    /** Информация об инстансе и его родителе */
    void AppendInstanceInfo(UMaterialInstance* MatInst, FString& OutText);
    
    /** Переопределенные параметры инстанса в табличном формате */
    void AppendParameterOverrides(UMaterialInstance* MatInst, FString& OutText);

    // -------------------------------
    // Вспомогательные методы
    // -------------------------------
    /** Преобразует Expression в читаемое описание */
    FString GetExpressionDescription(UMaterialExpression* Expression);
    
    /** Форматирует имя Shading Model */
    FString GetShadingModelName(EMaterialShadingModel Model);
    
    /** Форматирует BlendMode */
    FString GetBlendModeName(EBlendMode Mode);
};