// Copyright (c) 2026 Racoon Coder. All rights reserved.

#include "MCP/BPR_MCPTools.h"

#include "Core/BPR_Compat.h"

#if BPR_HAS_MCP

#include "BlueprintReader.h"                 // FBlueprintReaderModule + GetCoreInstance()
#include "Core/BPR_Core.h"
#include "Core/BPR_Types.h"
#include "Export/BPR_Exporter.h"

#include "IModelContextProtocolModule.h"
#include "IModelContextProtocolTool.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/Blueprint.h"
#include "WidgetBlueprint.h"
#include "Materials/Material.h"
#include "Materials/MaterialFunction.h"
#include "StructUtils/UserDefinedStruct.h"
#include "Engine/UserDefinedEnum.h"

#include "Blueprint/BlueprintSupport.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/Interface.h"
#include "UObject/SoftObjectPath.h"

#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"

#include "Async/Async.h"
#include "CoreGlobals.h"

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
namespace
{
	/** Resolves the first native parent class from an asset's registry tags. */
	UClass* ResolveNativeParentClass(const FAssetData& Asset)
	{
		const FString Path = Asset.GetTagValueRef<FString>(FBlueprintTags::NativeParentClassPath);
		if (Path.IsEmpty() || Path == TEXT("None"))
		{
			return nullptr;
		}
		const FSoftClassPath SoftPath(Path);
		return SoftPath.TryLoadClass<UObject>();
	}

	/**
	 * Maps an asset to its BlueprintReader type string. Distinct asset classes
	 * (Widget/Material/...) are resolved by class name; everything else is a
	 * Blueprint and is refined by its native parent class.
	 */
	FString AssetToTypeString(const FAssetData& Asset)
	{
		const FName ClassName = Asset.AssetClassPath.GetAssetName();

		if (ClassName == UWidgetBlueprint::StaticClass()->GetFName())   return TEXT("Widget");
		if (ClassName == UMaterial::StaticClass()->GetFName())          return TEXT("Material");
		if (ClassName == UMaterialFunction::StaticClass()->GetFName())  return TEXT("MaterialFunction");
		if (ClassName == UUserDefinedStruct::StaticClass()->GetFName()) return TEXT("Structure");
		if (ClassName == UUserDefinedEnum::StaticClass()->GetFName())   return TEXT("Enum");

		if (UClass* NativeParent = ResolveNativeParentClass(Asset))
		{
			if (NativeParent->IsChildOf(UInterface::StaticClass()))
			{
				return TEXT("Interface");
			}
			if (NativeParent->IsChildOf(UActorComponent::StaticClass()))
			{
				return TEXT("ActorComponent");
			}
			if (NativeParent->IsChildOf(AActor::StaticClass()))
			{
				return TEXT("Actor");
			}
		}
		return TEXT("Blueprint");
	}

	BPR_Core* GetCore()
	{
		FBlueprintReaderModule& Mod =
			FModuleManager::LoadModuleChecked<FBlueprintReaderModule>(TEXT("BlueprintReader"));
		return Mod.GetCoreInstance();
	}

	/** The supported asset classes, shared by SearchAssets and asset resolution. */
	TArray<FTopLevelAssetPath> GetSupportedClassPaths()
	{
		return {
			UBlueprint::StaticClass()->GetClassPathName(),
			UWidgetBlueprint::StaticClass()->GetClassPathName(),
			UMaterial::StaticClass()->GetClassPathName(),
			UMaterialFunction::StaticClass()->GetClassPathName(),
			UUserDefinedStruct::StaticClass()->GetClassPathName(),
			UUserDefinedEnum::StaticClass()->GetClassPathName(),
		};
	}

