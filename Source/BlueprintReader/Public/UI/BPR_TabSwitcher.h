#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

struct FBPR_ExtractedData;

class SBPR_TabSwitcher : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBPR_TabSwitcher) {}
	SLATE_END_ARGS()

	/** Конструктор виджета */
	void Construct(const FArguments& InArgs);

	/** Передаём данные из Core */
	void SetData(const FBPR_ExtractedData& InData);

private:
	/** Обработчики кликов по вкладкам */
	FReply OnStructureTabClicked();
	FReply OnGraphTabClicked();

	/** Переключение активной панели */
	void SwitchToIndex(int32 Index);

private:
	/** Переключатель панелей */
	TSharedPtr<SWidgetSwitcher> TabSwitcher;

	/** Текстовые виджеты для каждой вкладки */
	TSharedPtr<class SBPR_TextWidget> StructureTextWidget;
	TSharedPtr<class SBPR_TextWidget> GraphTextWidget;
};
