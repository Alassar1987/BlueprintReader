#include "Extractors/BPR_Extractor_Material.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Engine/Texture.h"
#include "Engine/EngineTypes.h"

//==============================================================================
// Constructor / Destructor
//==============================================================================
BPR_Extractor_Material::BPR_Extractor_Material() {}
BPR_Extractor_Material::~BPR_Extractor_Material() {}

//==============================================================================
// Main entry point
//==============================================================================
void BPR_Extractor_Material::ProcessMaterial(UObject* SelectedObject, FBPR_ExtractedData& OutData)
{
    if (!SelectedObject)
    {
        LogError(TEXT("SelectedObject is null!"));
        OutData.Structure = FText::FromString("Error: SelectedObject is null.");
        OutData.Graph = FText::FromString("Error: SelectedObject is null.");
        return;
    }

    FString TmpStructure;
    FString TmpGraph;

    // Обработка базового материала
    if (UMaterial* Material = Cast<UMaterial>(SelectedObject))
    {
        TmpStructure = FString::Printf(TEXT("# Material Structure: %s\n\n"), *Material->GetName());
        TmpGraph = FString::Printf(TEXT("# Material Graph: %s\n\n"), *Material->GetName());
        
        ProcessMaterialAsset(Material, TmpStructure, TmpGraph);
    }
    // Обработка инстанса материала
    else if (UMaterialInstance* MatInst = Cast<UMaterialInstance>(SelectedObject))
    {
        TmpStructure = FString::Printf(TEXT("# Material Instance Structure: %s\n\n"), *MatInst->GetName());
        TmpGraph = FString::Printf(TEXT("# Material Instance Graph: %s\n\n"), *MatInst->GetName());
        
        ProcessMaterialInstanceAsset(MatInst, TmpStructure, TmpGraph);
    }
    else
    {
        LogWarning(TEXT("Selected object is not a Material or MaterialInstance."));
        TmpStructure = TEXT("Warning: Selected object is not a Material or MaterialInstance.");
        TmpGraph = TEXT("Warning: Selected object is not a Material or MaterialInstance.");
    }

    OutData.Structure = FText::FromString(TmpStructure);
    OutData.Graph = FText::FromString(TmpGraph);
    
    LogMessage(FString::Printf(TEXT("Material extraction finished for %s"), *SelectedObject->GetName()));
}

//==============================================================================
// Logging
//==============================================================================
void BPR_Extractor_Material::LogMessage(const FString& Msg) 
{ 
    UE_LOG(LogTemp, Log, TEXT("[BPR_Extractor_Material] %s"), *Msg); 
}

void BPR_Extractor_Material::LogWarning(const FString& Msg) 
{ 
    UE_LOG(LogTemp, Warning, TEXT("[BPR_Extractor_Material] %s"), *Msg); 
}

void BPR_Extractor_Material::LogError(const FString& Msg) 
{ 
    UE_LOG(LogTemp, Error, TEXT("[BPR_Extractor_Material] %s"), *Msg); 
}

//==============================================================================
// UMaterial Processing
//==============================================================================
void BPR_Extractor_Material::ProcessMaterialAsset(UMaterial* Material, FString& OutStructure, FString& OutGraph)
{
    if (!Material) return;

    AppendMaterialProperties(Material, OutStructure);
    AppendMaterialParameters(Material, OutStructure);
    AppendMaterialExpressions(Material, OutGraph);
}

void BPR_Extractor_Material::AppendMaterialProperties(UMaterial* Material, FString& OutText)
{
    if (!Material) return;

    OutText += TEXT("## Material Properties\n");
    OutText += TEXT("| Property | Value |\n");
    OutText += TEXT("|----------|-------|\n");

    // BlendMode
    FString BlendMode = GetBlendModeName(Material->GetBlendMode());
    OutText += FString::Printf(TEXT("| BlendMode | %s |\n"), *BlendMode);

    // ShadingModels
    FMaterialShadingModelField ShadingModels = Material->GetShadingModels();
    FString ShadingModelsStr;
    for (uint32 i = 0; i < uint32(MSM_MAX); ++i)
    {
        if (ShadingModels.HasShadingModel((EMaterialShadingModel)i))
        {
            if (!ShadingModelsStr.IsEmpty()) ShadingModelsStr += TEXT(", ");
            ShadingModelsStr += GetShadingModelName((EMaterialShadingModel)i);
        }
    }
    OutText += FString::Printf(TEXT("| ShadingModel | %s |\n"), *ShadingModelsStr);

    // TwoSided
    OutText += FString::Printf(TEXT("| TwoSided | %s |\n"), 
        Material->IsTwoSided() ? TEXT("true") : TEXT("false"));

    // IsMasked
    OutText += FString::Printf(TEXT("| IsMasked | %s |\n"), 
        Material->IsMasked() ? TEXT("true") : TEXT("false"));

    OutText += TEXT("\n");
}