	/**
	 * Resolves an agent-supplied asset reference. A full object path (contains '/')
	 * is used as-is; otherwise the value is treated as a short asset name and looked
	 * up in the AssetRegistry. Must match exactly one supported asset.
	 */
	bool ResolveAssetPath(const FString& InPath, FString& OutPath, FString& OutError)
	{
		if (InPath.Contains(TEXT("/")))
		{
			OutPath = InPath;
			return true;
		}

		IAssetRegistry& AssetRegistry =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

		TArray<FString> Matches;
		TSet<FString> Seen;
		for (const FTopLevelAssetPath& ClassPath : GetSupportedClassPaths())
		{
			TArray<FAssetData> ClassAssets;
			AssetRegistry.GetAssetsByClass(ClassPath, ClassAssets, /*bSearchSubClasses=*/true);
			for (const FAssetData& Asset : ClassAssets)
			{
				if (Asset.AssetName.ToString() != InPath)
				{
					continue;
				}
				const FString Path = Asset.GetObjectPathString();
				if (Seen.Contains(Path))
				{
					continue;
				}
				Seen.Add(Path);
				Matches.Add(Path);
			}
		}

		if (Matches.Num() == 1)
		{
			OutPath = Matches[0];
			return true;
		}

		if (Matches.Num() == 0)
		{
			OutError = FString::Printf(
				TEXT("Error: no asset named '%s'. Use search_blueprint_reader_assets to resolve a valid name or full path."),
				*InPath);
			return false;
		}

		OutError = FString::Printf(
			TEXT("Error: '%s' is ambiguous (%d matches). Pass a full path, one of:\n%s"),
			*InPath, Matches.Num(), *FString::Join(Matches, TEXT("\n")));
		return false;
	}

	bool ExtractAsset(const FString& AssetPath, FBPR_ExtractedData& OutData, FString& OutError)
	{
		FString ResolvedPath;
		if (!ResolveAssetPath(AssetPath, ResolvedPath, OutError))
		{
			return false;
		}

		UObject* Asset = LoadObject<UObject>(nullptr, *ResolvedPath);
		if (!Asset)
		{
			OutError = FString::Printf(
				TEXT("Error: asset not found at '%s'. Use search_blueprint_reader_assets to resolve a valid path."),
				*ResolvedPath);
			return false;
		}

		BPR_Core* Core = GetCore();
		if (!Core)
		{
			OutError = TEXT("Error: BlueprintReader Core is not initialized.");
			return false;
		}

		Core->ExtractAsset(Asset, OutData);
		return true;
	}

	//--------------------------------------------------------------------------
	// MCP tool wrapper (one IModelContextProtocolTool instance per tool)
	//--------------------------------------------------------------------------
	struct FBPR_MCPTool : IModelContextProtocolTool
	{
		FString Name;
		FString Description;
		TSharedPtr<FJsonObject> InputSchema;
		TFunction<FString(const TSharedPtr<FJsonObject>&)> Handler;

		virtual FString GetName() const override { return Name; }
		virtual FString GetDescription() const override { return Description; }
		virtual TSharedPtr<FJsonObject> GetInputJsonSchema() const override { return InputSchema; }

		virtual FModelContextProtocolToolResult Run(const TSharedPtr<FJsonObject>& Params) override
		{
			const FString Text = Handler ? Handler(Params) : TEXT("no handler");
			return UE::ModelContextProtocol::MakeTextResult(Text);
		}

		// The MCP server dispatches RunAsync (not Run) from its HTTP worker thread.
		// LoadObject / AssetRegistry / BPR_Core::ExtractAsset are game-thread only, so hop
		// to the game thread before executing the handler. Handler is captured by value, so
		// the task stays valid even if this tool object is destroyed before the task runs.
		virtual void RunAsync(const FModelContextProtocolToolRequestId& RequestId, const TSharedPtr<FJsonObject>& Params, const FResultCallback& OnComplete) override
		{
			auto Execute = [Handler = Handler, Params]()
			{
				const FString Text = Handler ? Handler(Params) : TEXT("no handler");
				return UE::ModelContextProtocol::MakeTextResult(Text);
			};

			if (IsInGameThread())
			{
				OnComplete(Execute());
				return;
			}

			AsyncTask(ENamedThreads::GameThread, [Execute = MoveTemp(Execute), OnComplete]()
			{
				OnComplete(Execute());
			});
		}
	};

