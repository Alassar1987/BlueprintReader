#include "Extractors/BPR_Extractor_MaterialFunction.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialFunctionInstance.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionFunctionInput.h"
#include "Materials/MaterialExpressionFunctionOutput.h"

#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionComment.h"
#include "Materials/MaterialExpressionTextureSample.h" // если хотим показывать текстуры
#include "Engine/Texture.h" // для получения имен текстур

#include "Materials/MaterialFunctionInterface.h" // для GetExpressions()
#include "UObject/ObjectPtr.h"                   // для TObjectPtr
#include "Containers/ArrayView.h"                // для TConstArrayView




#include "Engine/Engine.h"

BPR_Extractor_MaterialFunction::BPR_Extractor_MaterialFunction() {}
BPR_Extractor_MaterialFunction::~BPR_Extractor_MaterialFunction() {}

void BPR_Extractor_MaterialFunction::ProcessMaterialFunction(UObject* SelectedObject, FText& OutText)
{
#if WITH_EDITOR
    if (!SelectedObject)
    {
        LogError(TEXT("SelectedObject is null!"));
        OutText = FText::FromString("Error: SelectedObject is null.");
        return;
    }

    FString Result;

    if (IsMaterialFunction(SelectedObject))
    {
        UMaterialFunction* Func = Cast<UMaterialFunction>(SelectedObject);
        Result += FString::Printf(TEXT("# Material Function Export: %s\n\n"), *Func->GetName());
        ExtractMaterialFunction(Func, Result);
    }
    else if (IsMaterialFunctionInstance(SelectedObject))
    {
        UMaterialFunctionInstance* Instance = Cast<UMaterialFunctionInstance>(SelectedObject);
        Result += FString::Printf(TEXT("# Material Function Instance Export: %s\n\n"), *Instance->GetName());
        ExtractMaterialFunctionInstance(Instance, Result);
    }
    else
    {
        LogWarning(TEXT("Selected object is not a Material Function or Material Function Instance."));
        OutText = FText::FromString("Warning: Selected object is not a Material Function or Material Function Instance.");
        return;
    }

    OutText = FText::FromString(Result);
#endif
}

// ----------------------------------------------------------
// Проверки типов
// ----------------------------------------------------------

bool BPR_Extractor_MaterialFunction::IsMaterialFunction(UObject* Obj) const
{
    return Obj && Obj->IsA<UMaterialFunction>();
}

bool BPR_Extractor_MaterialFunction::IsMaterialFunctionInstance(UObject* Obj) const
{
    return Obj && Obj->IsA<UMaterialFunctionInstance>();
}

// ----------------------------------------------------------
// Основные обработчики
// ----------------------------------------------------------

void BPR_Extractor_MaterialFunction::ExtractMaterialFunction(UMaterialFunction* Function, FString& OutText)
{
    if (!Function) return;

    AppendFunctionInputs(Function, OutText);
    OutText += TEXT("\n");

    AppendFunctionOutputs(Function, OutText);
    OutText += TEXT("\n");

    AppendFunctionExpressions(Function, OutText);
    OutText += TEXT("\n");
}

void BPR_Extractor_MaterialFunction::ExtractMaterialFunctionInstance(UMaterialFunctionInstance* Instance, FString& OutText)
{
    if (!Instance) return;

    UObject* Parent = Instance->Parent;
    OutText += FString::Printf(TEXT("**Parent Function:** %s\n\n"),
        Parent ? *Parent->GetName() : TEXT("None"));

    AppendInstanceOverrides(Instance, OutText);
}

// ----------------------------------------------------------
// Подсекции (пока пустые заглушки — наполним позже)
// ----------------------------------------------------------