void BPR_Extractor_Material::AppendMaterialParameters(UMaterial* Material, FString& OutText)
{
    if (!Material) return;

    // Собираем все параметры из Expression
    TArray<UMaterialExpressionScalarParameter*> ScalarParams;
    TArray<UMaterialExpressionVectorParameter*> VectorParams;
    TArray<UMaterialExpressionTextureObjectParameter*> TextureParams;

    TConstArrayView<TObjectPtr<UMaterialExpression>> Expressions = Material->GetExpressions();

    for (UMaterialExpression* Expr : Expressions)
    {
        if (!Expr) continue;

        if (auto* Scalar = Cast<UMaterialExpressionScalarParameter>(Expr))
            ScalarParams.Add(Scalar);
        else if (auto* Vector = Cast<UMaterialExpressionVectorParameter>(Expr))
            VectorParams.Add(Vector);
        else if (auto* Texture = Cast<UMaterialExpressionTextureObjectParameter>(Expr))
            TextureParams.Add(Texture);
    }


    // Scalar Parameters
    if (ScalarParams.Num() > 0)
    {
        OutText += TEXT("## Scalar Parameters\n");
        OutText += TEXT("| Name | Default Value | Group | Description |\n");
        OutText += TEXT("|------|---------------|-------|-------------|\n");

        for (UMaterialExpressionScalarParameter* Param : ScalarParams)
        {
            FString Name = Param->ParameterName.ToString();
            FString Value = FString::Printf(TEXT("%.6g"), Param->DefaultValue);
            FString Group = Param->Group.ToString();
            FString Desc = Param->Desc;

            OutText += FString::Printf(TEXT("| %s | %s | %s | %s |\n"),
                *Name, *Value, *Group, *Desc);
        }

        OutText += TEXT("\n");
    }

    // Vector Parameters
    if (VectorParams.Num() > 0)
    {
        OutText += TEXT("## Vector Parameters\n");
        OutText += TEXT("| Name | R | G | B | A | Group | Description |\n");
        OutText += TEXT("|------|---|---|---|---|-------|-------------|\n");

        for (UMaterialExpressionVectorParameter* Param : VectorParams)
        {
            FString Name = Param->ParameterName.ToString();
            const FLinearColor& V = Param->DefaultValue;
            FString Group = Param->Group.ToString();
            FString Desc = Param->Desc;

            OutText += FString::Printf(TEXT("| %s | %.3g | %.3g | %.3g | %.3g | %s | %s |\n"),
                *Name, V.R, V.G, V.B, V.A, *Group, *Desc);
        }

        OutText += TEXT("\n");
    }

    // Texture Parameters
    if (TextureParams.Num() > 0)
    {
        OutText += TEXT("## Texture Parameters\n");
        OutText += TEXT("| Name | Default Texture | Group | Description |\n");
        OutText += TEXT("|------|----------------|-------|-------------|\n");

        for (UMaterialExpressionTextureObjectParameter* Param : TextureParams)
        {
            FString Name = Param->ParameterName.ToString();
            FString TexName = Param->Texture ? Param->Texture->GetName() : TEXT("None");
            FString Group = Param->Group.ToString();
            FString Desc = Param->Desc;

            OutText += FString::Printf(TEXT("| %s | %s | %s | %s |\n"),
                *Name, *TexName, *Group, *Desc);
        }

        OutText += TEXT("\n");
    }

    // Если параметров нет
    if (ScalarParams.Num() == 0 && VectorParams.Num() == 0 && TextureParams.Num() == 0)
    {
        OutText += TEXT("## Parameters\n");
        OutText += TEXT("_No exposed parameters found._\n\n");
    }
}

void BPR_Extractor_Material::AppendMaterialExpressions(UMaterial* Material, FString& OutText)
{
    if (!Material) return;

    OutText += TEXT("## Expressions\n");

    TConstArrayView<TObjectPtr<UMaterialExpression>> Expressions = Material->GetExpressions();

    if (Expressions.Num() == 0)
    {
        OutText += TEXT("_No expressions found._\n");
        return;
    }


    OutText += FString::Printf(TEXT("Total expressions: %d\n\n"), Expressions.Num());

    // Группируем по типам для удобства
    TMap<FString, int32> ExpressionCounts;

    for (UMaterialExpression* Expr : Expressions)
    {
        if (!Expr) continue;

        FString TypeName = Expr->GetClass()->GetName();
        // Убираем префикс "MaterialExpression"
        TypeName.RemoveFromStart(TEXT("MaterialExpression"));

        ExpressionCounts.FindOrAdd(TypeName)++;
    }

    // Выводим статистику по типам
    OutText += TEXT("### Expression Types:\n");
    OutText += TEXT("| Type | Count |\n");
    OutText += TEXT("|------|-------|\n");

    // Сортируем по количеству
    ExpressionCounts.ValueSort([](int32 A, int32 B) { return A > B; });

    for (const auto& Pair : ExpressionCounts)
    {
        OutText += FString::Printf(TEXT("| %s | %d |\n"), *Pair.Key, Pair.Value);
    }

    OutText += TEXT("\n### Expression List:\n");

    for (UMaterialExpression* Expr : Expressions)
    {
        if (!Expr) continue;

        FString Description = GetExpressionDescription(Expr);
        OutText += FString::Printf(TEXT("- %s\n"), *Description);
    }

    OutText += TEXT("\n");
}