	TSharedPtr<FJsonObject> MakeInputSchema(
		const TArray<TPair<FString, FString>>& Properties,
		const TArray<FString>& Required = {})
	{
		TSharedPtr<FJsonObject> Schema = MakeShared<FJsonObject>();
		Schema->SetStringField(TEXT("type"), TEXT("object"));

		TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
		for (const TPair<FString, FString>& P : Properties)
		{
			TSharedPtr<FJsonObject> PO = MakeShared<FJsonObject>();
			PO->SetStringField(TEXT("type"), P.Value);
			Props->SetObjectField(P.Key, PO);
		}
		Schema->SetObjectField(TEXT("properties"), Props);

		if (Required.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> Req;
			for (const FString& R : Required)
			{
				Req.Add(MakeShared<FJsonValueString>(R));
			}
			Schema->SetArrayField(TEXT("required"), Req);
		}
		return Schema;
	}

	FString JsonGetString(const TSharedPtr<FJsonObject>& Params, const TCHAR* Key, const FString& Default = TEXT(""))
	{
		if (Params.IsValid())
		{
			FString Value;
			if (Params->TryGetStringField(Key, Value))
			{
				return Value;
			}
		}
		return Default;
	}

	int32 JsonGetInt(const TSharedPtr<FJsonObject>& Params, const TCHAR* Key, int32 Default = 0)
	{
		if (Params.IsValid())
		{
			int32 Value;
			if (Params->TryGetNumberField(Key, Value))
			{
				return Value;
			}
		}
		return Default;
	}

	TArray<TSharedRef<IModelContextProtocolTool>> GRegisteredTools;

	//--------------------------------------------------------------------------
	// Tool implementations (return text; structured output lands in M9)
	//--------------------------------------------------------------------------
	FString Impl_ListSupportedTypes(const TSharedPtr<FJsonObject>&)
	{
		const TArray<FString> Types = {
			TEXT("Blueprint"), TEXT("Actor"), TEXT("ActorComponent"), TEXT("Widget"),
			TEXT("Material"), TEXT("MaterialFunction"), TEXT("Enum"),
			TEXT("Structure"), TEXT("Interface")
		};
		return FString::Join(Types, TEXT(", "));
	}

	FString Impl_SearchAssets(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Query = JsonGetString(Params, TEXT("query"));
		const FString TypeFilter = JsonGetString(Params, TEXT("type_filter"));
		const int32 Limit = JsonGetInt(Params, TEXT("limit"), 20);

		IAssetRegistry& AssetRegistry =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

		const TArray<FTopLevelAssetPath> ClassPaths = GetSupportedClassPaths();

		const FString TypeFilterLower = TypeFilter.ToLower();
		TArray<FString> Lines;
		TSet<FString> Seen;

		for (const FTopLevelAssetPath& ClassPath : ClassPaths)
		{
			TArray<FAssetData> ClassAssets;
			AssetRegistry.GetAssetsByClass(ClassPath, ClassAssets, /*bSearchSubClasses=*/true);
			for (const FAssetData& Asset : ClassAssets)
			{
				const FString Name = Asset.AssetName.ToString();
				const FString Path = Asset.GetObjectPathString();
				if (!Query.IsEmpty()
					&& !Name.Contains(Query, ESearchCase::IgnoreCase)
					&& !Path.Contains(Query, ESearchCase::IgnoreCase))
				{
					continue;
				}
				const FString Type = AssetToTypeString(Asset);
				if (!TypeFilterLower.IsEmpty() && Type.ToLower() != TypeFilterLower)
				{
					continue;
				}
				if (Seen.Contains(Path))
				{
					continue;
				}
				Seen.Add(Path);
				Lines.Add(FString::Printf(TEXT("%s | %s | %s"), *Name, *Type, *Path));
				if (Limit > 0 && Lines.Num() >= Limit)
				{
					break;
				}
			}
			if (Limit > 0 && Lines.Num() >= Limit)
			{
				break;
			}
		}

		return Lines.Num() > 0
			? FString::Printf(TEXT("%d asset(s):\n"), Lines.Num()) + FString::Join(Lines, TEXT("\n"))
			: TEXT("No assets found.");
	}

