#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Layout/SScrollBox.h"

class SBPR_TextWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SBPR_TextWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetText(const FText& InText);

protected:
    // Перехватываем колесо мыши
    virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
    // Применяет текущий размер шрифта к текстовому блоку
    void UpdateFont();

private:
    TSharedPtr<SMultiLineEditableText> MultiLineText;
    TSharedPtr<SScrollBox> ScrollBox;

    // Базовый размер шрифта
    int32 BaseFontSize = 12;

    // Текущий масштаб текста (1.0 = нормальный размер)
    float TextScale = 1.0f;
};
