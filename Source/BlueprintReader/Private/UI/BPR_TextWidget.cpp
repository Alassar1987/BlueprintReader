#include "UI/BPR_TextWidget.h"

void SBPR_TextWidget::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SScrollBox)
        .Orientation(EOrientation::Orient_Vertical)

        + SScrollBox::Slot()
        [
            SAssignNew(MultiLineText, SMultiLineEditableText)
            .IsReadOnly(true)
            .AllowContextMenu(true)
            .AutoWrapText(true)
            .Text(FText::FromString("Waiting for data..."))
        ]
    ];

    UE_LOG(LogTemp, Log, TEXT("SBPR_TextWidget: Constructed"));
}

void SBPR_TextWidget::SetText(const FText& InText)
{
    // Конструкция гарантирует MultiLineText после Construct
    MultiLineText->SetText(InText);

    UE_LOG(LogTemp, Log, TEXT("SBPR_TextWidget: Text applied (%d chars)"),
        InText.ToString().Len());
}