	FString Impl_ReadAsset(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Path = JsonGetString(Params, TEXT("asset_path"));
		if (Path.IsEmpty())
		{
			return TEXT("Error: 'asset_path' is required.");
		}

		FBPR_ExtractedData Data;
		FString Error;
		if (!ExtractAsset(Path, Data, Error))
		{
			return Error;
		}

		if (Data.AssetType == EAssetType::Unknown)
		{
			return FString::Printf(TEXT("Error: asset '%s' is not a supported BlueprintReader type."), *Path);
		}

		const EOutputFormat Format = BPR_Exporter::ParseOutputFormat(JsonGetString(Params, TEXT("format")));
		const FString Markdown = BPR_Exporter::BuildMarkdown(Data, Format);
		if (Markdown.IsEmpty())
		{
			return FString::Printf(TEXT("Error: no extractable data for asset '%s'."), *Path);
		}
		return Markdown;
	}

	FString Impl_ReadAssetSection(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Path = JsonGetString(Params, TEXT("asset_path"));
		const FString Section = JsonGetString(Params, TEXT("section"));
		if (Path.IsEmpty())
		{
			return TEXT("Error: 'asset_path' is required.");
		}

		FBPR_ExtractedData Data;
		FString Error;
		if (!ExtractAsset(Path, Data, Error))
		{
			return Error;
		}

		if (Data.AssetType == EAssetType::Unknown)
		{
			return FString::Printf(TEXT("Error: asset '%s' is not a supported BlueprintReader type."), *Path);
		}

		const FString SectionLower = Section.ToLower();
		if (SectionLower == TEXT("structure"))
		{
			Data.Graph = FText::GetEmpty();
			Data.Design = FText::GetEmpty();
		}
		else if (SectionLower == TEXT("graph"))
		{
			Data.Structure = FText::GetEmpty();
			Data.Design = FText::GetEmpty();
		}
		else if (SectionLower == TEXT("design"))
		{
			Data.Structure = FText::GetEmpty();
			Data.Graph = FText::GetEmpty();
		}
		else
		{
			return FString::Printf(TEXT("Error: unknown section '%s'. Use one of: structure, graph, design."), *Section);
		}

		const EOutputFormat Format = BPR_Exporter::ParseOutputFormat(JsonGetString(Params, TEXT("format")));
		const FString Markdown = BPR_Exporter::BuildMarkdown(Data, Format);
		if (Markdown.IsEmpty())
		{
			return FString::Printf(TEXT("Error: section '%s' is empty or not present for asset '%s'."), *Section, *Path);
		}
		return Markdown;
	}

	FString Impl_ExportAsset(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Path = JsonGetString(Params, TEXT("asset_path"));
		const FString OutputDir = JsonGetString(Params, TEXT("output_dir"));
		if (Path.IsEmpty())
		{
			return TEXT("Error: 'asset_path' is required.");
		}

		FBPR_ExtractedData Data;
		FString Error;
		if (!ExtractAsset(Path, Data, Error))
		{
			return Error;
		}

		if (Data.AssetType == EAssetType::Unknown)
		{
			return FString::Printf(TEXT("Error: asset '%s' is not a supported BlueprintReader type."), *Path);
		}

		FString Dir = OutputDir.IsEmpty()
			? FPaths::Combine(FPaths::ProjectDir(), TEXT("BPR-TEMP"))
			: OutputDir;
		IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);

