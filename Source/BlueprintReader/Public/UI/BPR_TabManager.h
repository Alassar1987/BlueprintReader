#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"

class SBPR_TextWidget;

/**
 * SBPR_TabManager
 * Управляет вкладками внутри окна
 */
class BLUEPRINTREADER_API SBPR_TabManager : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBPR_TabManager) {}
	SLATE_END_ARGS()

	/** Конструктор виджета */
	void Construct(const FArguments& InArgs);

private:
	/** TabManager для управления вкладками */
	TSharedPtr<FTabManager> TabManager;

	/** Виджеты для вкладок */
	TSharedPtr<SBPR_TextWidget> StructureWidget;
	TSharedPtr<SBPR_TextWidget> GraphWidget;

	/** Создание вкладок */
	TSharedRef<SDockTab> SpawnStructureTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnGraphTab(const FSpawnTabArgs& Args);
};