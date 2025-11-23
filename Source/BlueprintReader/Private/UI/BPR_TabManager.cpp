#include "UI/BPR_TabManager.h"
#include "UI/BPR_TextWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"

void SBPR_TabManager::Construct(const FArguments& InArgs)
{
    // Для standalone окна создаём "легковесный" layout без полноценного TabManager
    // Используем простой TabManager со Stack виджетом
    
    // Регистрируем spawner'ы через глобальный TabManager
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner("StructureTab",
        FOnSpawnTab::CreateSP(this, &SBPR_TabManager::SpawnStructureTab))
        .SetDisplayName(FText::FromString("Structure"));

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner("GraphTab",
        FOnSpawnTab::CreateSP(this, &SBPR_TabManager::SpawnGraphTab))
        .SetDisplayName(FText::FromString("Graph"));

    // Создаём layout для отображения
    TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("BPR_MainLayout")
        ->AddArea(
            FTabManager::NewPrimaryArea()
            ->SetOrientation(Orient_Horizontal)
            ->Split(
                FTabManager::NewStack()
                ->AddTab("StructureTab", ETabState::OpenedTab)
                ->AddTab("GraphTab", ETabState::OpenedTab)
                ->SetForegroundTab(FTabId("StructureTab"))
            )
        );

    // Получаем TabManager от глобального менеджера
    TabManager = FGlobalTabmanager::Get();

    // Размещаем восстановленный layout в виджет
    ChildSlot
    [
        TabManager->RestoreFrom(Layout, TSharedPtr<SWindow>()).ToSharedRef()
    ];
}

TSharedRef<SDockTab> SBPR_TabManager::SpawnStructureTab(const FSpawnTabArgs& Args)
{
    StructureWidget = SNew(SBPR_TextWidget);

    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)  // Важно: NomadTab для независимого окна
        .OnCanCloseTab(SDockTab::FCanCloseTab::CreateLambda([]() { return false; }))
        [
            StructureWidget.ToSharedRef()
        ];
}

TSharedRef<SDockTab> SBPR_TabManager::SpawnGraphTab(const FSpawnTabArgs& Args)
{
    GraphWidget = SNew(SBPR_TextWidget);

    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)  // Важно: NomadTab для независимого окна
        .OnCanCloseTab(SDockTab::FCanCloseTab::CreateLambda([]() { return false; }))
        [
            GraphWidget.ToSharedRef()
        ];
}