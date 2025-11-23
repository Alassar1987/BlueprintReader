#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"

class SBPR_TextWidget;
class BPR_Core;

/**
 * SBPR_TabManager
 *
 * Управляет вкладками внутри окна плагина:
 * - Две вкладки: Structure и Graph
 * - Вкладки нельзя закрывать отдельно
 */
class SBPR_TabManager : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBPR_TabManager) {}
	SLATE_END_ARGS()

	/** Создаёт окно с вкладками и TabManager */
	void OpenWindow(BPR_Core* InCore);

private:
	/** Ссылка на окно */
	TWeakPtr<SWindow> Window;

	/** Ссылка на Core */
	BPR_Core* Core = nullptr;

	/** TabManager для управления вкладками */
	TSharedPtr<FTabManager> TabManager;

	/** Виджеты для вкладок */
	TSharedPtr<SBPR_TextWidget> StructureWidget;
	TSharedPtr<SBPR_TextWidget> GraphWidget;

	/** Создание вкладок */
	TSharedRef<SDockTab> SpawnStructureTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnGraphTab(const FSpawnTabArgs& Args);
};
