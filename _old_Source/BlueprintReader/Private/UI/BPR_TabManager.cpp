#include "UI/BPR_TabManager.h"
#include "UI/BPR_TextWidget.h"
#include "Core/BPR_Core.h"

#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"

void SBPR_TabManager::OpenWindow(BPR_Core* InCore)
{
	if (!InCore) return;
	Core = InCore;

	if (Window.IsValid())
	{
		Window.Pin()->BringToFront();
		return;
	}

	// Создаём dummy NomadTabSpawner для FTabManager::New
	TSharedRef<FTabSpawner> DummySpawner = FTabSpawner::Create(
		FName("DummySpawner"),
		FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&) { return SNew(SDockTab); })
	);

	// Создаём TabManager
	TabManager = FTabManager::New(nullptr, DummySpawner);

	// Регистрируем вкладки
	TabManager->RegisterTabSpawner("StructureTab",
		FOnSpawnTab::CreateRaw(this, &SBPR_TabManager::SpawnStructureTab))
		.SetDisplayName(FText::FromString("Structure"));

	TabManager->RegisterTabSpawner("GraphTab",
		FOnSpawnTab::CreateRaw(this, &SBPR_TabManager::SpawnGraphTab))
		.SetDisplayName(FText::FromString("Graph"));

	// Layout с двумя вкладками
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

	// Создаём окно
	TSharedRef<SWindow> NewWindow = SNew(SWindow)
		.Title(FText::FromString("Blueprint Markdown"))
		.ClientSize(FVector2D(800, 600))
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		[
			TabManager->RestoreFrom(Layout, TSharedPtr<SWindow>()).ToSharedRef()
		];

	FSlateApplication::Get().AddWindow(NewWindow);
	Window = NewWindow;
}

TSharedRef<SDockTab> SBPR_TabManager::SpawnStructureTab(const FSpawnTabArgs& Args)
{
	StructureWidget = SNew(SBPR_TextWidget);

	if (Core)
	{
		StructureWidget->SetText(Core->GetStructureText());
	}

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.OnCanCloseTab(SDockTab::FCanCloseTab::CreateLambda([]() { return false; }))
		[
			StructureWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> SBPR_TabManager::SpawnGraphTab(const FSpawnTabArgs& Args)
{
	GraphWidget = SNew(SBPR_TextWidget);

	if (Core)
	{
		GraphWidget->SetText(Core->GetGraphText());
	}

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.OnCanCloseTab(SDockTab::FCanCloseTab::CreateLambda([]() { return false; }))
		[
			GraphWidget.ToSharedRef()
		];
}
