// Copyright (c) 2026 Racoon Coder. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BPR_Types.h"

/**
 * BPR_Exporter
 *
 * Single shared export layer used by both the UI "Export to file…" button and
 * the MCP export tool. It turns FBPR_ExtractedData into a document and writes it
 * to disk, so the two entry points never duplicate formatting/serialization logic.
 *
 * Design rule: keep this layer pure (no Slate, no MCP). It only depends on
 * Core/BPR_Types.h, so it builds on every engine version the plugin supports.
 */
namespace BPR_Exporter
{
	/** Maps an EAssetType to its display string, e.g. EAssetType::Widget -> "Widget". */
	BLUEPRINTREADER_API FString AssetTypeToString(EAssetType Type);

	/**
	 * Parses a format string to EOutputFormat. Accepted: "human_readable" (also
	 * "humanreadable"/"human"/"full"), "minimal" (also "min"), "compact". Anything
	 * else (including empty) falls back to Compact.
	 */
	BLUEPRINTREADER_API EOutputFormat ParseOutputFormat(const FString& FormatString);

	/**
	 * Builds the export document from extracted data.
	 *
	 * Only sections with real content are emitted; placeholder values ("N/A",
	 * "No Data found", empty) are skipped. Returns an empty string when there is
	 * nothing meaningful to export.
	 *
	 * @param Format     HumanReadable (full Markdown, spaced for people),
	 *                   Compact (balanced, fewer blank lines — good for LLMs),
	 *                   Minimal (bare content, least tokens).
	 * @param bMarkdown  When true, uses Markdown headings (# / ##); when false,
	 *                   emits plain-text section titles (for .txt export).
	 */
	BLUEPRINTREADER_API FString BuildDocument(const FBPR_ExtractedData& Data, EOutputFormat Format, bool bMarkdown = true);

	/** Convenience: BuildDocument with bMarkdown = true. */
	BLUEPRINTREADER_API FString BuildMarkdown(const FBPR_ExtractedData& Data, EOutputFormat Format);

	/**
	 * Builds the document for Data and writes it to TargetPath (UTF-8, no BOM).
	 * @return TargetPath on success, empty string on failure (or when there is
	 *         no meaningful content).
	 */
	BLUEPRINTREADER_API FString ExportToFile(const FBPR_ExtractedData& Data, const FString& TargetPath, EOutputFormat Format, bool bMarkdown = true);
}
