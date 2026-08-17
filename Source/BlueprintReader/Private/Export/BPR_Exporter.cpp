// Copyright (c) 2026 Racoon Coder. All rights reserved.

#include "Export/BPR_Exporter.h"

#include "Misc/FileHelper.h"

namespace
{
	/** True when a section carries real content rather than an extractor placeholder. */
	bool IsMeaningful(const FText& InText)
	{
		const FString S = InText.ToString().TrimStartAndEnd();
		return !S.IsEmpty() && S != TEXT("N/A") && S != TEXT("No Data found");
	}
}

namespace BPR_Exporter
{

FString AssetTypeToString(EAssetType Type)
{
	switch (Type)
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

EOutputFormat ParseOutputFormat(const FString& FormatString)
{
	const FString Lower = FormatString.TrimStartAndEnd().ToLower();
	if (Lower == TEXT("human_readable") || Lower == TEXT("humanreadable") || Lower == TEXT("human") || Lower == TEXT("full"))
	{
		return EOutputFormat::HumanReadable;
	}
	if (Lower == TEXT("minimal") || Lower == TEXT("min"))
	{
		return EOutputFormat::Minimal;
	}
	return EOutputFormat::Compact; // "compact" and any unknown value
}

FString BuildDocument(const FBPR_ExtractedData& Data, EOutputFormat Format, bool bMarkdown)
{
	const bool bMinimal = (Format == EOutputFormat::Minimal);

	// Collect meaningful sections in canonical order (Structure -> Graph -> Design).
	TArray<TPair<FString, FString>> Sections;
	if (IsMeaningful(Data.Structure)) Sections.Add(TPair<FString, FString>(TEXT("Structure"), Data.Structure.ToString()));
	if (IsMeaningful(Data.Graph))     Sections.Add(TPair<FString, FString>(TEXT("Graph"),     Data.Graph.ToString()));
	// Design is a human-facing (Widget UI hierarchy) section; drop it in Minimal to save tokens.
	if (!bMinimal && IsMeaningful(Data.Design))
	{
		Sections.Add(TPair<FString, FString>(TEXT("Design"), Data.Design.ToString()));
	}

	if (Sections.Num() == 0)
	{
		return TEXT("");
	}

	const FString Title = FString::Printf(
		TEXT("%s (%s)"), *Data.AssetName, *AssetTypeToString(Data.AssetType));
	const FString Path = Data.AssetPath;

	FString Out;

	switch (Format)
	{
	case EOutputFormat::Minimal:
	{
		// Bare content: plain title + path, then "Section:" headers, no Markdown symbols.
		Out += Title + TEXT("\n");
		if (!Path.IsEmpty()) { Out += Path + TEXT("\n"); }
		for (const TPair<FString, FString>& Section : Sections)
		{
			Out += Section.Key + TEXT(":\n") + Section.Value + TEXT("\n");
		}
		break;
	}

	case EOutputFormat::Compact:
	{
		// Markdown headings, no decorative blank lines (token-balanced default for agents).
		Out += (bMarkdown ? TEXT("# ") : TEXT("")) + Title + TEXT("\n");
		if (!Path.IsEmpty()) { Out += (bMarkdown ? TEXT("> ") : TEXT("")) + Path + TEXT("\n"); }
		for (const TPair<FString, FString>& Section : Sections)
		{
			Out += (bMarkdown ? TEXT("## ") : TEXT("")) + Section.Key + TEXT("\n");
			Out += Section.Value + TEXT("\n");
		}
		break;
	}

	case EOutputFormat::HumanReadable:
	default:
	{
		// Full Markdown, spaced for humans.
		Out += (bMarkdown ? TEXT("# ") : TEXT("")) + Title + TEXT("\n\n");
		if (!Path.IsEmpty()) { Out += (bMarkdown ? TEXT("> ") : TEXT("")) + Path + TEXT("\n\n"); }
		for (const TPair<FString, FString>& Section : Sections)
		{
			Out += (bMarkdown ? TEXT("## ") : TEXT("")) + Section.Key + TEXT("\n\n");
			Out += Section.Value + TEXT("\n\n");
		}
		break;
	}
	}

	return Out.TrimEnd();
}

FString BuildMarkdown(const FBPR_ExtractedData& Data, EOutputFormat Format)
{
	return BuildDocument(Data, Format, /*bMarkdown=*/true);
}

FString ExportToFile(const FBPR_ExtractedData& Data, const FString& TargetPath, EOutputFormat Format, bool bMarkdown)
{
	const FString Document = BuildDocument(Data, Format, bMarkdown);
	if (Document.IsEmpty())
	{
		return TEXT("");
	}

	if (FFileHelper::SaveStringToFile(Document, *TargetPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		return TargetPath;
	}
	return TEXT("");
}

} // namespace BPR_Exporter
