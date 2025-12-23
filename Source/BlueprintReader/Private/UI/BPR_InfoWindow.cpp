#include "UI/BPR_InfoWindow.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"

BPR_InfoWindow::~BPR_InfoWindow()
{
    if (TSharedPtr<SWindow> W = Window.Pin())
    {
        W->RequestDestroyWindow();
    }
}

void BPR_InfoWindow::Open(const FParams& Params)
{
    // Если окно уже открыто, поднимаем его на передний план
    if (Window.IsValid())
    {
        Window.Pin()->BringToFront();
        return;
    }

    // Создаём новое окно
    TSharedRef<SWindow> NewWindow = SNew(SWindow)
        .Title(Params.Title)
        .ClientSize(FVector2D(600, 300))
        .SupportsMaximize(false)
        .SupportsMinimize(false)
        .IsTopmostWindow(true)
        [
            SNew(SBorder)
            .Padding(10)
            [
                SNew(SVerticalBox)

                // 1️⃣ Основной warning текст
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 10)
                [
                    SNew(STextBlock)
                    .Text(Params.Message)
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16)) // рабочий шрифт
                    .ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
                    .Justification(ETextJustify::Center)
                ]

                // 2️⃣ Подробное пояснение
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(20, 5)
                [
                    SNew(STextBlock)
                    .Text(Params.SubMessage)
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)) // рабочий шрифт
                    .ColorAndOpacity(FSlateColor(FLinearColor::White))
                    .AutoWrapText(true)
                ]

                // 3️⃣ Кнопка "Check for updates"
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(0, 20)
                [
                    SNew(SButton)
                    .Text(Params.OptionalButtonText.IsEmpty() ? FText::FromString("Check for updates") : Params.OptionalButtonText)
                    .Visibility(Params.OptionalURL.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
                    .OnClicked_Lambda([this, Url = Params.OptionalURL]()
                    {
                        if (!Url.IsEmpty())
                        {
                            FPlatformProcess::LaunchURL(*Url, nullptr, nullptr);
                        }
                        return FReply::Handled();
                    })
                ]

                // 4️⃣ Подсказка под кнопкой
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(0, 5)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("View the roadmap and learn about plans"))
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 14)) // рабочий шрифт
                    .ColorAndOpacity(FSlateColor(FLinearColor::Gray))
                    .Justification(ETextJustify::Center)
                ]

                // 5️⃣ Кнопка OK
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Right)
                .Padding(0, 20, 20, 10)
                [
                    SNew(SButton)
                    .Text(FText::FromString("OK"))
                    .OnClicked_Lambda([this]()
                    {
                        if (TSharedPtr<SWindow> W = Window.Pin())
                        {
                            W->RequestDestroyWindow();
                        }
                        return FReply::Handled();
                    })
                ]
            ]
        ];

    // Добавляем окно в Slate
    FSlateApplication::Get().AddWindow(NewWindow);
    Window = NewWindow;
}


// Закрываем окно при нажатии OK
FReply BPR_InfoWindow::OnOkClicked()
{
    if (TSharedPtr<SWindow> W = Window.Pin())
    {
        W->RequestDestroyWindow();
    }
    return FReply::Handled();
}

// Открываем браузер при нажатии на кнопку URL
FReply BPR_InfoWindow::OnUrlClicked(const FString& Url)
{
    if (!Url.IsEmpty())
    {
        FPlatformProcess::LaunchURL(*Url, nullptr, nullptr);
    }
    return FReply::Handled();
}
