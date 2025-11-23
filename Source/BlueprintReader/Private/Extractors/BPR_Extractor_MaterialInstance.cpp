#include "Extractors/BPR_Extractor_MaterialInstance.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/Material.h"

BPR_Extractor_MaterialInstance::BPR_Extractor_MaterialInstance() {}
BPR_Extractor_MaterialInstance::~BPR_Extractor_MaterialInstance() {}

void BPR_Extractor_MaterialInstance::ProcessMaterialInstance(UObject* SelectedObject, FText& OutText)
{
    if (!SelectedObject)
    {
        LogError(TEXT("SelectedObject is null!"));
        OutText = FText::FromString("Error: SelectedObject is null.");
        return;
    }

    UMaterialInstance* MI = Cast<UMaterialInstance>(SelectedObject);
    if (!MI)
    {
        LogWarning(TEXT("Selected object is not a MaterialInstance."));
        OutText = FText::FromString("Warning: Selected object is not a MaterialInstance.");
        return;
    }

    FString TmpText = FString::Printf(TEXT("# MaterialInstance Export: %s\n\n"), *MI->GetName());

    AppendMaterialInstanceParameters(MI, TmpText);

    OutText = FText::FromString(TmpText);
}

//--------------------
// Логирование
//--------------------
void BPR_Extractor_MaterialInstance::LogMessage(const FString& Msg) { UE_LOG(LogTemp, Log, TEXT("[BPRBPR_Extractor_MaterialInstance] %s"), *Msg); }
void BPR_Extractor_MaterialInstance::LogWarning(const FString& Msg) { UE_LOG(LogTemp, Warning, TEXT("[BPRBPR_Extractor_MaterialInstance] %s"), *Msg); }
void BPR_Extractor_MaterialInstance::LogError(const FString& Msg) { UE_LOG(LogTemp, Error, TEXT("[BPRBPR_Extractor_MaterialInstance] %s"), *Msg); }

//--------------------
// Параметры MaterialInstance
//--------------------
void BPR_Extractor_MaterialInstance::AppendMaterialInstanceParameters(UMaterialInstance* MI, FString& OutText)
{
#if WITH_EDITOR
    if (!MI) return;

    OutText += TEXT("## Parameters\n");

    TArray<FMaterialParameterInfo> ParameterInfos;
    TArray<FGuid> ParameterIds;

    // Scalar параметры
    MI->GetAllScalarParameterInfo(ParameterInfos, ParameterIds);
    for (const FMaterialParameterInfo& Info : ParameterInfos)
    {
        float Value;
        if (MI->GetScalarParameterValue(Info, Value))
        {
            OutText += FString::Printf(TEXT("- Scalar: %s = %f\n"), *Info.Name.ToString(), Value);
        }
    }

    ParameterInfos.Reset();
    ParameterIds.Reset();

    // Vector параметры
    MI->GetAllVectorParameterInfo(ParameterInfos, ParameterIds);
    for (const FMaterialParameterInfo& Info : ParameterInfos)
    {
        FLinearColor Value;
        if (MI->GetVectorParameterValue(Info, Value))
        {
            OutText += FString::Printf(TEXT("- Vector: %s = (%f, %f, %f, %f)\n"),
                *Info.Name.ToString(), Value.R, Value.G, Value.B, Value.A);
        }
    }

    ParameterInfos.Reset();
    ParameterIds.Reset();

    // Texture параметры
    MI->GetAllTextureParameterInfo(ParameterInfos, ParameterIds);
    for (const FMaterialParameterInfo& Info : ParameterInfos)
    {
        UTexture* Value = nullptr;
        if (MI->GetTextureParameterValue(Info, Value))
        {
            FString TexName = Value ? Value->GetName() : TEXT("None");
            OutText += FString::Printf(TEXT("- Texture: %s = %s\n"), *Info.Name.ToString(), *TexName);
        }
    }
#endif
}
