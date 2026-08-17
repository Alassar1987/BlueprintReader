// Copyright (c) 2026 Racoon Coder. All rights reserved.

#include "MCP/BPR_BlueprintReaderToolset.h"

#include "BlueprintReader.h"                 // FBlueprintReaderModule + GetCoreInstance()
#include "Core/BPR_Core.h"
#include "Core/BPR_Types.h"

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
// Local helpers
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

	// Asset-registry class name → BlueprintReader type string (coarse; Actor/ActorComponent/
	// Interface all live under UBlueprint and are only distinguished at extract time).
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
				TEXT("Error: asset not found at '%s'. Use SearchAssets to resolve a valid path."),
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
}

//------------------------------------------------------------------------------
// Tools
//------------------------------------------------------------------------------
TArray<FString> UBlueprintReaderToolset::ListSupportedTypes()
{
	return {
		TEXT("Blueprint"), TEXT("Actor"), TEXT("ActorComponent"), TEXT("Widget"),
		TEXT("Material"), TEXT("MaterialFunction"), TEXT("Enum"),
		TEXT("Structure"), TEXT("Interface")
	};
}

TArray<FBPR_AssetRef> UBlueprintReaderToolset::SearchAssets(
	const FString& Query, const FString& TypeFilter, int32 Limit)
{
	TArray<FBPR_AssetRef> Results;

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
	TSet<FString> SeenPaths;

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

			FBPR_AssetRef Ref;
			Ref.Name = Name;
			Ref.Path = Asset.GetObjectPathString();
			Ref.Class = Asset.AssetClassPath.GetAssetName().ToString();
			Ref.Type = ClassToTypeString(Asset.AssetClassPath.GetAssetName());

			if (!TypeFilterLower.IsEmpty() && Ref.Type.ToLower() != TypeFilterLower)
			{
				continue;
			}

			if (SeenPaths.Contains(Ref.Path))
			{
				continue;
			}
			SeenPaths.Add(Ref.Path);
			Results.Add(Ref);

			if (Limit > 0 && Results.Num() >= Limit)
			{
				break;
			}
		}

		if (Limit > 0 && Results.Num() >= Limit)
		{
			break;
		}
	}

	return Results;
}

FBPR_AssetDump UBlueprintReaderToolset::ReadAsset(const FString& AssetPath)
{
	return ExtractDump(AssetPath, TEXT(""));
}

FBPR_AssetDump UBlueprintReaderToolset::ReadAssetSection(
	const FString& AssetPath, const FString& Section)
{
	return ExtractDump(AssetPath, Section);
}

FString UBlueprintReaderToolset::ExportAsset(
	const FString& AssetPath, const FString& OutputDir, const FString& Format)
{
	FBPR_AssetDump Dump = ExtractDump(AssetPath, TEXT(""));
	if (Dump.Type == TEXT("NotFound") || Dump.Type == TEXT("Error"))
	{
		return FString::Printf(TEXT("Error: %s"), *Dump.Structure);
	}

	FString Dir = OutputDir.IsEmpty()
		? FPaths::Combine(FPaths::ProjectDir(), TEXT("BPR-TEMP"))
		: OutputDir;
	IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);

	// TODO M9: honour `Format` (HumanReadable/Compact/Minimal) once BPR_Exporter is extracted.
	FString Markdown;
	Markdown += FString::Printf(TEXT("# %s (%s)\n\n"), *Dump.AssetName, *Dump.Type);
	if (!Dump.Structure.IsEmpty()) { Markdown += TEXT("## Structure\n\n") + Dump.Structure + TEXT("\n\n"); }
	if (!Dump.Graph.IsEmpty())     { Markdown += TEXT("## Graph\n\n")     + Dump.Graph     + TEXT("\n\n"); }
	if (!Dump.Design.IsEmpty())    { Markdown += TEXT("## Design\n\n")    + Dump.Design    + TEXT("\n\n"); }

	const FString FullPath = FPaths::Combine(Dir, Dump.AssetName + TEXT(".md"));
	if (FFileHelper::SaveStringToFile(Markdown, *FullPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		return FullPath;
	}
	return FString::Printf(TEXT("Error: failed to write '%s'"), *FullPath);
}