		const EOutputFormat Format = BPR_Exporter::ParseOutputFormat(JsonGetString(Params, TEXT("format")));
		if (BPR_Exporter::BuildMarkdown(Data, Format).IsEmpty())
		{
			return FString::Printf(TEXT("Error: no extractable data for asset '%s'."), *Path);
		}

		const FString FullPath = FPaths::Combine(Dir, Data.AssetName + TEXT(".md"));
		const FString Written = BPR_Exporter::ExportToFile(Data, FullPath, Format);
		if (!Written.IsEmpty())
		{
			return Written;
		}
		return FString::Printf(TEXT("Error: failed to write '%s'"), *FullPath);
	}
}

#endif // BPR_HAS_MCP

//------------------------------------------------------------------------------
// Registration (defined on all engine versions; bodies are MCP-only)
//------------------------------------------------------------------------------
void RegisterBlueprintReaderMCPTools()
{
#if BPR_HAS_MCP
	IModelContextProtocolModule* MCP = IModelContextProtocolModule::Get();
	if (!MCP)
	{
		return; // Unreal MCP plugin not enabled — nothing to register.
	}

	auto Add = [&MCP](const FString& Name, const FString& Description,
		TSharedPtr<FJsonObject> Schema, TFunction<FString(const TSharedPtr<FJsonObject>&)> Handler)
	{
		TSharedRef<FBPR_MCPTool> Tool = MakeShared<FBPR_MCPTool>();
		Tool->Name = Name;
		Tool->Description = Description;
		Tool->InputSchema = Schema;
		Tool->Handler = Handler;
		if (MCP->AddTool(Tool))
		{
			GRegisteredTools.Add(Tool);
		}
	};

	Add(TEXT("list_blueprint_reader_types"),
		TEXT("Lists the asset types BlueprintReader can extract."),
		MakeInputSchema({}), &Impl_ListSupportedTypes);

	Add(TEXT("search_blueprint_reader_assets"),
		TEXT("Searches supported assets by name or path substring (case-insensitive); optional type filter. Returns 'Name | Type | Path' lines."),
		MakeInputSchema({ {TEXT("query"), TEXT("string")}, {TEXT("type_filter"), TEXT("string")}, {TEXT("limit"), TEXT("integer")} },
			{ TEXT("query") }),
		&Impl_SearchAssets);

	Add(TEXT("read_blueprint_reader_asset"),
		TEXT("Extracts a full BlueprintReader dump (Structure + Graph + Design) of the asset at asset_path, as Markdown. Optional 'format': human_readable | compact | minimal (default compact)."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")}, {TEXT("format"), TEXT("string")} }, { TEXT("asset_path") }),
		&Impl_ReadAsset);

	Add(TEXT("read_blueprint_reader_asset_section"),
		TEXT("Extracts a single section (structure | graph | design) of the asset at asset_path — cheaper on tokens. Optional 'format': human_readable | compact | minimal (default compact)."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")}, {TEXT("section"), TEXT("string")}, {TEXT("format"), TEXT("string")} },
			{ TEXT("asset_path"), TEXT("section") }),
		&Impl_ReadAssetSection);

	Add(TEXT("export_blueprint_reader_asset"),
		TEXT("Exports the asset at asset_path to a .md file and returns the written file path. Optional 'output_dir' and 'format' (human_readable | compact | minimal, default compact)."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")}, {TEXT("output_dir"), TEXT("string")}, {TEXT("format"), TEXT("string")} },
			{ TEXT("asset_path") }),
		&Impl_ExportAsset);
#endif
}

void UnregisterBlueprintReaderMCPTools()
{
#if BPR_HAS_MCP
	if (IModelContextProtocolModule* MCP = IModelContextProtocolModule::Get())
	{
		for (const TSharedRef<IModelContextProtocolTool>& Tool : GRegisteredTools)
		{
			MCP->RemoveTool(Tool);
		}
	}
	GRegisteredTools.Empty();
#endif
}
