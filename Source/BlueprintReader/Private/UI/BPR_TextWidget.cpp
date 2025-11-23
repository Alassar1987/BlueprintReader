#include "UI/BPR_TextWidget.h"

void SBPR_TextWidget::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SAssignNew(ScrollBox, SBPR_ScrollBox)
        .Orientation(EOrientation::Orient_Vertical)
        [
            SAssignNew(TextBlock, STextBlock)
            .AutoWrapText(true)
        ]
    ];
	
    // Устанавливаем начальный текст сразу после создания виджета
    if (TextBlock.IsValid())
    {
        TextBlock->SetText(FText::FromString("Привет, мир!"));
    }
}

void SBPR_TextWidget::SetText(const FText& NewText)
{
    if (TextBlock.IsValid())
    {
        TextBlock->SetText(NewText);
    }
}
