#include "Extractors/BPR_Extractor_Material.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Engine/Texture.h"
#include "Engine/EngineTypes.h" // FMaterialShadingModelField

BPR_Extractor_Material::BPR_Extractor_Material() {}
BPR_Extractor_Material::~BPR_Extractor_Material() {}

void BPR_Extractor_Material::ProcessMaterial(UObject* SelectedObject, FText& OutText)
{
	if (!SelectedObject)
	{
		LogError(TEXT("SelectedObject is null!"));
		OutText = FText::FromString("Error: SelectedObject is null.");
		return;
	}

	UMaterial* Material = Cast<UMaterial>(SelectedObject);
	if (!Material)
	{
		LogWarning(TEXT("Selected object is not a Material."));
		OutText = FText::FromString("Warning: Selected object is not a Material.");
		return;
	}

	FString TmpText = FString::Printf(TEXT("# Material Export: %s\n\n"), *Material->GetName());

	AppendMaterialProperties(Material, TmpText);
	AppendMaterialExpressions(Material, TmpText);

	OutText = FText::FromString(TmpText);
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
