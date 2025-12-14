#include "Extractors/BPR_Extractor_Material.h"
#include "Core/BPR_Core.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
#include "MaterialExpressionIO.h"
#include "Materials/MaterialExpressionReroute.h"
#include "Materials/MaterialExpressionNamedReroute.h"
#include "Materials/MaterialExpressionStaticSwitchParameter.h" // для StaticSwitch
#include "UObject/UnrealType.h" // FProperty, FStructProperty

BPR_Extractor_Material::BPR_Extractor_Material() {}
BPR_Extractor_Material::~BPR_Extractor_Material() {}

void BPR_Extractor_Material::ProcessMaterial(UObject* SelectedObject, FBPR_ExtractedData& OutData)
{
	if (!SelectedObject)
	{
		LogError(TEXT("SelectedObject is null!"));
		OutData.Structure = FText::FromString("Error: SelectedObject is null.");
		OutData.Graph = FText::FromString("Error: SelectedObject is null.");
		return;
	}

	UMaterial* Material = Cast<UMaterial>(SelectedObject);
	UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(SelectedObject);

	if (!Material && !MaterialInstance)
	{
		LogWarning(TEXT("Selected object is not a Material or MaterialInstance."));
		OutData.Structure = FText::FromString("Warning: Selected object is not a Material or MaterialInstance.");
		OutData.Graph = FText::FromString("Warning: Selected object is not a Material or MaterialInstance.");
		return;
	}

	FString TmpStructure;
	FString TmpGraph;

	// Для обычного материала
	if (Material)
	{
		AppendMaterialInfo(Material, TmpStructure);
		AppendMaterialParameters(Material, TmpStructure);
		AppendMaterialGraph(Material, TmpGraph);
	}

	// Для материала-инстанса
	if (MaterialInstance)
	{
		AppendMaterialInstanceInfo(MaterialInstance, TmpStructure);
		AppendMaterialInstanceOverrides(MaterialInstance, TmpStructure);

		UMaterial* ParentMaterial = MaterialInstance->GetMaterial();
		if (ParentMaterial)
		{
			AppendMaterialGraph(ParentMaterial, TmpGraph);
		}
	}

	OutData.Structure = FText::FromString(TmpStructure);
	OutData.Graph = FText::FromString(TmpGraph);
}


//==============================================================================
// Информация о материале
//==============================================================================
void BPR_Extractor_Material::AppendMaterialInfo(UMaterial* Material, FString& OutText)
{
	if (!Material) return;

	OutText += FString::Printf(TEXT("Material: %s\n"), *CleanName(Material->GetName()));
	OutText += FString::Printf(TEXT("Domain: %s\n"), *UEnum::GetValueAsString(Material->MaterialDomain));
	OutText += FString::Printf(TEXT("BlendMode: %s\n"), *UEnum::GetValueAsString(Material->BlendMode));
	OutText += FString::Printf(TEXT("ShadingModel: %s\n"), *UEnum::GetValueAsString(Material->GetShadingModels().GetFirstShadingModel()));
	OutText += FString::Printf(TEXT("TwoSided: %s\n"), Material->TwoSided ? TEXT("True") : TEXT("False"));
	OutText += FString::Printf(TEXT("DitheredLODTransition: %s\n"), Material->DitheredLODTransition ? TEXT("True") : TEXT("False"));
	OutText += TEXT("\n");
}

