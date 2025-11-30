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

private:
    TSharedPtr<SMultiLineEditableText> MultiLineText;
};
