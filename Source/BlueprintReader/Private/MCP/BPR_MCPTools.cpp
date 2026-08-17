// Copyright (c) 2026 Racoon Coder. All rights reserved.

#include "MCP/BPR_MCPTools.h"

#include "Core/BPR_Compat.h"

#if BPR_HAS_MCP

#include "BlueprintReader.h"                 // FBlueprintReaderModule + GetCoreInstance()
#include "Core/BPR_Core.h"
#include "Core/BPR_Types.h"
#include "MCP/BPR_AgentTypes.h"

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

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"

//------------------------------------------------------------------------------
// Extraction core
//------------------------------------------------------------------------------
namespace
{
	FString AssetTypeToString(EAssetType In)
	{
		switch (In)
		{
		case EAssetType::Blueprint:        return TEXT("Blueprint");
		case EAssetType::Actor:            return TEXT("Actor");
		case EAssetType::Widget:           return TEXT("Widget");
		case EAssetType::Material:         return TEXT("Material");
		case EAssetType::MaterialFunction: return TEXT("MaterialFunction");
		case EAssetType::ActorComponent:   return TEXT("ActorComponent");
		case EAssetType::Enum:             return TEXT("Enum");
		case EAssetType::Structure:        return TEXT("Structure");
		case EAssetType::InterfaceBP:      return TEXT("Interface");
		default:                           return TEXT("Unknown");
		}
	}

	FString ClassToTypeString(const FName& ClassName)
	{
		if (ClassName == UWidgetBlueprint::StaticClass()->GetFName())     return TEXT("Widget");
		if (ClassName == UMaterial::StaticClass()->GetFName())            return TEXT("Material");
		if (ClassName == UMaterialFunction::StaticClass()->GetFName())    return TEXT("MaterialFunction");
		if (ClassName == UUserDefinedStruct::StaticClass()->GetFName())   return TEXT("Structure");
		if (ClassName == UUserDefinedEnum::StaticClass()->GetFName())     return TEXT("Enum");
		return TEXT("Blueprint");
	}

	BPR_Core* GetCore()
	{
		FBlueprintReaderModule& Mod =
			FModuleManager::LoadModuleChecked<FBlueprintReaderModule>(TEXT("BlueprintReader"));
		return Mod.GetCoreInstance();
	}

	FBPR_AssetDump ExtractDump(const FString& AssetPath, const FString& Section)
	{
		FBPR_AssetDump Dump;
		Dump.AssetPath = AssetPath;

		UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
		if (!Asset)
		{
			Dump.Type = TEXT("NotFound");
			Dump.Structure = FString::Printf(
				TEXT("Error: asset not found at '%s'. Use search_blueprint_reader_assets to resolve a valid path."),
				*AssetPath);
			return Dump;
		}

		BPR_Core* Core = GetCore();
		if (!Core)
		{
			Dump.Type = TEXT("Error");
			Dump.Structure = TEXT("Error: BlueprintReader Core is not initialized.");
			return Dump;
		}

		FBPR_ExtractedData Data;
		Core->ExtractAsset(Asset, Data);

		Dump.AssetName = Data.AssetName;
		Dump.Type = AssetTypeToString(Data.AssetType);

		const FString SectionLower = Section.ToLower();
		if (SectionLower == TEXT("graph"))
		{
			Dump.Graph = Data.Graph.ToString();
		}
		else if (SectionLower == TEXT("design"))
		{
			Dump.Design = Data.Design.ToString();
		}
		else if (SectionLower == TEXT("structure"))
		{
			Dump.Structure = Data.Structure.ToString();
		}
		else
		{
			Dump.Structure = Data.Structure.ToString();
			Dump.Graph = Data.Graph.ToString();
			Dump.Design = Data.Design.ToString();
		}

		// M7 placeholder checksum; real content hash arrives in M8.
		Dump.Checksum = AssetPath;

		return Dump;
	}

