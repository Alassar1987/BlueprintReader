#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "UI/BPR_ContentBrowserAssetActions.h"
#include "Widgets/Docking/SDockTab.h"
#endif

#include "Core/BPR_Core.h"

//==============================================================================
//  FBlueprintReaderModule — главный модуль плагина
//==============================================================================
//  
//  Пока переименован только модуль, остальной код и классы оставлены без изменений.
//  Позже можно будет рефакторить Core, ContentBrowserAssetActions и методы вкладки.
//==============================================================================

class FBlueprintReaderModule : public IModuleInterface
{
public:

	//---------------------------------------------------------------------- 
	//  Жизненный цикл модуля
	//---------------------------------------------------------------------- 
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	//---------------------------------------------------------------------- 
	//  Доступ к Core
	//---------------------------------------------------------------------- 
	/** Возвращает экземпляр ядра плагина */
	BPR_Core* GetCoreInstance() const { return CoreInstance; }

#if WITH_EDITOR
	//---------------------------------------------------------------------- 
	//  Вкладка вывода
	//---------------------------------------------------------------------- 
	/** Создаёт вкладку плагина и возвращает её виджет */
	TSharedRef<SDockTab> SpawnBPROutputTab(const FSpawnTabArgs& Args);

	/** Открывает вкладку (если она уже есть — активирует) */
	void OpenBPROutputTab();

private:
	/** Менеджер пользовательских пунктов меню в Content Browser */
	TSharedPtr<FBPR_ContentBrowserAssetActions> ContentBrowserActions;
#endif

private:
	/** Основной объект логики плагина */
	BPR_Core* CoreInstance = nullptr;
};
