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
            // Текст по умолчанию пустой
            .Text(FText::GetEmpty())
            // Можно выделять текст и копировать
            .IsReadOnly(true)
            .AllowContextMenu(true)
            .AutoWrapText(true)
            // При необходимости можно добавить стили текста
        ]
    ];
}

void SBPR_TextWidget::SetText(const FText& InText)
{
    if (MultiLineText.IsValid())
    {
        MultiLineText->SetText(InText);
    }
}
