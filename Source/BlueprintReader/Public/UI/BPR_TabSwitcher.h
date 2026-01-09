// Copyright (c) 2026 Racoon Coder. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Core/BPR_Core.h"

class SBPR_TabSwitcher : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBPR_TabSwitcher)
		: _InitialData()
	{}
	SLATE_ARGUMENT(TOptional<FBPR_ExtractedData>, InitialData)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Sets the data after the widget is created */
	void SetData(const FBPR_ExtractedData& InData);

private:
	FReply OnStructureTabClicked();
	FReply OnGraphTabClicked();
	void SwitchToIndex(int32 Index);

	/** Applies data if widgets are ready */
	void ApplyPendingData();

private:
	TSharedPtr<SWidgetSwitcher> TabSwitcher;
	TSharedPtr<class SBPR_TextWidget> StructureTextWidget;
	TSharedPtr<class SBPR_TextWidget> GraphTextWidget;

	/** Data to be applied */
	TOptional<FBPR_ExtractedData> PendingData;
};