//==============================================================================
// Параметры материала
//==============================================================================
void BPR_Extractor_Material::AppendMaterialParameters(UMaterial* Material, FString& OutText)
{
	if (!Material) return;

	OutText += TEXT("## Material Parameters\n");

	// -----------------
	// Scalar
	// -----------------
	TArray<FMaterialParameterInfo> ScalarInfos;
	TArray<FGuid> ScalarIds;
	Material->GetAllScalarParameterInfo(ScalarInfos, ScalarIds);

	for (const FMaterialParameterInfo& Info : ScalarInfos)
	{
		float Value = 0.f;
		Material->GetScalarParameterValue(Info, Value);

		OutText += FString::Printf(TEXT("Scalar: %s = %.3f\n"),
			*Info.Name.ToString(), Value);
	}

	// -----------------
	// Vector
	// -----------------
	TArray<FMaterialParameterInfo> VectorInfos;
	TArray<FGuid> VectorIds;
	Material->GetAllVectorParameterInfo(VectorInfos, VectorIds);

	for (const FMaterialParameterInfo& Info : VectorInfos)
	{
		FLinearColor Value;
		Material->GetVectorParameterValue(Info, Value);

		OutText += FString::Printf(TEXT("Vector: %s = (%f,%f,%f,%f)\n"),
			*Info.Name.ToString(),
			Value.R, Value.G, Value.B, Value.A);
	}

	// -----------------
	// Texture
	// -----------------
	TArray<FMaterialParameterInfo> TextureInfos;
	TArray<FGuid> TextureIds;
	Material->GetAllTextureParameterInfo(TextureInfos, TextureIds);

	for (const FMaterialParameterInfo& Info : TextureInfos)
	{
		UTexture* Tex = nullptr;
		Material->GetTextureParameterValue(Info, Tex);

		FString TexName = Tex ? CleanName(Tex->GetName()) : TEXT("None");
		OutText += FString::Printf(TEXT("Texture: %s = %s\n"),
			*Info.Name.ToString(), *TexName);
	}

	// -----------------
	// StaticSwitch
	// -----------------
	TArray<const UMaterialExpressionStaticSwitchParameter*> Switches;
	Material->GetAllExpressionsOfType(Switches);

	for (const UMaterialExpressionStaticSwitchParameter* Switch : Switches)
	{
		OutText += FString::Printf(TEXT("StaticSwitch: %s = %s\n"),
			*Switch->ParameterName.ToString(),
			Switch->DefaultValue ? TEXT("True") : TEXT("False"));
	}


	OutText += TEXT("\n");
}


//==============================================================================
// Обход графа материала
//==============================================================================
void BPR_Extractor_Material::AppendMaterialGraph(UMaterial* Material, FString& OutText)
{
	if (!Material) return;

	OutText += TEXT("## Material Graph\n");

	struct FMatOutput
	{
		FString Name;
		EMaterialProperty Property;
	};
	TArray<FMatOutput> Outputs = {
		{TEXT("BaseColor"), MP_BaseColor},
		{TEXT("Metallic"), MP_Metallic},
		{TEXT("Specular"), MP_Specular},
		{TEXT("Roughness"), MP_Roughness},
		{TEXT("EmissiveColor"), MP_EmissiveColor},
		{TEXT("Opacity"), MP_Opacity},
		{TEXT("OpacityMask"), MP_OpacityMask},
		{TEXT("Normal"), MP_Normal},
		{TEXT("WorldPositionOffset"), MP_WorldPositionOffset}
	};

	// DAG структуры
	TMap<UMaterialExpression*, int32> NodeIds;
	TMap<int32, FString> NodeTexts;
	int32 NextId = 1;

	// Фаза 1: Outputs
	for (const FMatOutput& Out : Outputs)
	{
		TArray<UMaterialExpression*> RootExpressions;
		Material->GetExpressionsInPropertyChain(Out.Property, RootExpressions, nullptr); 

		if (RootExpressions.Num() == 0)
		{
			OutText += FString::Printf(TEXT("%s → None\n"), *Out.Name);
			continue;
		}

		for (UMaterialExpression* RootExpr : RootExpressions)
		{
			AppendMaterialOutput(Out.Name, RootExpr, OutText, NodeIds, NodeTexts, NextId);
		}
	}

	// Фаза 2: Nodes
	OutText += TEXT("\n## Nodes\n");
	for (auto& Pair : NodeTexts)
	{
		OutText += Pair.Value;
	}
}




//==============================================================================
// Информация о MaterialInstance
//==============================================================================
void BPR_Extractor_Material::AppendMaterialInstanceInfo(UMaterialInstance* Instance, FString& OutText)
{
	if (!Instance) return;

	OutText += FString::Printf(TEXT("MaterialInstance: %s\n"), *CleanName(Instance->GetName()));
	UMaterial* Parent = Instance->GetMaterial();
	if (Parent)
	{
		OutText += FString::Printf(TEXT("Parent Material: %s\n"), *CleanName(Parent->GetName()));
	}
	OutText += TEXT("\n");
}


