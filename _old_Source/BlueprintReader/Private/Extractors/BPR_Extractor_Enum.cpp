#include "Extractors/BPR_Extractor_Enum.h"
#include "UObject/UnrealType.h" // для UEnum
#include "Misc/OutputDeviceDebug.h"

BPR_Extractor_Enum::BPR_Extractor_Enum() {}
BPR_Extractor_Enum::~BPR_Extractor_Enum() {}

void BPR_Extractor_Enum::ProcessEnum(UObject* Object, FText& OutText)
{
#if WITH_EDITOR
    FString Result;

    UEnum* Enum = Cast<UEnum>(Object);
    if (!Enum)
    {
        OutText = FText::FromString("Object is not an Enum.");
        return;
    }

    Result += FString::Printf(TEXT("# Enum: %s\n\n"), *Enum->GetName());

    AppendEnumEntries(Enum, Result);

    OutText = FText::FromString(Result);
#endif
}

void BPR_Extractor_Enum::AppendEnumEntries(UEnum* Enum, FString& OutText)
{
#if WITH_EDITOR
    int32 NumEnums = Enum->NumEnums();
    for (int32 i = 0; i < NumEnums; ++i)
    {
        // Получаем имя и значение
        FString Name = Enum->GetNameStringByIndex(i);
        int64 Value = Enum->GetValueByIndex(i);

        // Можно добавить метаданные, если нужно:
        FString DisplayName = Enum->GetDisplayNameTextByIndex(i).ToString();

        OutText += FString::Printf(TEXT("### %d) %s = %lld (%s)\n"), i, *Name, Value, *DisplayName);
    }
#endif
}