//==============================================================================
// MaterialInstance Processing
//==============================================================================
void BPR_Extractor_Material::ProcessMaterialInstanceAsset(UMaterialInstance* MatInst, FString& OutStructure, FString& OutGraph)
{
    if (!MatInst) return;

    AppendInstanceInfo(MatInst, OutStructure);
    AppendParameterOverrides(MatInst, OutStructure);

    // График инстанса ссылается на родителя
    OutGraph += TEXT("## Graph Information\n");
    OutGraph += TEXT("MaterialInstance does not have its own expression graph.\n");
    OutGraph += TEXT("The graph is inherited from the parent material.\n\n");

    if (MatInst->Parent)
    {
        OutGraph += FString::Printf(TEXT("**Parent Material:** %s\n"), 
            *MatInst->Parent->GetName());
        OutGraph += TEXT("_To view the expression graph, open the parent material._\n");
    }
}

void BPR_Extractor_Material::AppendInstanceInfo(UMaterialInstance* MatInst, FString& OutText)
{
    if (!MatInst) return;

    OutText += TEXT("## Instance Information\n");
    OutText += TEXT("| Property | Value |\n");
    OutText += TEXT("|----------|-------|\n");

    // Parent
    if (MatInst->Parent)
    {
        OutText += FString::Printf(TEXT("| Parent Material | %s |\n"), 
            *MatInst->Parent->GetName());

        // Если родитель тоже инстанс - показываем цепочку
        if (UMaterialInstance* ParentInst = Cast<UMaterialInstance>(MatInst->Parent.Get()))
        {
            OutText += FString::Printf(TEXT("| Parent Type | MaterialInstance |\n"));
            
            // Ищем корневой материал
            UMaterial* BaseMaterial = MatInst->GetBaseMaterial();
            if (BaseMaterial)
            {
                OutText += FString::Printf(TEXT("| Base Material | %s |\n"), 
                    *BaseMaterial->GetName());
            }
        }
        else if (UMaterial* ParentMat = Cast<UMaterial>(MatInst->Parent.Get()))
        {
            OutText += FString::Printf(TEXT("| Parent Type | Material |\n"));
        }
    }
    else
    {
        OutText += TEXT("| Parent Material | None |\n");
    }

    OutText += TEXT("\n");

    // Если есть родитель-материал, показываем его свойства
    if (UMaterial* BaseMat = MatInst->GetBaseMaterial())
    {
        OutText += TEXT("## Inherited Properties (from base material)\n");
        AppendMaterialProperties(BaseMat, OutText);
    }
}

void BPR_Extractor_Material::AppendParameterOverrides(UMaterialInstance* MatInst, FString& OutText)
{
    if (!MatInst) return;

    bool HasOverrides = false;

    // Scalar Parameters
    if (MatInst->ScalarParameterValues.Num() > 0)
    {
        OutText += TEXT("## Scalar Parameter Overrides\n");
        OutText += TEXT("| Name | Value |\n");
        OutText += TEXT("|------|-------|\n");

        for (const FScalarParameterValue& Param : MatInst->ScalarParameterValues)
        {
            FString Name = Param.ParameterInfo.Name.ToString();
            OutText += FString::Printf(TEXT("| %s | %.6g |\n"), *Name, Param.ParameterValue);
        }

        OutText += TEXT("\n");
        HasOverrides = true;
    }

    // Vector Parameters
    if (MatInst->VectorParameterValues.Num() > 0)
    {
        OutText += TEXT("## Vector Parameter Overrides\n");
        OutText += TEXT("| Name | R | G | B | A |\n");
        OutText += TEXT("|------|---|---|---|---|\n");

        for (const FVectorParameterValue& Param : MatInst->VectorParameterValues)
        {
            FString Name = Param.ParameterInfo.Name.ToString();
            const FLinearColor& V = Param.ParameterValue;
            OutText += FString::Printf(TEXT("| %s | %.3g | %.3g | %.3g | %.3g |\n"),
                *Name, V.R, V.G, V.B, V.A);
        }

        OutText += TEXT("\n");
        HasOverrides = true;
    }

    // Texture Parameters
    if (MatInst->TextureParameterValues.Num() > 0)
    {
        OutText += TEXT("## Texture Parameter Overrides\n");
        OutText += TEXT("| Name | Texture |\n");
        OutText += TEXT("|------|----------|\n");

        for (const FTextureParameterValue& Param : MatInst->TextureParameterValues)
        {
            FString Name = Param.ParameterInfo.Name.ToString();
            FString TexName = Param.ParameterValue ? Param.ParameterValue->GetName() : TEXT("None");
            OutText += FString::Printf(TEXT("| %s | %s |\n"), *Name, *TexName);
        }

        OutText += TEXT("\n");
        HasOverrides = true;
    }

    if (!HasOverrides)
    {
        OutText += TEXT("## Parameter Overrides\n");
        OutText += TEXT("_No parameter overrides found._\n\n");
    }
}

