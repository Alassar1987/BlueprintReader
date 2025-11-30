#include "UI/BPR_TextWidget.h"
#include "Styling/CoreStyle.h"
#include "Fonts/SlateFontInfo.h"
#include "Logging/LogMacros.h"

void SBPR_TextWidget::Construct(const FArguments& InArgs)
{
    BaseFontSize = 12;
    TextScale = 1.0f;

    ChildSlot
[
    SAssignNew(ScrollBox, SScrollBox)
    .Orientation(EOrientation::Orient_Vertical)

    + SScrollBox::Slot()
    [
        SAssignNew(MultiLineText, SMultiLineEditableText)
        .Text(FText::FromString("Waiting for data..."))
        .Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), BaseFontSize))
        .IsReadOnly(true)
        .AutoWrapText(true)
        .AllowContextMenu(true)
    ]
];


    UpdateFont();

    UE_LOG(LogTemp, Log, TEXT("SBPR_TextWidget: Constructed"));
}

void SBPR_TextWidget::SetText(const FText& InText)
{
    if (MultiLineText.IsValid())
    {
        MultiLineText->SetText(InText);
        UE_LOG(LogTemp, Log, TEXT("SBPR_TextWidget: Text applied (%d chars)"), InText.ToString().Len());
    }
}

FReply SBPR_TextWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.IsControlDown())
    {
        // Масштабируем текст
        TextScale = FMath::Clamp(TextScale + MouseEvent.GetWheelDelta() * 0.1f, 0.5f, 3.0f);
        UpdateFont();
        return FReply::Handled();
    }

    // Обычный скролл через ScrollBox
    if (ScrollBox.IsValid())
    {
        return ScrollBox->OnMouseWheel(MyGeometry, MouseEvent);
    }

    return FReply::Unhandled();
}

void SBPR_TextWidget::UpdateFont()
{
    if (MultiLineText.IsValid())
    {
        FSlateFontInfo FontInfo = MultiLineText->GetFont();
        FontInfo.Size = FMath::RoundToInt(BaseFontSize * TextScale);
        MultiLineText->SetFont(FontInfo);

        UE_LOG(LogTemp, Log, TEXT("SBPR_TextWidget: Font updated to %d"), FMath::RoundToInt(BaseFontSize * TextScale));

    }
}