//==============================================================================
// Рекурсивный обход Expression
//==============================================================================
void BPR_Extractor_Material::ProcessExpressionDAG(
	UMaterialExpression* Expression,
	TMap<UMaterialExpression*, int32>& NodeIds,
	TMap<int32, FString>& NodeTexts,
	int32& NextId)
{
	if (!Expression)
		return;

	// Если нода уже обработана, ничего не делаем
	if (NodeIds.Contains(Expression))
		return;

	// Присваиваем уникальный ID
	int32 NodeId = NextId++;
	NodeIds.Add(Expression, NodeId);

	// Читаемое имя ноды
	FString NodeDisplayName = GetReadableNodeName(Expression, NodeId);

	// Строим текст ноды
	FString NodeText = FString::Printf(TEXT("Node: %s\n"), *NodeDisplayName);

	int32 NumInputs = Expression->CountInputs();
	for (int32 i = 0; i < NumInputs; ++i)
	{
		FExpressionInput* Input = Expression->GetInput(i);
		if (!Input) continue;

		FString InputName = Expression->GetInputName(i).ToString();

		if (Input->Expression)
		{
			UMaterialExpression* ChildExpr = Input->Expression;

			// Рекурсивно обрабатываем child
			ProcessExpressionDAG(ChildExpr, NodeIds, NodeTexts, NextId);

			// Добавляем ссылку на child через читаемое имя
			int32 ChildId = NodeIds[ChildExpr];
			FString ChildDisplayName = GetReadableNodeName(ChildExpr, ChildId);
			NodeText += FString::Printf(TEXT("  Input: %s -> %s\n"), *InputName, *ChildDisplayName);
		}
		else
		{
			NodeText += FString::Printf(TEXT("  Input: %s = Unconnected\n"), *InputName);
		}
	}

	// Сохраняем текст ноды один раз
	NodeTexts.Add(NodeId, NodeText);
}





//==============================================================================
// Обход Material Output
//==============================================================================
void BPR_Extractor_Material::AppendMaterialOutput(
	const FString& OutputName,
	UMaterialExpression* RootExpression,
	FString& OutText,
	TMap<UMaterialExpression*, int32>& NodeIds,
	TMap<int32, FString>& NodeTexts,
	int32& NextId)
{
	if (!RootExpression)
	{
		OutText += FString::Printf(TEXT("%s → None\n"), *OutputName);
		return;
	}

	// Получаем или присваиваем ID для RootExpression и всех вложенных нод
	if (!NodeIds.Contains(RootExpression))
	{
		ProcessExpressionDAG(RootExpression, NodeIds, NodeTexts, NextId);
	}

	// Сбор всех нод через новый хелпер
	TSet<UMaterialExpression*> Visited;
	TArray<FString> NodeNames;
	CollectNodesRecursive(RootExpression, Visited, NodeNames, NodeIds);

	// Соединяем через запятую и выводим в строку
	FString JoinedNodes = FString::Join(NodeNames, TEXT(", "));
	OutText += FString::Printf(TEXT("%s = %s\n"), *OutputName, *JoinedNodes);
}