//==============================================================================
// Helper Methods
//==============================================================================
FString BPR_Extractor_Material::GetExpressionDescription(UMaterialExpression* Expression)
{
    if (!Expression) return TEXT("None");

    FString TypeName = Expression->GetClass()->GetName();
    TypeName.RemoveFromStart(TEXT("MaterialExpression"));

    // TextureSample
    if (UMaterialExpressionTextureSample* TexSample = Cast<UMaterialExpressionTextureSample>(Expression))
    {
        FString TexName = TexSample->Texture ? TexSample->Texture->GetName() : TEXT("None");
        return FString::Printf(TEXT("%s: %s"), *TypeName, *TexName);
    }

    // ScalarParameter
    if (UMaterialExpressionScalarParameter* ScalarParam = Cast<UMaterialExpressionScalarParameter>(Expression))
    {
        return FString::Printf(TEXT("%s: %s (%.3g)"), 
            *TypeName, 
            *ScalarParam->ParameterName.ToString(), 
            ScalarParam->DefaultValue);
    }

    // VectorParameter
    if (UMaterialExpressionVectorParameter* VectorParam = Cast<UMaterialExpressionVectorParameter>(Expression))
    {
        const FLinearColor& V = VectorParam->DefaultValue;
        return FString::Printf(TEXT("%s: %s (%.2g, %.2g, %.2g, %.2g)"), 
            *TypeName,
            *VectorParam->ParameterName.ToString(),
            V.R, V.G, V.B, V.A);
    }

    // TextureParameter
    if (UMaterialExpressionTextureObjectParameter* TexParam = Cast<UMaterialExpressionTextureObjectParameter>(Expression))
    {
        FString TexName = TexParam->Texture ? TexParam->Texture->GetName() : TEXT("None");
        return FString::Printf(TEXT("%s: %s = %s"), 
            *TypeName,
            *TexParam->ParameterName.ToString(),
            *TexName);
    }

    // Constant
    if (UMaterialExpressionConstant* Const = Cast<UMaterialExpressionConstant>(Expression))
    {
        return FString::Printf(TEXT("%s: %.6g"), *TypeName, Const->R);
    }

    // Fallback - просто тип
    return TypeName;
}

FString BPR_Extractor_Material::GetShadingModelName(EMaterialShadingModel Model)
{
    switch (Model)
    {
        case MSM_Unlit: return TEXT("Unlit");
        case MSM_DefaultLit: return TEXT("DefaultLit");
        case MSM_Subsurface: return TEXT("Subsurface");
        case MSM_PreintegratedSkin: return TEXT("PreintegratedSkin");
        case MSM_ClearCoat: return TEXT("ClearCoat");
        case MSM_SubsurfaceProfile: return TEXT("SubsurfaceProfile");
        case MSM_TwoSidedFoliage: return TEXT("TwoSidedFoliage");
        case MSM_Hair: return TEXT("Hair");
        case MSM_Cloth: return TEXT("Cloth");
        case MSM_Eye: return TEXT("Eye");
        case MSM_SingleLayerWater: return TEXT("SingleLayerWater");
        case MSM_ThinTranslucent: return TEXT("ThinTranslucent");
        default: return TEXT("Unknown");
    }
}

FString BPR_Extractor_Material::GetBlendModeName(EBlendMode Mode)
{
    switch (Mode)
    {
        case BLEND_Opaque: return TEXT("Opaque");
        case BLEND_Masked: return TEXT("Masked");
        case BLEND_Translucent: return TEXT("Translucent");
        case BLEND_Additive: return TEXT("Additive");
        case BLEND_Modulate: return TEXT("Modulate");
        case BLEND_AlphaComposite: return TEXT("AlphaComposite");
        case BLEND_AlphaHoldout: return TEXT("AlphaHoldout");
        default: return TEXT("Unknown");
    }
}