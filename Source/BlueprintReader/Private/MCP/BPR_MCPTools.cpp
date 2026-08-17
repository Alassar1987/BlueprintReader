// Copyright (c) 2026 Racoon Coder. All rights reserved.

#include "MCP/BPR_MCPTools.h"

#include "Core/BPR_Compat.h"

#if BPR_HAS_MCP

#include "BlueprintReader.h"                 // FBlueprintReaderModule + GetCoreInstance()
#include "Core/BPR_Core.h"
#include "Core/BPR_Types.h"
#include "Export/BPR_Exporter.h"
#include "MCP/BPR_AgentTypes.h"              // FBPR_AssetDump (structured output)

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
#include "IO/IoHash.h"

namespace MCP = UE::ModelContextProtocol;

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
		// Unsupported blueprint subclasses (AnimBlueprint, ControlRig, DamageType,
		// MacroLibrary, PCG nodes, ...) have no extractor. Return an empty type so
		// SearchAssets skips them instead of advertising a "Blueprint" type that
		// ReadAsset cannot actually handle.
		return FString();
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

	/** Resolves (short name or full path) and loads an asset; fills OutError on failure. */
	bool ResolveAndLoadAsset(const FString& AssetPath, UObject*& OutAsset, FString& OutError)
	{
		FString ResolvedPath;
		if (!ResolveAssetPath(AssetPath, ResolvedPath, OutError))
		{
			return false;
		}
		OutAsset = LoadObject<UObject>(nullptr, *ResolvedPath);
		if (!OutAsset)
		{
			OutError = FString::Printf(
				TEXT("Error: asset not found at '%s'. Use search_blueprint_reader_assets to resolve a valid path."),
				*ResolvedPath);
			return false;
		}
		return true;
	}

	bool ExtractAsset(const FString& AssetPath, FBPR_ExtractedData& OutData, FString& OutError)
	{
		UObject* Asset = nullptr;
		if (!ResolveAndLoadAsset(AssetPath, Asset, OutError))
		{
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
		TFunction<FModelContextProtocolToolResult(const TSharedPtr<FJsonObject>&)> Handler;

		virtual FString GetName() const override { return Name; }
		virtual FString GetDescription() const override { return Description; }
		virtual TSharedPtr<FJsonObject> GetInputJsonSchema() const override { return InputSchema; }

		virtual FModelContextProtocolToolResult Run(const TSharedPtr<FJsonObject>& Params) override
		{
			// Synchronous path: game-thread only (handlers touch UObjects). The MCP server
			// dispatches RunAsync, so this is a fallback for direct callers.
			check(IsInGameThread());
			return Handler ? Handler(Params) : MCP::MakeErrorResult(TEXT("no handler"));
		}

		// The MCP server dispatches RunAsync (not Run) from its HTTP worker thread.
		// LoadObject / AssetRegistry / BPR_Core::ExtractAsset are game-thread only, so hop
		// to the game thread before executing the handler. Handler is captured by value, so
		// the task stays valid even if this tool object is destroyed before the task runs.
		virtual void RunAsync(const FModelContextProtocolToolRequestId& RequestId, const TSharedPtr<FJsonObject>& Params, const FResultCallback& OnComplete) override
		{
			auto Execute = [Handler = Handler, Params]()
			{
				return Handler ? Handler(Params) : MCP::MakeErrorResult(TEXT("no handler"));
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

	/** Saved package hash (FIoHash) as hex, or empty when unsaved/unavailable (defined below). */
	FString GetPackageSavedHashString(FName PackageName);

	//--------------------------------------------------------------------------
	// Tool implementations
	//--------------------------------------------------------------------------
	FModelContextProtocolToolResult Impl_ListSupportedTypes(const TSharedPtr<FJsonObject>&)
	{
		const TArray<FString> Types = {
			TEXT("Actor"), TEXT("ActorComponent"), TEXT("Widget"),
			TEXT("Material"), TEXT("MaterialFunction"), TEXT("Enum"),
			TEXT("Structure"), TEXT("Interface")
		};
		return MCP::MakeTextResult(FString::Join(Types, TEXT(", ")));
	}

	FModelContextProtocolToolResult Impl_SearchAssets(const TSharedPtr<FJsonObject>& Params)
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
				if (Type.IsEmpty())
				{
					continue; // no extractor for this blueprint subclass
				}
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

		const FString Result = Lines.Num() > 0
			? FString::Printf(TEXT("%d asset(s):\n"), Lines.Num()) + FString::Join(Lines, TEXT("\n"))
			: TEXT("No assets found.");
		return MCP::MakeTextResult(Result);
	}

	/** True when the agent requested structured (JSON) output; default for read tools. */
	bool IsJsonOutput(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Output = JsonGetString(Params, TEXT("output"), TEXT("json")).TrimStartAndEnd().ToLower();
		return Output == TEXT("json") || Output == TEXT("structured") || Output == TEXT("structured_content");
	}

	/** Validates the 'output' parameter (json | text); returns false + error on unknown value. */
	bool ValidateOutputParam(const TSharedPtr<FJsonObject>& Params, FString& OutError)
	{
		const FString Output = JsonGetString(Params, TEXT("output"), TEXT("json")).TrimStartAndEnd().ToLower();
		if (Output == TEXT("json") || Output == TEXT("structured") || Output == TEXT("structured_content")
			|| Output == TEXT("text"))
		{
			return true;
		}
		OutError = FString::Printf(TEXT("Error: unknown output '%s'. Use: json | text."), *Output);
		return false;
	}

	/** Builds a structured FBPR_AssetDump from extracted data + saved package hash. */
	FBPR_AssetDump BuildAssetDump(const FBPR_ExtractedData& Data, const FString& Checksum)
	{
		FBPR_AssetDump Dump;
		Dump.AssetPath = Data.AssetPath;
		Dump.AssetName = Data.AssetName;
		Dump.Type = BPR_Exporter::AssetTypeToString(Data.AssetType);
		Dump.Structure = Data.Structure.ToString();
		Dump.Graph = Data.Graph.ToString();
		Dump.Design = Data.Design.ToString();
		Dump.Checksum = Checksum;
		return Dump;
	}

	FModelContextProtocolToolResult Impl_ReadAsset(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Path = JsonGetString(Params, TEXT("asset_path"));
		if (Path.IsEmpty())
		{
			return MCP::MakeErrorResult(TEXT("Error: 'asset_path' is required."));
		}

		FString OutputError;
		if (!ValidateOutputParam(Params, OutputError))
		{
			return MCP::MakeErrorResult(OutputError);
		}

		UObject* Asset = nullptr;
		FString Error;
		if (!ResolveAndLoadAsset(Path, Asset, Error))
		{
			return MCP::MakeErrorResult(Error);
		}

		BPR_Core* Core = GetCore();
		if (!Core)
		{
			return MCP::MakeErrorResult(TEXT("Error: BlueprintReader Core is not initialized."));
		}

		FBPR_ExtractedData Data;
		Core->ExtractAsset(Asset, Data);

		if (Data.AssetType == EAssetType::Unknown)
		{
			return MCP::MakeErrorResult(FString::Printf(TEXT("Error: asset '%s' is not a supported BlueprintReader type."), *Path));
		}

		if (IsJsonOutput(Params))
		{
			const FString Checksum = GetPackageSavedHashString(Asset->GetOutermost()->GetFName());
			return MCP::MakeStructuredContentResult(BuildAssetDump(Data, Checksum));
		}

		const EOutputFormat Format = BPR_Exporter::ParseOutputFormat(JsonGetString(Params, TEXT("format")));
		const FString Markdown = BPR_Exporter::BuildMarkdown(Data, Format);
		if (Markdown.IsEmpty())
		{
			return MCP::MakeErrorResult(FString::Printf(TEXT("Error: no extractable data for asset '%s'."), *Path));
		}
		return MCP::MakeTextResult(Markdown);
	}

	FModelContextProtocolToolResult Impl_ReadAssetSection(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Path = JsonGetString(Params, TEXT("asset_path"));
		const FString Section = JsonGetString(Params, TEXT("section"));
		if (Path.IsEmpty())
		{
			return MCP::MakeErrorResult(TEXT("Error: 'asset_path' is required."));
		}

		FString OutputError;
		if (!ValidateOutputParam(Params, OutputError))
		{
			return MCP::MakeErrorResult(OutputError);
		}

		UObject* Asset = nullptr;
		FString Error;
		if (!ResolveAndLoadAsset(Path, Asset, Error))
		{
			return MCP::MakeErrorResult(Error);
		}

		BPR_Core* Core = GetCore();
		if (!Core)
		{
			return MCP::MakeErrorResult(TEXT("Error: BlueprintReader Core is not initialized."));
		}

		FBPR_ExtractedData Data;
		Core->ExtractAsset(Asset, Data);

		if (Data.AssetType == EAssetType::Unknown)
		{
			return MCP::MakeErrorResult(FString::Printf(TEXT("Error: asset '%s' is not a supported BlueprintReader type."), *Path));
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
			return MCP::MakeErrorResult(FString::Printf(TEXT("Error: unknown section '%s'. Use one of: structure, graph, design."), *Section));
		}

		if (IsJsonOutput(Params))
		{
			const FString Checksum = GetPackageSavedHashString(Asset->GetOutermost()->GetFName());
			return MCP::MakeStructuredContentResult(BuildAssetDump(Data, Checksum));
		}

		const EOutputFormat Format = BPR_Exporter::ParseOutputFormat(JsonGetString(Params, TEXT("format")));
		const FString Markdown = BPR_Exporter::BuildMarkdown(Data, Format);
		if (Markdown.IsEmpty())
		{
			return MCP::MakeErrorResult(FString::Printf(TEXT("Error: section '%s' is empty or not present for asset '%s'."), *Section, *Path));
		}
		return MCP::MakeTextResult(Markdown);
	}

	FModelContextProtocolToolResult Impl_ExportAsset(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Path = JsonGetString(Params, TEXT("asset_path"));
		const FString OutputDir = JsonGetString(Params, TEXT("output_dir"));
		if (Path.IsEmpty())
		{
			return MCP::MakeErrorResult(TEXT("Error: 'asset_path' is required."));
		}

		FBPR_ExtractedData Data;
		FString Error;
		if (!ExtractAsset(Path, Data, Error))
		{
			return MCP::MakeErrorResult(Error);
		}

		if (Data.AssetType == EAssetType::Unknown)
		{
			return MCP::MakeErrorResult(FString::Printf(TEXT("Error: asset '%s' is not a supported BlueprintReader type."), *Path));
		}

		FString Dir = OutputDir.IsEmpty()
			? FPaths::Combine(FPaths::ProjectDir(), TEXT("BPR-TEMP"))
			: OutputDir;
		IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);

		const EOutputFormat Format = BPR_Exporter::ParseOutputFormat(JsonGetString(Params, TEXT("format")));
		if (BPR_Exporter::BuildMarkdown(Data, Format).IsEmpty())
		{
			return MCP::MakeErrorResult(FString::Printf(TEXT("Error: no extractable data for asset '%s'."), *Path));
		}

		const FString FullPath = FPaths::Combine(Dir, Data.AssetName + TEXT(".md"));
		const FString Written = BPR_Exporter::ExportToFile(Data, FullPath, Format);
		if (!Written.IsEmpty())
		{
			return MCP::MakeTextResult(Written);
		}
		return MCP::MakeErrorResult(FString::Printf(TEXT("Error: failed to write '%s'"), *FullPath));
	}

	//--------------------------------------------------------------------------
	// M8: validation / references / checksum
	//--------------------------------------------------------------------------
	FString BlueprintStatusToString(EBlueprintStatus Status)
	{
		switch (Status)
		{
		case BS_Dirty:                return TEXT("Dirty");
		case BS_Error:                return TEXT("Error");
		case BS_UpToDate:             return TEXT("UpToDate");
		case BS_BeingCreated:         return TEXT("BeingCreated");
		case BS_UpToDateWithWarnings: return TEXT("UpToDateWithWarnings");
		case BS_Unknown:
		default:                      return TEXT("Unknown");
		}
	}

	/** Saved package hash (FIoHash) as hex, or empty when unsaved/unavailable. */
	FString GetPackageSavedHashString(FName PackageName)
	{
		IAssetRegistry& AssetRegistry =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		const TOptional<FAssetPackageData> PackageData = AssetRegistry.GetAssetPackageDataCopy(PackageName);
		if (!PackageData.IsSet())
		{
			return FString();
		}
#if WITH_EDITORONLY_DATA
		const FIoHash Hash = PackageData->GetPackageSavedHash();
		if (Hash.IsZero())
		{
			return FString();
		}
		return LexToString(Hash);
#else
		return FString();
#endif
	}

	FModelContextProtocolToolResult Impl_GetChecksum(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Path = JsonGetString(Params, TEXT("asset_path"));
		if (Path.IsEmpty())
		{
			return MCP::MakeErrorResult(TEXT("Error: 'asset_path' is required."));
		}

		UObject* Asset = nullptr;
		FString Error;
		if (!ResolveAndLoadAsset(Path, Asset, Error))
		{
			return MCP::MakeErrorResult(Error);
		}

		const FName PackageName = Asset->GetOutermost()->GetFName();
		const FString Hash = GetPackageSavedHashString(PackageName);
		if (Hash.IsEmpty())
		{
			return MCP::MakeErrorResult(FString::Printf(
				TEXT("Error: no saved package hash for '%s' (package may be unsaved or not on disk)."),
				*PackageName.ToString()));
		}
		return MCP::MakeTextResult(Hash);
	}

	FModelContextProtocolToolResult Impl_GetReferences(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Path = JsonGetString(Params, TEXT("asset_path"));
		const FString Direction = JsonGetString(Params, TEXT("direction"), TEXT("dependencies"));
		if (Path.IsEmpty())
		{
			return MCP::MakeErrorResult(TEXT("Error: 'asset_path' is required."));
		}

		UObject* Asset = nullptr;
		FString Error;
		if (!ResolveAndLoadAsset(Path, Asset, Error))
		{
			return MCP::MakeErrorResult(Error);
		}

		IAssetRegistry& AssetRegistry =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

		const FName PackageName = Asset->GetOutermost()->GetFName();
		const FString DirectionLower = Direction.ToLower();
		if (DirectionLower != TEXT("dependencies") && DirectionLower != TEXT("referencers"))
		{
			return MCP::MakeErrorResult(FString::Printf(TEXT("Error: unknown direction '%s'. Use: dependencies | referencers."), *Direction));
		}
		const bool bReferencers = (DirectionLower == TEXT("referencers"));
		TArray<FName> Refs;
		if (bReferencers)
		{
			AssetRegistry.GetReferencers(PackageName, Refs, UE::AssetRegistry::EDependencyCategory::All);
		}
		else
		{
			AssetRegistry.GetDependencies(PackageName, Refs, UE::AssetRegistry::EDependencyCategory::All);
		}

		const TCHAR* Label = bReferencers ? TEXT("referencers") : TEXT("dependencies");
		if (Refs.Num() == 0)
		{
			return MCP::MakeTextResult(FString::Printf(TEXT("0 %s for '%s'."), Label, *PackageName.ToString()));
		}

		Refs.Sort(FNameLexicalLess());
		FString Out = FString::Printf(TEXT("%d %s for '%s':\n"), Refs.Num(), Label, *PackageName.ToString());
		for (const FName& Ref : Refs)
		{
			Out += Ref.ToString() + TEXT("\n");
		}
		return MCP::MakeTextResult(Out.TrimEnd());
	}

	FModelContextProtocolToolResult Impl_ValidateAsset(const TSharedPtr<FJsonObject>& Params)
	{
		const FString Path = JsonGetString(Params, TEXT("asset_path"));
		if (Path.IsEmpty())
		{
			return MCP::MakeErrorResult(TEXT("Error: 'asset_path' is required."));
		}

		UObject* Asset = nullptr;
		FString Error;
		if (!ResolveAndLoadAsset(Path, Asset, Error))
		{
			return MCP::MakeErrorResult(Error);
		}

		FString Out;
		Out += FString::Printf(TEXT("# Validation: %s\n\n"), *Asset->GetName());
		Out += FString::Printf(TEXT("> %s\n\n"), *Asset->GetPathName());

		bool bValid = true;

		// Compile status (Blueprints only; other types carry no compile-state concept here).
		if (UBlueprint* BP = Cast<UBlueprint>(Asset))
		{
			const EBlueprintStatus Status = BP->Status;
			const bool bHasErrors = (Status == BS_Error);
			const bool bUpToDate = (Status == BS_UpToDate || Status == BS_UpToDateWithWarnings);
			Out += TEXT("## Compile\n");
			Out += FString::Printf(TEXT("- Status: %s\n"), *BlueprintStatusToString(Status));
			Out += FString::Printf(TEXT("- Up to date: %s\n"), bUpToDate ? TEXT("true") : TEXT("false"));
			Out += FString::Printf(TEXT("- Has compile errors: %s\n\n"), bHasErrors ? TEXT("true") : TEXT("false"));
			if (bHasErrors)
			{
				bValid = false;
			}
		}

		// Dependencies + missing /Game/ packages (informational heuristic).
		IAssetRegistry& AssetRegistry =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		const FName PackageName = Asset->GetOutermost()->GetFName();
		TArray<FName> Deps;
		AssetRegistry.GetDependencies(PackageName, Deps, UE::AssetRegistry::EDependencyCategory::All);

		int32 GameDeps = 0;
		TArray<FString> MissingGameDeps;
		for (const FName& Dep : Deps)
		{
			const FString DepStr = Dep.ToString();
			if (!DepStr.StartsWith(TEXT("/Game/")))
			{
				continue;
			}
			++GameDeps;
			TArray<FAssetData> Found;
			AssetRegistry.GetAssetsByPackageName(Dep, Found);
			if (Found.Num() == 0)
			{
				MissingGameDeps.Add(DepStr);
			}
		}

		Out += TEXT("## Dependencies\n");
		Out += FString::Printf(TEXT("- Total: %d\n"), Deps.Num());
		Out += FString::Printf(TEXT("- /Game/ packages: %d\n"), GameDeps);
		Out += FString::Printf(TEXT("- Missing from registry: %d\n"), MissingGameDeps.Num());
		for (const FString& M : MissingGameDeps)
		{
			Out += FString::Printf(TEXT("  - %s\n"), *M);
		}
		Out += TEXT("\n");

		// Checksum
		const FString Hash = GetPackageSavedHashString(PackageName);
		Out += TEXT("## Checksum\n");
		if (Hash.IsEmpty())
		{
			Out += TEXT("- none (unsaved)\n\n");
		}
		else
		{
			Out += FString::Printf(TEXT("- %s\n\n"), *Hash);
		}

		// Verdict
		Out += TEXT("## Verdict\n");
		Out += FString::Printf(TEXT("- Valid: %s\n"), bValid ? TEXT("true") : TEXT("false"));

		return MCP::MakeTextResult(Out);
	}
}

#endif // BPR_HAS_MCP

//------------------------------------------------------------------------------
// Registration (defined on all engine versions; bodies are MCP-only)
//------------------------------------------------------------------------------
void RegisterBlueprintReaderMCPTools()
{
#if BPR_HAS_MCP
	IModelContextProtocolModule* MCPModule = IModelContextProtocolModule::Get();
	if (!MCPModule)
	{
		return; // Unreal MCP plugin not enabled — nothing to register.
	}

	auto Add = [&MCPModule](const FString& Name, const FString& Description,
		TSharedPtr<FJsonObject> Schema, TFunction<FModelContextProtocolToolResult(const TSharedPtr<FJsonObject>&)> Handler)
	{
		TSharedRef<FBPR_MCPTool> Tool = MakeShared<FBPR_MCPTool>();
		Tool->Name = Name;
		Tool->Description = Description;
		Tool->InputSchema = Schema;
		Tool->Handler = Handler;
		if (MCPModule->AddTool(Tool))
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
		TEXT("Extracts the asset at asset_path. 'output': json (default, structured) | text (Markdown); 'format' applies to text output."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")}, {TEXT("output"), TEXT("string")}, {TEXT("format"), TEXT("string")} },
			{ TEXT("asset_path") }),
		&Impl_ReadAsset);

	Add(TEXT("read_blueprint_reader_asset_section"),
		TEXT("Extracts a single section (structure | graph | design) of the asset at asset_path. 'output': json (default) | text."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")}, {TEXT("section"), TEXT("string")}, {TEXT("output"), TEXT("string")}, {TEXT("format"), TEXT("string")} },
			{ TEXT("asset_path"), TEXT("section") }),
		&Impl_ReadAssetSection);

	Add(TEXT("export_blueprint_reader_asset"),
		TEXT("Exports the asset at asset_path to a .md file and returns the written file path. Optional 'output_dir' and 'format' (human_readable | compact | minimal, default compact)."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")}, {TEXT("output_dir"), TEXT("string")}, {TEXT("format"), TEXT("string")} },
			{ TEXT("asset_path") }),
		&Impl_ExportAsset);

	Add(TEXT("get_blueprint_reader_checksum"),
		TEXT("Returns the saved package hash (checksum) of the asset at asset_path — for freshness checks."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")} }, { TEXT("asset_path") }),
		&Impl_GetChecksum);

	Add(TEXT("get_blueprint_reader_references"),
		TEXT("Lists the asset's package dependencies (or referencers) from the Asset Registry. 'direction': dependencies (default) | referencers."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")}, {TEXT("direction"), TEXT("string")} },
			{ TEXT("asset_path") }),
		&Impl_GetReferences);

	Add(TEXT("validate_blueprint_reader_asset"),
		TEXT("Validates the asset: compile status (Blueprints), dependency count + missing /Game/ packages, checksum, verdict."),
		MakeInputSchema({ {TEXT("asset_path"), TEXT("string")} }, { TEXT("asset_path") }),
		&Impl_ValidateAsset);
#endif
}

void UnregisterBlueprintReaderMCPTools()
{
#if BPR_HAS_MCP
	if (IModelContextProtocolModule* MCPModule = IModelContextProtocolModule::Get())
	{
		for (const TSharedRef<IModelContextProtocolTool>& Tool : GRegisteredTools)
		{
			MCPModule->RemoveTool(Tool);
		}
	}
	GRegisteredTools.Empty();
#endif
}
