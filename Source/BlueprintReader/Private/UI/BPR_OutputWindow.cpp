#include "UI/BPR_OutputWindow.h"
#include "UI/BPR_TabSwitcher.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"

BPR_OutputWindow::BPR_OutputWindow()
{
	// Создаём TabSwitcher
	TabSwitcher = MakeShared<SBPR_TabSwitcher>();
}

BPR_OutputWindow::~BPR_OutputWindow()
{
	// Закрываем окно, если оно существует
	if (TSharedPtr<SWindow> W = Window.Pin())
	{
		W->RequestDestroyWindow();
	}
}

void BPR_OutputWindow::Open()
{
	// Если окно уже открыто — поднимаем его на передний план
	if (Window.IsValid())
	{
		Window.Pin()->BringToFront();
		return;
	}

	// Создаём новое окно Slate
	TSharedRef<SWindow> NewWindow = SNew(SWindow)
		.Title(FText::FromString("BPR Output"))
		.ClientSize(FVector2D(800, 600))
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		[
			// Вставляем TabSwitcher прямо в окно
			TabSwitcher->AsShared()
		];

	FSlateApplication::Get().AddWindow(NewWindow);
	Window = NewWindow;

	UE_LOG(LogTemp, Log, TEXT("BPR_OutputWindow: Window opened successfully"));
    
	// ВАЖНО: Не нужно вручную вызывать TryApplyPendingData!
	// TabSwitcher сам применит данные через RegisterActiveTimer в SetData()
}