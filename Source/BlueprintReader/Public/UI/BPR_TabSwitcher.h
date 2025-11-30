#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Core/BPR_Core.h" // полный тип FBPR_ExtractedData

class SBPR_TabSwitcher : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBPR_TabSwitcher)
		: _InitialData()
	{}
	SLATE_ARGUMENT(TOptional<FBPR_ExtractedData>, InitialData)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

	/** Устанавливает данные после создания виджета */
	void SetData(const FBPR_ExtractedData& InData);

private:
	FReply OnStructureTabClicked();
	FReply OnGraphTabClicked();
	void SwitchToIndex(int32 Index);

	/** Применяет данные, если виджеты готовы */
	void ApplyPendingData();

private:
	TSharedPtr<SWidgetSwitcher> TabSwitcher;
	TSharedPtr<class SBPR_TextWidget> StructureTextWidget;
	TSharedPtr<class SBPR_TextWidget> GraphTextWidget;

	/** Данные, которые нужно применить */
	TOptional<FBPR_ExtractedData> PendingData;
};
