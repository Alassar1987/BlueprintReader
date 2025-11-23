#include "Extractors/BPR_Extractor_MFInstance.h"
#include "Materials/MaterialFunctionInstance.h"
#include "Materials/MaterialFunction.h"

void BPR_Extractor_MFInstance::ProcessMFInstance(UObject* Object, FText& OutText)
{
#if WITH_EDITOR
    FString Result;

    UMaterialFunctionInstance* Instance = Cast<UMaterialFunctionInstance>(Object);
    if (!Instance)
    {
        OutText = FText::FromString("Object is not a Material Function Instance.");
        return;
    }

    Result += TEXT("# Material Function Instance\n\n");

    AppendBasicInfo(Instance, Result);
    AppendOverrides(Instance, Result);

    OutText = FText::FromString(Result);

#endif
}

void BPR_Extractor_MFInstance::AppendBasicInfo(UMaterialFunctionInstance* Instance, FString& OutText)
{
    OutText += TEXT("## Basic Info\n\n");

    OutText += FString::Printf(TEXT("- **Name:** %s\n"), *Instance->GetName());

    if (Instance->Parent)
    {
        OutText += FString::Printf(TEXT("- **Parent Function:** %s\n"),
            *Instance->Parent->GetName());
    }
    else
    {
        OutText += TEXT("- **Parent Function:** None\n");
    }

    OutText += TEXT("\n");
}

void BPR_Extractor_MFInstance::AppendOverrides(UMaterialFunctionInstance* Instance, FString& OutText)
{
    OutText += TEXT("## Overrides\n\n");

    // --- Scalar Parameters ---
    if (Instance->ScalarParameterValues.Num() > 0)
    {
        OutText += TEXT("### Scalar Parameters\n");

        for (const FScalarParameterValue& P : Instance->ScalarParameterValues)
        {
            FString ParamName = P.ParameterInfo.Name.ToString();
            OutText += FString::Printf(TEXT("- **%s:** %f\n"),
                *ParamName,
                P.ParameterValue);
        }

        OutText += TEXT("\n");
    }

    // --- Vector Parameters ---
    if (Instance->VectorParameterValues.Num() > 0)
    {
        OutText += TEXT("### Vector Parameters\n");

        for (const FVectorParameterValue& P : Instance->VectorParameterValues)
        {
            FString ParamName = P.ParameterInfo.Name.ToString();
            const FLinearColor& C = P.ParameterValue;

            OutText += FString::Printf(
                TEXT("- **%s:** R=%f, G=%f, B=%f, A=%f\n"),
                *ParamName, C.R, C.G, C.B, C.A
            );
        }

        OutText += TEXT("\n");
    }

    // --- Texture parameters ---
    if (Instance->TextureParameterValues.Num() > 0)
    {
        OutText += TEXT("### Texture Parameters\n");

        for (const FTextureParameterValue& P : Instance->TextureParameterValues)
        {
            FString ParamName = P.ParameterInfo.Name.ToString();
            FString TexName = P.ParameterValue ? P.ParameterValue->GetName() : TEXT("None");

            OutText += FString::Printf(TEXT("- **%s:** %s\n"),
                *ParamName,
                *TexName);
        }

        OutText += TEXT("\n");
    }

    // --- Static Switch parameters ---
    if (Instance->StaticSwitchParameterValues.Num() > 0)
    {
        OutText += TEXT("### Static Switch Parameters\n");

        for (const FStaticSwitchParameter& P : Instance->StaticSwitchParameterValues)
        {
            FString ParamName = P.ParameterInfo.Name.ToString();

            OutText += FString::Printf(TEXT("- **%s:** %s\n"),
                *ParamName,
                P.bOverride ? (P.Value ? TEXT("Enabled") : TEXT("Disabled")) : TEXT("Not Overridden"));
        }

        OutText += TEXT("\n");
    }

    // --- Static Component Mask parameters ---
    if (Instance->StaticComponentMaskParameterValues.Num() > 0)
    {
        OutText += TEXT("### Static Component Mask Parameters\n");

        for (const FStaticComponentMaskParameter& P : Instance->StaticComponentMaskParameterValues)
        {
            FString ParamName = P.ParameterInfo.Name.ToString();

            OutText += FString::Printf(TEXT("- **%s:** R=%s, G=%s, B=%s, A=%s\n"),
                *ParamName,
                P.R ? TEXT("True") : TEXT("False"),
                P.G ? TEXT("True") : TEXT("False"),
                P.B ? TEXT("True") : TEXT("False"),
                P.A ? TEXT("True") : TEXT("False"));
        }

        OutText += TEXT("\n");
    }

    // Check if no overrides exist
    bool bHasAnyOverrides = 
        Instance->ScalarParameterValues.Num() > 0 ||
        Instance->VectorParameterValues.Num() > 0 ||
        Instance->TextureParameterValues.Num() > 0 ||
        Instance->StaticSwitchParameterValues.Num() > 0 ||
        Instance->StaticComponentMaskParameterValues.Num() > 0;

    if (!bHasAnyOverrides)
    {
        OutText += TEXT("_This Material Function Instance has no overrides._\n\n");
    }
}