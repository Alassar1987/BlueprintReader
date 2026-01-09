// Copyright (c) 2026 Racoon Coder. All rights reserved.

#include "Extractors/BPR_Extractor_Enum.h"
#include "UObject/UnrealType.h" // для UEnum

BPR_Extractor_Enum::BPR_Extractor_Enum() {}
BPR_Extractor_Enum::~BPR_Extractor_Enum() {}

void BPR_Extractor_Enum::ProcessEnum(UObject* Object, FBPR_ExtractedData& OutData)
{
#if WITH_EDITOR
    FString StructureText;
    FString GraphText = TEXT("# Enum has no graph\n");

    UEnum* Enum = Cast<UEnum>(Object);
    if (!Enum)
    {
        StructureText = TEXT("Object is not an Enum.");
        GraphText = TEXT("N/A");
        OutData.Structure = FText::FromString(StructureText);
        OutData.Graph = FText::FromString(GraphText);
        return;
    }

    StructureText += FString::Printf(TEXT("# Enum: %s\n\n"), *Enum->GetName());

    AppendEnumEntries(Enum, StructureText);

    OutData.Structure = FText::FromString(StructureText);
    OutData.Graph = FText::FromString(GraphText);
#endif
}

void BPR_Extractor_Enum::AppendEnumEntries(UEnum* Enum, FString& OutText)
{
#if WITH_EDITOR
    int32 NumEnums = Enum->NumEnums();
    for (int32 i = 0; i < NumEnums; ++i)
    {
        FString Name = Enum->GetNameStringByIndex(i);
        int64 Value = Enum->GetValueByIndex(i);
        FString DisplayName = Enum->GetDisplayNameTextByIndex(i).ToString();

        OutText += FString::Printf(TEXT("### %d) %s = %lld (%s)\n"), i, *Name, Value, *DisplayName);
    }
#endif
}