void BPR_Extractor_MaterialFunction::AppendFunctionInputs(UMaterialFunction* Function, FString& OutText)
{
#if WITH_EDITOR

    OutText += TEXT("## Inputs\n\n");

    if (!Function)
    {
        OutText += TEXT("Function is null.\n\n");
        return;
    }

    // Получаем view на список выражений внутри функции (UE5.7)
    auto ExpressionsView = Function->GetExpressions();

    // Собираем ноды типа UMaterialExpressionFunctionInput
    TArray<UMaterialExpressionFunctionInput*> InputNodes;
    InputNodes.Reserve(ExpressionsView.Num());

    for (TObjectPtr<UMaterialExpression> ExprPtr : ExpressionsView)
    {
        UMaterialExpression* Expr = ExprPtr.Get();
        if (!Expr) continue;

        if (UMaterialExpressionFunctionInput* InputNode = Cast<UMaterialExpressionFunctionInput>(Expr))
        {
            InputNodes.Add(InputNode);
        }
    }

    if (InputNodes.Num() == 0)
    {
        OutText += TEXT("_This function has no inputs._\n\n");
        return;
    }

    int32 Index = 1;
    for (UMaterialExpressionFunctionInput* Input : InputNodes)
    {
        if (!Input) continue;

        FString Name = Input->InputName.IsNone() ? TEXT("UnnamedInput") : Input->InputName.ToString();
        OutText += FString::Printf(TEXT("### %d) %s\n"), Index++, *Name);

        // Тип входа — GetInputTypeDisplayName возвращает FString в UE5.x
        {
            FString TypeStr = UMaterialExpressionFunctionInput::GetInputTypeDisplayName(static_cast<EFunctionInputType>(Input->InputType));
            OutText += FString::Printf(TEXT("- **Type:** %s\n"), *TypeStr);
        }

        // Default / Preview value (если используется)
        if (Input->bUsePreviewValueAsDefault)
        {
            // PreviewValue — обычно FVector4f / FLinearColor-совместимый
            // Приводим к double для корректного форматирования
            const auto& V = Input->PreviewValue;
            OutText += FString::Printf(TEXT("- **Default (preview):** (%.6g, %.6g, %.6g, %.6g)\n"),
                                      static_cast<double>(V.X), static_cast<double>(V.Y),
                                      static_cast<double>(V.Z), static_cast<double>(V.W));
        }
        else
        {
            OutText += TEXT("- **Default:** None\n");
        }

        // Sort priority
        OutText += FString::Printf(TEXT("- **SortPriority:** %d\n"), Input->SortPriority);

        // Optional: preview expression
        if (Input->Preview.Expression)
        {
            if (UMaterialExpression* PreviewExpr = Input->Preview.Expression)
            {
                OutText += FString::Printf(TEXT("- **Preview connected to:** %s\n"), *PreviewExpr->GetClass()->GetName());
            }
        }

        OutText += TEXT("\n");
    }

#endif // WITH_EDITOR
}




void BPR_Extractor_MaterialFunction::AppendFunctionOutputs(UMaterialFunction* Function, FString& OutText)
{
#if WITH_EDITOR

    OutText += TEXT("## Outputs\n\n");

    if (!Function)
    {
        OutText += TEXT("Function is null.\n");
        return;
    }

    TArrayView<const TObjectPtr<UMaterialExpression>> Expressions = Function->GetExpressions();

    TArray<UMaterialExpressionFunctionOutput*> Outputs;

    for (UMaterialExpression* Expr : Expressions)
    {
        if (auto* Out = Cast<UMaterialExpressionFunctionOutput>(Expr))
        {
            Outputs.Add(Out);
        }
    }

    if (Outputs.Num() == 0)
    {
        OutText += TEXT("_This function has no outputs._\n");
        return;
    }

    for (int32 i = 0; i < Outputs.Num(); ++i)
    {
        const UMaterialExpressionFunctionOutput* Output = Outputs[i];

        FString Name = Output->OutputName.IsNone()
            ? TEXT("UnnamedOutput")
            : Output->OutputName.ToString();

        OutText += FString::Printf(TEXT("### %d) %s\n"), i + 1, *Name);

        OutText += FString::Printf(TEXT("- **SortPriority:** %d\n"), Output->SortPriority);
        OutText += FString::Printf(TEXT("- **Previewed:** %s\n"), Output->bLastPreviewed ? TEXT("true") : TEXT("false"));

        // Проверяем подключение
        if (Output->A.Expression)
        {
            OutText += FString::Printf(
                TEXT("- **Connected from:** %s\n"),
                *Output->A.Expression->GetClass()->GetName()
            );
        }
        else
        {
            OutText += TEXT("- **Connected from:** _None_\n");
        }

        OutText += TEXT("\n");
    }

#endif // WITH_EDITOR
}


