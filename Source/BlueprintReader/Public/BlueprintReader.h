#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "Widgets/Docking/SDockTab.h"
#include "UI/BPR_ContentBrowserAssetActions.h"
#endif

#include "Core/BPR_Core.h"

class FBlueprintReaderModule : public IModuleInterface
{
public:
	// Жизненный цикл модуля
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// Доступ к Core
	BPR_Core* GetCoreInstance() const { return CoreInstance; }

#if WITH_EDITOR
	// Спавн и открытие главного окна (контейнера с вкладками)
	TSharedRef<SDockTab> SpawnBPRMainTab(const FSpawnTabArgs& Args);
	void OpenBPRMainTab();
private:
	// Менеджер пунктов контекстного меню Content Browser
	TSharedPtr<FBPR_ContentBrowserAssetActions> ContentBrowserActions;
#endif

private:
	// Основной объект логики плагина
	BPR_Core* CoreInstance = nullptr;
};
