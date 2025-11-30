#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Core/BPR_Core.h"

struct FBPR_ExtractedData;

class SBPR_TabSwitcher : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBPR_TabSwitcher) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetData(const FBPR_ExtractedData& InData);

private:
	FReply OnStructureTabClicked();
	FReply OnGraphTabClicked();
	void SwitchToIndex(int32 Index);
    
	// Отложенная установка данных через ActiveTimer
	EActiveTimerReturnType ApplyDataOnNextTick(double InCurrentTime, float InDeltaTime);

private:
	TSharedPtr<SWidgetSwitcher> TabSwitcher;
	TSharedPtr<class SBPR_TextWidget> StructureTextWidget;
	TSharedPtr<class SBPR_TextWidget> GraphTextWidget;
    
	// Данные для отложенной установки
	TOptional<FBPR_ExtractedData> PendingData;
};