void BPR_Extractor_MaterialFunction::AppendFunctionExpressions(UMaterialFunction* Function, FString& OutText)
{
#if WITH_EDITOR
    OutText += TEXT("## Expressions\n\n");

    if (!Function)
    {
        OutText += TEXT("Function is null.\n");
        return;
    }

    TConstArrayView<TObjectPtr<UMaterialExpression>> Expressions = Function->GetExpressions();

    for (int32 i = 0; i < Expressions.Num(); ++i)
    {
        UMaterialExpression* Expr = Expressions[i].Get(); // TObjectPtr -> обычный указатель
        if (!Expr) continue;

        FString ExprName = Expr->GetName();
        FString ExprClass = Expr->GetClass()->GetName();

        OutText += FString::Printf(TEXT("### %d) %s (%s)\n"), i + 1, *ExprName, *ExprClass);

        if (UMaterialExpressionFunctionInput* Input = Cast<UMaterialExpressionFunctionInput>(Expr))
        {
            FString InputNameStr = Input->InputName.IsNone() ? TEXT("UnnamedInput") : Input->InputName.ToString();
            OutText += FString::Printf(TEXT("- Input Name: %s\n"), *InputNameStr);
        }
        else if (UMaterialExpressionFunctionOutput* Output = Cast<UMaterialExpressionFunctionOutput>(Expr))
        {
            FString OutputNameStr = Output->OutputName.IsNone() ? TEXT("UnnamedOutput") : Output->OutputName.ToString();
            OutText += FString::Printf(TEXT("- Output Name: %s\n"), *OutputNameStr);
        }
        else if (UMaterialExpressionComment* Comment = Cast<UMaterialExpressionComment>(Expr))
        {
            FString CommentStr = Comment->Text; // FText -> FString
            OutText += FString::Printf(TEXT("- Comment: %s\n"), *CommentStr);
        }
        else if (UMaterialExpressionTextureSample* Tex = Cast<UMaterialExpressionTextureSample>(Expr))
        {
            FString TexName = Tex->Texture ? Tex->Texture->GetName() : TEXT("None");
            OutText += FString::Printf(TEXT("- Texture: %s\n"), *TexName);
        }

        OutText += TEXT("\n");
    }
#endif
}





void BPR_Extractor_MaterialFunction::AppendInstanceOverrides(UMaterialFunctionInstance* Instance, FString& OutText)
{
#if WITH_EDITOR
    OutText += TEXT("## Parameter Overrides\n\n");

    if (!Instance)
    {
        OutText += TEXT("_Instance is null._\n");
        return;
    }

    // ----------------------------------------------------------
    // Scalar Parameters
    // ----------------------------------------------------------
    if (Instance->ScalarParameterValues.Num() > 0)
    {
        OutText += TEXT("### Scalar Parameters\n\n");
        for (const FScalarParameterValue& Param : Instance->ScalarParameterValues)
        {
            FString NameStr = Param.ParameterInfo.Name.IsNone() ? TEXT("Unnamed") : Param.ParameterInfo.Name.ToString();
            OutText += FString::Printf(TEXT("- %s: %.6g\n"), *NameStr, Param.ParameterValue);
        }
        OutText += TEXT("\n");
    }

    // ----------------------------------------------------------
    // Vector Parameters
    // ----------------------------------------------------------
    if (Instance->VectorParameterValues.Num() > 0)
    {
        OutText += TEXT("### Vector Parameters\n\n");
        for (const FVectorParameterValue& Param : Instance->VectorParameterValues)
        {
            FString NameStr = Param.ParameterInfo.Name.IsNone() ? TEXT("Unnamed") : Param.ParameterInfo.Name.ToString();
            const FLinearColor& V = Param.ParameterValue;
            OutText += FString::Printf(TEXT("- %s: (%.6g, %.6g, %.6g, %.6g)\n"), 
                                       *NameStr, V.R, V.G, V.B, V.A);
        }
        OutText += TEXT("\n");
    }

    // ----------------------------------------------------------
    // Texture Parameters
    // ----------------------------------------------------------
    if (Instance->TextureParameterValues.Num() > 0)
    {
        OutText += TEXT("### Texture Parameters\n\n");
        for (const FTextureParameterValue& Param : Instance->TextureParameterValues)
        {
            FString NameStr = Param.ParameterInfo.Name.IsNone() ? TEXT("Unnamed") : Param.ParameterInfo.Name.ToString();
            FString TexName = Param.ParameterValue ? Param.ParameterValue->GetName() : TEXT("None");
            OutText += FString::Printf(TEXT("- %s: %s\n"), *NameStr, *TexName);
        }
        OutText += TEXT("\n");
    }

    // Если вообще нет переопределенных параметров
    if (Instance->ScalarParameterValues.Num() == 0 &&
        Instance->VectorParameterValues.Num() == 0 &&
        Instance->TextureParameterValues.Num() == 0)
    {
        OutText += TEXT("_No parameter overrides found._\n\n");
    }
#endif // WITH_EDITOR
}



// ----------------------------------------------------------
// Логирование
// ----------------------------------------------------------

void BPR_Extractor_MaterialFunction::LogMessage(const FString& Msg)
{
    UE_LOG(LogTemp, Log, TEXT("[BPRBPR_Extractor_MaterialFunction] %s"), *Msg);
}

void BPR_Extractor_MaterialFunction::LogWarning(const FString& Msg)
{
    UE_LOG(LogTemp, Warning, TEXT("[BPRBPR_Extractor_MaterialFunction] %s"), *Msg);
}

void BPR_Extractor_MaterialFunction::LogError(const FString& Msg)
{
    UE_LOG(LogTemp, Error, TEXT("[BPR_Extractor_MaterialFunction] %s"), *Msg);
}
