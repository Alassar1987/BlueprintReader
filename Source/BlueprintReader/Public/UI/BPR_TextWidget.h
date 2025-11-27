#pragma once

#include "Widgets/SCompoundWidget.h"
#include "UI/BPR_ScrollBox.h" // твой кастомный ScrollBox
#include "Widgets/Text/SMultiLineEditableText.h"

class SBPR_TextWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SBPR_TextWidget) {}
    SLATE_END_ARGS()

    /** Конструирование виджета */
    void Construct(const FArguments& InArgs);

    /** Устанавливает новый текст */
    void SetText(const FText& InText);

protected:
    /** Ссылка на мультилайн текст */
    TSharedPtr<SMultiLineEditableText> MultiLineText;

    /** Скроллбокс для текста */
    TSharedPtr<SBPR_ScrollBox> ScrollBox;
};