void BPR_Extractor_Material::AppendMaterialInstanceOverrides(UMaterialInstance* Instance, FString& OutText)
{
    if (!Instance) return;

    OutText += TEXT("## MaterialInstance Parameters\n");

    UMaterial* Parent = Instance->GetMaterial();
    if (!Parent)
    {
        OutText += TEXT("No parent material.\n\n");
        return;
    }

    auto AppendScalar = [&](const FMaterialParameterInfo& Info)
    {
        float ParentValue = 0.f;
        Parent->GetScalarParameterValue(Info, ParentValue);

        float InstValue = 0.f;
        Instance->GetScalarParameterValue(Info.Name, InstValue, false);

        bool bOverridden = !FMath::IsNearlyEqual(ParentValue, InstValue);
        OutText += FString::Printf(TEXT("Scalar: %s = %.3f (%s)\n"),
            *Info.Name.ToString(),
            InstValue,
            bOverridden ? TEXT("Overridden") : TEXT("Default"));
    };

    auto AppendVector = [&](const FMaterialParameterInfo& Info)
    {
        FLinearColor ParentValue;
        Parent->GetVectorParameterValue(Info, ParentValue);

        FLinearColor InstValue;
        Instance->GetVectorParameterValue(Info.Name, InstValue, false);

        bool bOverridden = !ParentValue.Equals(InstValue);
        OutText += FString::Printf(TEXT("Vector: %s = (%f,%f,%f,%f) (%s)\n"),
            *Info.Name.ToString(),
            InstValue.R, InstValue.G, InstValue.B, InstValue.A,
            bOverridden ? TEXT("Overridden") : TEXT("Default"));
    };

    auto AppendTexture = [&](const FMaterialParameterInfo& Info)
    {
        UTexture* ParentTex = nullptr;
        Parent->GetTextureParameterValue(Info, ParentTex);

        UTexture* InstTex = nullptr;
        Instance->GetTextureParameterValue(Info.Name, InstTex, false);

        bool bOverridden = ParentTex != InstTex;
        FString InstName = InstTex ? CleanName(InstTex->GetName()) : TEXT("None");
        FString ParentName = ParentTex ? CleanName(ParentTex->GetName()) : TEXT("None");

        OutText += FString::Printf(TEXT("Texture: %s = %s (%s, Parent=%s)\n"),
            *Info.Name.ToString(),
            *InstName,
            bOverridden ? TEXT("Overridden") : TEXT("Default"),
            *ParentName);
    };

    // -----------------
    // Scalar
    TArray<FMaterialParameterInfo> ScalarInfos;
    TArray<FGuid> ScalarIds;
    Parent->GetAllScalarParameterInfo(ScalarInfos, ScalarIds);
    for (const auto& Info : ScalarInfos) AppendScalar(Info);

    // -----------------
    // Vector
    TArray<FMaterialParameterInfo> VectorInfos;
    TArray<FGuid> VectorIds;
    Parent->GetAllVectorParameterInfo(VectorInfos, VectorIds);
    for (const auto& Info : VectorInfos) AppendVector(Info);

    // -----------------
    // Texture
    TArray<FMaterialParameterInfo> TextureInfos;
    TArray<FGuid> TextureIds;
    Parent->GetAllTextureParameterInfo(TextureInfos, TextureIds);
    for (const auto& Info : TextureInfos) AppendTexture(Info);

    // -----------------
    // StaticSwitch
    FStaticParameterSet StaticParams;
    Instance->GetStaticParameterValues(StaticParams);
    for (const FStaticSwitchParameter& Param : StaticParams.StaticSwitchParameters)
    {
        OutText += FString::Printf(TEXT("StaticSwitch: %s = %s (%s)\n"),
            *Param.ParameterInfo.Name.ToString(),
            Param.Value ? TEXT("True") : TEXT("False"),
            Param.bOverride ? TEXT("Overridden") : TEXT("Default"));
    }

    OutText += TEXT("\n");
}





// Хелперы
FString BPR_Extractor_Material::CleanName(const FString& RawName)
{
	FString Result = RawName;

	int32 UnderscoreIndex;
	if (RawName.FindLastChar('_', UnderscoreIndex))
	{
		FString Tail = RawName.Mid(UnderscoreIndex + 1);

		// Material часто генерит *_GUID или *_NUMBER
		if (Tail.Len() >= 8 && Tail.IsNumeric())
		{
			Result = RawName.Left(UnderscoreIndex);
		}
	}

	return Result;
}

bool BPR_Extractor_Material::HasAnyInputs(UMaterialExpression* Expression)
{
	if (!Expression) return false;

	// Пока считаем, что если это не Constant —
	// у него потенциально есть входы
	return !Expression->IsA<UMaterialExpressionConstant>()
		&& !Expression->IsA<UMaterialExpressionScalarParameter>()
		&& !Expression->IsA<UMaterialExpressionVectorParameter>();
}


FString BPR_Extractor_Material::GetInputValueDescription(const FExpressionInput& Input)
{
	if (Input.Expression)
	{
		return FString::Printf(
			TEXT("<linked: %s>"),
			*GetReadableExpressionName(Input.Expression)
		);
	}

	return TEXT("Unconnected");
}


