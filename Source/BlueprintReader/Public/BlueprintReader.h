#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "UI/BPR_ContentBrowserAssetActions.h"
#endif

class BPR_Core;
class BPR_OutputWindow;
class BPR_InfoWindow;

class FBlueprintReaderModule : public IModuleInterface
{
public:
	// Жизненный цикл модуля
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Доступ к Core */
	BPR_Core* GetCoreInstance() const { return CoreInstance; }

	/** Доступ к окну */
	TSharedPtr<BPR_OutputWindow> GetOutputWindow() const { return OutputWindow; }

#if WITH_EDITOR
	/** Открыть окно вручную */
	void OpenOutputWindow();

	/** Обрабатывает клик по пункту контекстного меню */
	void HandleMenuClick(UObject* SelectedObject);
#endif

private:
	/** Центральная логика плагина */
	BPR_Core* CoreInstance = nullptr;

	/** Окно вывода с TabSwitcher */
	TSharedPtr<BPR_OutputWindow> OutputWindow;
	TSharedPtr<BPR_InfoWindow> InfoWindow;

#if WITH_EDITOR
	/** Контекстное меню в Content Browser */
	TSharedPtr<FBPR_ContentBrowserAssetActions> ContentBrowserActions;
#endif
};
