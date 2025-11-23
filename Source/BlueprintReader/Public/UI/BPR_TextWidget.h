#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UI/BPR_ScrollBox.h"

class BLUEPRINTREADER_API SBPR_TextWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SBPR_TextWidget) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    void SetText(const FText& NewText);

private:
    TSharedPtr<STextBlock> TextBlock;
    TSharedPtr<SBPR_ScrollBox> ScrollBox;
};
