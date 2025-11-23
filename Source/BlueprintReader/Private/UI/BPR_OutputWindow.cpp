#include "UI/BPR_OutputWindow.h"
#include "UI/BPR_TabManager.h"
#include "UI/BPR_TextWidget.h"

#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"

BPR_OutputWindow::BPR_OutputWindow()
{
	// Создаём менеджер вкладок
	TabManager = MakeShared<SBPR_TabManager>();
}

BPR_OutputWindow::~BPR_OutputWindow()
{
	// При необходимости можно закрывать окно
	if (TSharedPtr<SWindow> W = Window.Pin())
	{
		W->RequestDestroyWindow();
	}
}

void BPR_OutputWindow::Open()
{
	if (Window.IsValid())
	{
		Window.Pin()->BringToFront();
		return;
	}

	// Создаём окно Slate
	TSharedRef<SWindow> NewWindow = SNew(SWindow)
		.Title(FText::FromString("BPR Output"))
		.ClientSize(FVector2D(800, 600))
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		[
			TabManager->AsShared() // вставляем TabManager прямо в окно
		];

	FSlateApplication::Get().AddWindow(NewWindow);

	Window = NewWindow;
}
