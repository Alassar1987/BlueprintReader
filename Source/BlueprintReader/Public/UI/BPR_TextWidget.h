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
    // Intercepting the mouse wheel
    virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
    // Applies the current font size to a text block
    void UpdateFont();

private:
    TSharedPtr<SMultiLineEditableText> MultiLineText;
    TSharedPtr<SScrollBox> ScrollBox;

    // Base font size
    int32 BaseFontSize = 12;

    // Current text scale (1.0 = normal size)
    float TextScale = 1.0f;
};