	FString DumpToMarkdown(const FBPR_AssetDump& Dump)
	{
		if (Dump.Type == TEXT("NotFound") || Dump.Type == TEXT("Error"))
		{
			return Dump.Structure;
		}

		FString Out;
		Out += FString::Printf(TEXT("# %s (%s)\n\n"), *Dump.AssetName, *Dump.Type);
		if (!Dump.Structure.IsEmpty()) { Out += TEXT("## Structure\n\n") + Dump.Structure + TEXT("\n\n"); }
		if (!Dump.Graph.IsEmpty())     { Out += TEXT("## Graph\n\n")     + Dump.Graph     + TEXT("\n\n"); }
		if (!Dump.Design.IsEmpty())    { Out += TEXT("## Design\n\n")    + Dump.Design    + TEXT("\n\n"); }
		return Out;
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

		TArray<FTopLevelAssetPath> ClassPaths = {
			UBlueprint::StaticClass()->GetClassPathName(),
			UWidgetBlueprint::StaticClass()->GetClassPathName(),
			UMaterial::StaticClass()->GetClassPathName(),
			UMaterialFunction::StaticClass()->GetClassPathName(),
			UUserDefinedStruct::StaticClass()->GetClassPathName(),
			UUserDefinedEnum::StaticClass()->GetClassPathName(),
		};

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
				if (!Query.IsEmpty() && !Name.Contains(Query))
				{
					continue;
				}
				const FString Type = ClassToTypeString(Asset.AssetClassPath.GetAssetName());
				if (!TypeFilterLower.IsEmpty() && Type.ToLower() != TypeFilterLower)
				{
					continue;
				}
				const FString Path = Asset.GetObjectPathString();
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
		return DumpToMarkdown(ExtractDump(Path, TEXT("")));
	}

	FString Impl_ReadAssetSection(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Path = JsonGetString(Params, TEXT("asset_path"));
		const FString Section = JsonGetString(Params, TEXT("section"));
		if (Path.IsEmpty())
		{
			return TEXT("Error: 'asset_path' is required.");
		}
		return DumpToMarkdown(ExtractDump(Path, Section));
	}

	FString Impl_ExportAsset(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Path = JsonGetString(Params, TEXT("asset_path"));
		const FString OutputDir = JsonGetString(Params, TEXT("output_dir"));
		if (Path.IsEmpty())
		{
			return TEXT("Error: 'asset_path' is required.");
		}

		FBPR_AssetDump Dump = ExtractDump(Path, TEXT(""));
		if (Dump.Type == TEXT("NotFound") || Dump.Type == TEXT("Error"))
		{
			return Dump.Structure;
		}

		FString Dir = OutputDir.IsEmpty()
			? FPaths::Combine(FPaths::ProjectDir(), TEXT("BPR-TEMP"))
			: OutputDir;
		IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);

		const FString FullPath = FPaths::Combine(Dir, Dump.AssetName + TEXT(".md"));
		if (FFileHelper::SaveStringToFile(DumpToMarkdown(Dump), *FullPath,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			return FullPath;
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
		TEXT("Searches supported assets by name substring; optional type filter. Returns 'Name | Type | Path' lines."),
		MakeInputSchema({ {TEXT("query"), TEXT("string")}, {TEXT("type_filter"), TEXT("string")}, {TEXT("limit"), TEXT("integer")} },
			{ TEXT("query") }),
		&Impl_SearchAssets);

	Add(TEXT("read_blueprint_reader_asset"),
		TEXT("Extracts a full BlueprintReader dump (Structure + Graph + Design) of the asset at asset_path, as Markdown."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")} }, { TEXT("asset_path") }),
		&Impl_ReadAsset);

	Add(TEXT("read_blueprint_reader_asset_section"),
		TEXT("Extracts a single section (structure | graph | design) of the asset at asset_path — cheaper on tokens."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")}, {TEXT("section"), TEXT("string")} },
			{ TEXT("asset_path"), TEXT("section") }),
		&Impl_ReadAssetSection);

	Add(TEXT("export_blueprint_reader_asset"),
		TEXT("Exports the asset at asset_path to a .md file and returns the written file path."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")}, {TEXT("output_dir"), TEXT("string")} },
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
