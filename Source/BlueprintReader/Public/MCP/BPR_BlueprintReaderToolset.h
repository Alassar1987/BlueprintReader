// Copyright (c) 2026 Racoon Coder. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ToolsetRegistry/ToolsetDefinition.h"
#include "MCP/BPR_AgentTypes.h"
#include "BPR_BlueprintReaderToolset.generated.h"

/**
 * MCP toolset — exposes BlueprintReader extraction to AI agents (UE 5.8 Unreal MCP).
 *
 * Discovered automatically by the Toolset Registry: any static UFUNCTION marked
 * meta=(AICallable) becomes an MCP Tool. The MCP server serializes tool calls on the
 * game thread, matching BlueprintReader's synchronous extraction.
 *
 * Conventions (per Epic Unreal MCP docs):
 *  - small focused functions, one tool = one responsibility
 *  - structured return types (USTRUCT -> JSON Schema), not free-form strings
 */
UCLASS(BlueprintType, Hidden)
class UBlueprintReaderToolset : public UToolsetDefinition
{
	GENERATED_BODY()

public:
	/** Lists the asset types BlueprintReader can extract. */
	UFUNCTION(meta=(AICallable))
	static TArray<FString> ListSupportedTypes();

	/** Searches supported assets by name substring, with an optional type filter. */
	UFUNCTION(meta=(AICallable))
	static TArray<FBPR_AssetRef> SearchAssets(
		const FString& Query, const FString& TypeFilter, int32 Limit = 20);

	/** Extracts a full asset dump (Structure + Graph + Design) as Markdown. */
	UFUNCTION(meta=(AICallable))
	static FBPR_AssetDump ReadAsset(const FString& AssetPath);

	/** Extracts a single section only (Structure | Graph | Design) — token economy. */
	UFUNCTION(meta=(AICallable))
	static FBPR_AssetDump ReadAssetSection(const FString& AssetPath, const FString& Section);

	/** Exports the asset dump to a .md file and returns the written file path. */
	UFUNCTION(meta=(AICallable))
	static FString ExportAsset(
		const FString& AssetPath, const FString& OutputDir, const FString& Format);
};
