#include "BlueprintReader.h"

#include "Core/BPR_Core.h"
#include "UI/BPR_ContentBrowserAssetActions.h"
#include "UI/BPR_TabManager.h"          // твой новый виджет-менеджер вкладок

#include "Modules/ModuleManager.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define BPR_MAIN_TAB_NAME TEXT("BPR_MainTab")

//==============================================================================
// StartupModule
//==============================================================================
void FBlueprintReaderModule::StartupModule()
{
#if WITH_EDITOR
	// Создаём ядро плагина
	CoreInstance = new BPR_Core();

	// Регистрируем пункт в Content Browser
	ContentBrowserActions = MakeShared<FBPR_ContentBrowserAssetActions>();
	ContentBrowserActions->Register();

	// Регистрируем единый NomadTab — главный контейнер для внутренних вкладок
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName(BPR_MAIN_TAB_NAME),
		FOnSpawnTab::CreateRaw(this, &FBlueprintReaderModule::SpawnBPRMainTab)
	)
	.SetDisplayName(FText::FromString("BlueprintReader"))
	.SetMenuType(ETabSpawnerMenuType::Hidden);

#endif
}

//==============================================================================
// ShutdownModule
//==============================================================================
void FBlueprintReaderModule::ShutdownModule()
{
#if WITH_EDITOR
	// Удаляем Core
	if (CoreInstance)
	{
		delete CoreInstance;
		CoreInstance = nullptr;
	}

	// Убираем пункты меню
	if (ContentBrowserActions.IsValid())
	{
		ContentBrowserActions->Unregister();
		ContentBrowserActions.Reset();
	}

	// Снимаем регистрацию основного таба
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName(BPR_MAIN_TAB_NAME));
#endif
}

//==============================================================================
// Spawn главного окна (внутри него создаётся TabManager с двумя вкладками)
//==============================================================================
TSharedRef<SDockTab> FBlueprintReaderModule::SpawnBPRMainTab(const FSpawnTabArgs& Args)
{
	// Создаём твой главный виджет, который внутри создаёт две вкладки
	TSharedRef<SBPR_TabManager> MainWidget = SNew(SBPR_TabManager);

	// При необходимости можно передать ссылку на Core внутрь TabManager:
	// MainWidget->SetCoreInstance(CoreInstance);
	// (Реализуй метод в TabManager, если захочешь)

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			MainWidget
		];
}

//==============================================================================
// Открыть/активировать главное окно
//==============================================================================
void FBlueprintReaderModule::OpenBPRMainTab()
{
#if WITH_EDITOR
	FGlobalTabmanager::Get()->TryInvokeTab(FName(BPR_MAIN_TAB_NAME));
#endif
}

//==============================================================================
// Регистрация модуля
//==============================================================================
IMPLEMENT_MODULE(FBlueprintReaderModule, BlueprintReader);