FString BPR_Extractor_Material::GetReadableExpressionName(UMaterialExpression* Expression)
{
	if (!Expression)
		return TEXT("None");

	// 1. Базовый тип
	FString ClassName = Expression->GetClass()->GetName();
	ClassName.RemoveFromStart(TEXT("MaterialExpression"));

	FString Result = ClassName;

	// 2. Частные случаи (самые важные)
	if (auto* Tex = Cast<UMaterialExpressionTextureSample>(Expression))
	{
		FString TexName = Tex->Texture
			? CleanName(Tex->Texture->GetName())
			: TEXT("None");

		Result = FString::Printf(TEXT("TextureSample(%s)"), *TexName);
	}
	else if (auto* Scalar = Cast<UMaterialExpressionScalarParameter>(Expression))
	{
		Result = FString::Printf(
			TEXT("ScalarParam(%s = %.3f)"),
			*Scalar->ParameterName.ToString(),
			Scalar->DefaultValue
		);
	}
	else if (auto* Vector = Cast<UMaterialExpressionVectorParameter>(Expression))
	{
		Result = FString::Printf(
			TEXT("VectorParam(%s)"),
			*Vector->ParameterName.ToString()
		);
	}
	else if (auto* Const = Cast<UMaterialExpressionConstant>(Expression))
	{
		Result = FString::Printf(
			TEXT("Constant(%.3f)"),
			Const->R
		);
	}

	// 3. Комментарий
	if (!Expression->Desc.IsEmpty())
	{
		Result += FString::Printf(TEXT(" // %s"), *Expression->Desc);
	}

	return Result;
}

FString BPR_Extractor_Material::MakeIndent(int32 Level)
{
	return FString::ChrN(Level * 2, ' '); // 2 пробела на уровень
}

bool BPR_Extractor_Material::IsTransparentExpression(UMaterialExpression* Expression)
{
	if (!Expression) return false;

	// Reroute — всегда
	if (Expression->IsA<UMaterialExpressionReroute>()) return true;
	if (Expression->IsA<UMaterialExpressionNamedRerouteUsage>()) return true;

	// Declaration — особый случай (логически прозрачна)
	if (Expression->IsA<UMaterialExpressionNamedRerouteDeclaration>()) return true;

	return false;
}

UMaterialExpression* BPR_Extractor_Material::ResolveExpression(UMaterialExpression* Expr)
{
	if (!Expr)
	{
		return nullptr;
	}

	TSet<UMaterialExpression*> Visited;
	UMaterialExpression* Current = Expr;

	while (Current && IsTransparentExpression(Current))
	{
		// защита от циклов
		if (Visited.Contains(Current))
		{
			LogWarning(FString::Printf(
				TEXT("ResolveExpression: loop detected at %s"),
				*Current->GetName()
			));
			return nullptr;
		}

		Visited.Add(Current);

		// Получаем входы expression (разыменованные)
		TArray<UMaterialExpression*> InputExpressions;
		Current->GetAllInputExpressions(InputExpressions);

		if (InputExpressions.Num() == 0)
		{
			// прозрачная нода без входов
			return nullptr;
		}

		// Для прозрачных нод берём первый вход
		Current = InputExpressions[0];
	}

	return Current;
}

FString BPR_Extractor_Material::GetReadableNodeName(UMaterialExpression* Expr, int32 NodeId)
{
	if (!Expr)
		return FString::Printf(TEXT("UnknownNode_%d"), NodeId);

	// Получаем имя класса
	FString TypeName = Expr->GetClass()->GetName(); // например MaterialExpressionLinearInterpolate

	// Убираем префикс "MaterialExpression", если он есть
	const FString Prefix = TEXT("MaterialExpression");
	if (TypeName.StartsWith(Prefix))
	{
		TypeName = TypeName.RightChop(Prefix.Len());
	}

	return FString::Printf(TEXT("%s_%d"), *TypeName, NodeId);
}

void BPR_Extractor_Material::CollectNodesRecursive(
	UMaterialExpression* Expr,
	TSet<UMaterialExpression*>& Visited,
	TArray<FString>& OutNodeNames,
	TMap<UMaterialExpression*, int32>& NodeIds)
{
	if (!Expr || Visited.Contains(Expr))
		return;

	Visited.Add(Expr);

	int32 NodeId = NodeIds[Expr];
	OutNodeNames.Add(GetReadableNodeName(Expr, NodeId));

	int32 NumInputs = Expr->CountInputs();
	for (int32 i = 0; i < NumInputs; ++i)
	{
		FExpressionInput* Input = Expr->GetInput(i);
		if (Input && Input->Expression)
		{
			CollectNodesRecursive(Input->Expression, Visited, OutNodeNames, NodeIds);
		}
	}
}


// Логгеры
void BPR_Extractor_Material::LogWarning(const FString& Msg)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
}

void BPR_Extractor_Material::LogError(const FString& Msg)
{
	UE_LOG(LogTemp, Error, TEXT("%s"), *Msg);
}

void BPR_Extractor_Material::LogMessage(const FString& Msg)
{
	UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);
}