#include "BlueprintReader.h"

#include "Core/BPR_Core.h"
#include "Output/BPR_WindowOutputMD.h"
#include "UI/BPR_ContentBrowserAssetActions.h"

#include "Modules/ModuleManager.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

//==============================================================================
//  StartupModule
//==============================================================================

void FBlueprintReaderModule::StartupModule()
{
#if WITH_EDITOR

    // Создаём ядро плагина
    CoreInstance = new BPR_Core();

    // Регистрируем действия в контекстном меню Content Browser
    ContentBrowserActions = MakeShared<FBPR_ContentBrowserAssetActions>();
    ContentBrowserActions->Register();

    // Регистрируем вкладку (NomadTab) для отображения вывода
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        FName("BPROutputTab"),
        FOnSpawnTab::CreateRaw(this, &FBlueprintReaderModule::SpawnBPROutputTab)
    )
    .SetDisplayName(FText::FromString("BPR Output"))
    .SetMenuType(ETabSpawnerMenuType::Hidden);

#endif
}

//==============================================================================
//  ShutdownModule
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

    // Убираем действия из Content Browser
    if (ContentBrowserActions.IsValid())
    {
        ContentBrowserActions->Unregister();
        ContentBrowserActions.Reset();
    }

    // Снимаем регистрацию вкладки
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("BPROutputTab"));

#endif
}

//==============================================================================
//  SpawnBPROutputTab
//==============================================================================

TSharedRef<SDockTab> FBlueprintReaderModule::SpawnBPROutputTab(const FSpawnTabArgs& Args)
{
    // Создаём окно
    TSharedRef<SBPR_WindowOutputMD> OutputWindow = SNew(SBPR_WindowOutputMD);

    // Сообщаем Core, куда выводить Markdown
    if (CoreInstance)
    {
        CoreInstance->RegisterOutputWindow(OutputWindow);
    }

    // Создаём вкладку и вкладываем в неё окно
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            OutputWindow
        ];
}

//==============================================================================
//  OpenBPROutputTab
//==============================================================================

void FBlueprintReaderModule::OpenBPROutputTab()
{
#if WITH_EDITOR
    FGlobalTabmanager::Get()->TryInvokeTab(FName("BPROutputTab"));
#endif
}

//==============================================================================
//  Регистрация модуля
//==============================================================================

IMPLEMENT_MODULE(FBlueprintReaderModule, BlueprintReader);
