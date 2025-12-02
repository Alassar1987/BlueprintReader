#include "Extractors/BPR_Extractor_Material.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Engine/Texture.h"
#include "Engine/EngineTypes.h" // FMaterialShadingModelField
#include "Materials/MaterialInstanceConstant.h"

BPR_Extractor_Material::BPR_Extractor_Material() {}
BPR_Extractor_Material::~BPR_Extractor_Material() {}

void BPR_Extractor_Material::ProcessMaterial(UObject* SelectedObject, FBPR_ExtractedData& OutData)
{
#if WITH_EDITOR
    if (!SelectedObject)
    {
        LogError(TEXT("SelectedObject is null!"));
        OutData.Structure = FText::FromString("Error: SelectedObject is null.");
        OutData.Graph = FText::FromString("Error: SelectedObject is null.");
        return;
    }

    FString TmpTextStructure;
    FString TmpTextGraph;

    // Сначала проверяем обычный материал
    if (UMaterial* Material = Cast<UMaterial>(SelectedObject))
    {
        TmpTextStructure = FString::Printf(TEXT("# Material Structure: %s\n\n"), *Material->GetName());
        TmpTextGraph     = FString::Printf(TEXT("# Material Graph: %s\n\n"), *Material->GetName());

        AppendMaterialProperties(Material, TmpTextStructure);
        AppendMaterialExpressions(Material, TmpTextGraph);
    }
    // Если инстанс материала
    else if (UMaterialInstance* MatInst = Cast<UMaterialInstance>(SelectedObject))
    {
        TmpTextStructure = FString::Printf(TEXT("# Material Instance Structure: %s\n\n"), *MatInst->GetName());
        TmpTextGraph     = FString::Printf(TEXT("# Material Instance Graph: %s\n\n"), *MatInst->GetName());

        // Структура инстанса = родитель + переопределенные параметры
    	if (MatInst->Parent)
    	{
    		if (UMaterial* ParentMat = Cast<UMaterial>(MatInst->Parent.Get()))
    		{
    			TmpTextStructure += FString::Printf(TEXT("**Parent Material:** %s\n\n"), *ParentMat->GetName());
    			AppendMaterialProperties(ParentMat, TmpTextStructure);
    		}
    	}


        // Параметры инстанса
        if (MatInst->ScalarParameterValues.Num() > 0 || MatInst->VectorParameterValues.Num() > 0 || MatInst->TextureParameterValues.Num() > 0)
        {
            TmpTextStructure += TEXT("\n## Parameter Overrides\n\n");

            for (const FScalarParameterValue& Param : MatInst->ScalarParameterValues)
            {
                FString Name = Param.ParameterInfo.Name.IsNone() ? TEXT("Unnamed") : Param.ParameterInfo.Name.ToString();
                TmpTextStructure += FString::Printf(TEXT("- %s: %.6g\n"), *Name, Param.ParameterValue);
            }

            for (const FVectorParameterValue& Param : MatInst->VectorParameterValues)
            {
                const FLinearColor& V = Param.ParameterValue;
                FString Name = Param.ParameterInfo.Name.IsNone() ? TEXT("Unnamed") : Param.ParameterInfo.Name.ToString();
                TmpTextStructure += FString::Printf(TEXT("- %s: (%.6g, %.6g, %.6g, %.6g)\n"), *Name, V.R, V.G, V.B, V.A);
            }

            for (const FTextureParameterValue& Param : MatInst->TextureParameterValues)
            {
                FString Name = Param.ParameterInfo.Name.IsNone() ? TEXT("Unnamed") : Param.ParameterInfo.Name.ToString();
                FString TexName = Param.ParameterValue ? Param.ParameterValue->GetName() : TEXT("None");
                TmpTextStructure += FString::Printf(TEXT("- %s: %s\n"), *Name, *TexName);
            }
        }
        else
        {
            TmpTextStructure += TEXT("_No parameter overrides found._\n");
        }

        // Графа собственного нет, можно оставить пустым или указать на родителя
        TmpTextGraph += TEXT("_MaterialInstance has no own expressions. Graph is in parent material._\n");
    }
    else
    {
        LogWarning(TEXT("Selected object is not a Material or MaterialInstance."));
        TmpTextStructure = TEXT("Warning: Selected object is not a Material or MaterialInstance.");
        TmpTextGraph     = TEXT("Warning: Selected object is not a Material or MaterialInstance.");
    }

    OutData.Structure = FText::FromString(TmpTextStructure);
    OutData.Graph     = FText::FromString(TmpTextGraph);
#endif
}



//--------------------
// Логирование
//--------------------
void BPR_Extractor_Material::LogMessage(const FString& Msg) { UE_LOG(LogTemp, Log, TEXT("[BPRBPR_Extractor_Material] %s"), *Msg); }
void BPR_Extractor_Material::LogWarning(const FString& Msg) { UE_LOG(LogTemp, Warning, TEXT("[BPRBPR_Extractor_Material] %s"), *Msg); }
void BPR_Extractor_Material::LogError(const FString& Msg) { UE_LOG(LogTemp, Error, TEXT("[BPRBPR_Extractor_Material] %s"), *Msg); }

//--------------------
// Свойства материала
//--------------------
void BPR_Extractor_Material::AppendMaterialProperties(UMaterial* Material, FString& OutText)
{
#if WITH_EDITOR
	if (!Material) return;

	OutText += TEXT("## Material Properties\n");

	// TwoSided
	OutText += FString::Printf(TEXT("- TwoSided: %s\n"), Material->IsTwoSided() ? TEXT("true") : TEXT("false"));

	// BlendMode
	UEnum* BlendEnum = StaticEnum<EBlendMode>();
	FString BlendStr = BlendEnum ? BlendEnum->GetNameStringByValue((int64)Material->GetBlendMode()) : TEXT("Unknown");
	OutText += FString::Printf(TEXT("- BlendMode: %s\n"), *BlendStr);

	// ShadingModels
	FMaterialShadingModelField ShadingModels = Material->GetShadingModels();
	FString ShadingModelsStr;
	for (uint32 i = 0; i < uint32(MSM_MAX); ++i)
	{
		if (ShadingModels.HasShadingModel((EMaterialShadingModel)i))
			ShadingModelsStr += FString::Printf(TEXT("%s "), *UEnum::GetValueAsString((EMaterialShadingModel)i));
	}
	OutText += FString::Printf(TEXT("- ShadingModels: %s\n"), *ShadingModelsStr);
#endif
}

//--------------------
// Expression-ноды
//--------------------
void BPR_Extractor_Material::AppendMaterialExpressions(UMaterial* Material, FString& OutText)
{
#if WITH_EDITOR
	if (!Material) return;

	OutText += TEXT("\n## Expressions\n");

	// Обходим все Expression в проекте и фильтруем по материалу
	for (TObjectIterator<UMaterialExpression> It; It; ++It)
	{
		UMaterialExpression* Expression = *It;
		if (!Expression || Expression->Material != Material) continue;

		OutText += FString::Printf(TEXT("- %s\n"), *GetExpressionDescription(Expression));
	}
#endif
}

FString BPR_Extractor_Material::GetExpressionDescription(UMaterialExpression* Expression)
{
	if (!Expression) return TEXT("None");

#if WITH_EDITOR
	if (UMaterialExpressionTextureSample* TexSample = Cast<UMaterialExpressionTextureSample>(Expression))
	{
		FString TexName = TexSample->Texture ? TexSample->Texture->GetName() : TEXT("None");
		return FString::Printf(TEXT("TextureSample: %s"), *TexName);
	}
#endif

	return Expression->GetName();
}
