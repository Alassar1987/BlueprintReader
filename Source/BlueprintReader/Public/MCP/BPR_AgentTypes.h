// Copyright (c) 2026 Racoon Coder. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BPR_AgentTypes.generated.h"

/**
 * Reference to a discovered asset (used by SearchAssets / GetReferences).
 * Serialized to JSON Schema for MCP clients.
 */
USTRUCT(BlueprintType)
struct FBPR_AssetRef
{
	GENERATED_BODY()

	/** Display name of the asset. */
	UPROPERTY()
	FString Name;

	/** Object path, e.g. /Game/Enemies/BP_Enemy.BP_Enemy */
	UPROPERTY()
	FString Path;

	/** BlueprintReader asset type, e.g. Blueprint / Material / Widget / Structure. */
	UPROPERTY()
	FString Type;

	/** Native UClass name, e.g. Blueprint / Material / UserDefinedStruct. */
	UPROPERTY()
	FString Class;
};

/**
 * Full extraction result for a single asset (Markdown text per section).
 * Serialized to JSON Schema for MCP clients.
 */
USTRUCT(BlueprintType)
struct FBPR_AssetDump
{
	GENERATED_BODY()

	/** Object path of the source asset. */
	UPROPERTY()
	FString AssetPath;

	/** Display name of the source asset. */
	UPROPERTY()
	FString AssetName;

	/** BlueprintReader asset type. */
	UPROPERTY()
	FString Type;

	/** Structure section (variables, components, parameters) — Markdown. */
	UPROPERTY()
	FString Structure;

	/** Graph section (execution flow, data connections) — Markdown. */
	UPROPERTY()
	FString Graph;

	/** Design section (Widget UI hierarchy) — Markdown, empty for non-widget assets. */
	UPROPERTY()
	FString Design;

	/** Asset checksum (trust/freshness). Placeholder in M7; content hash in M8. */
	UPROPERTY()
	FString Checksum;

	/** File path if the dump was exported; empty otherwise. */
	UPROPERTY()
	FString ExportedFile;
};
