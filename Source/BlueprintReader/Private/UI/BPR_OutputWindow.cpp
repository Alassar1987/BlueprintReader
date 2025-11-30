#include "UI/BPR_OutputWindow.h"
#include "UI/BPR_TabSwitcher.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"

BPR_OutputWindow::BPR_OutputWindow()
{
	// Конструктор больше не создаёт TabSwitcher через MakeShared
}

BPR_OutputWindow::~BPR_OutputWindow()
{
	// Закрываем окно, если оно существует
	if (TSharedPtr<SWindow> W = Window.Pin())
	{
		W->RequestDestroyWindow();
	}
}

void BPR_OutputWindow::Open(const TOptional<FBPR_ExtractedData>& InitialData)
{
	// Если окно уже открыто — поднимаем его на передний план
	if (Window.IsValid())
	{
		Window.Pin()->BringToFront();
		return;
	}

	// Создаём TabSwitcher и сразу передаём начальные данные
	TSharedRef<SBPR_TabSwitcher> TabSwitcherRef = SNew(SBPR_TabSwitcher)
		.InitialData(InitialData);

	// Создаём новое окно Slate и вставляем в него TabSwitcher
	TSharedRef<SWindow> NewWindow = SNew(SWindow)
		.Title(FText::FromString("BPR Output"))
		.ClientSize(FVector2D(800, 600))
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		[
			TabSwitcherRef
		];

	FSlateApplication::Get().AddWindow(NewWindow);

	// Сохраняем ссылки
	Window = NewWindow;
	TabSwitcher = TabSwitcherRef;

	UE_LOG(LogTemp, Log, TEXT("BPR_OutputWindow: Window opened successfully"));
}
