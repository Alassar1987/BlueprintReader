#include "UI/BPR_TextWidget.h"

void SBPR_TextWidget::Construct(const FArguments& InArgs)
{
    // Создаём виджет
    ChildSlot
    [
        SAssignNew(ScrollBox, SBPR_ScrollBox)
        .Orientation(Orient_Vertical)
        [
            SAssignNew(MultiLineText, SMultiLineEditableText)
            // Текст по умолчанию
            .Text(FText::FromString("Waiting for data..."))
            // Можно выделять текст и копировать
            .IsReadOnly(true)
            .AllowContextMenu(true)
            .AutoWrapText(true)
        ]
    ];
    
    UE_LOG(LogTemp, Log, TEXT("SBPR_TextWidget: Constructed successfully"));
}

void SBPR_TextWidget::SetText(const FText& InText)
{
    if (!MultiLineText.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SBPR_TextWidget: Cannot set text - MultiLineText is invalid!"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("SBPR_TextWidget: Setting text (length: %d): %s"), 
        InText.ToString().Len(), 
        *InText.ToString().Left(100)); // Логируем первые 100 символов
    
    MultiLineText->SetText(InText);
}