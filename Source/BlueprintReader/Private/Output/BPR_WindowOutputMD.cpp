#include "Output/BPR_WindowOutputMD.h"

#include "UI/BPR_ScrollBox.h"
#include "Styling/CoreStyle.h"
#include "Fonts/SlateFontInfo.h"
#include "Logging/LogMacros.h"


//==============================================================================
//  Construct
//==============================================================================
//
//  Создаёт окно вывода и настраивает виджеты:
//  - кастомный ScrollBox с отключением скролла при Ctrl;
//  - многострочный текстовый блок с авто-переносом;
//  - установка начального текста.
//
//==============================================================================

void SBPR_WindowOutputMD::Construct(const FArguments& InArgs)
{
    const FText InitialText = InArgs._InitialText;

    BaseFontSize = 12;

    ChildSlot
    [
        SAssignNew(ScrollBox, SBPR_ScrollBox)
        .Orientation(EOrientation::Orient_Vertical)
        [
            SAssignNew(OutputTextBlock, SMultiLineEditableText)
            .Text(InitialText)
            .Font(FSlateFontInfo(
                FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"),
                BaseFontSize
            ))
            .IsReadOnly(true)
            .AutoWrapText(true)
        ]
    ];

    UpdateFont();
}



//==============================================================================
//  SetText
//==============================================================================

void SBPR_WindowOutputMD::SetText(const FText& NewText)
{
    if (OutputTextBlock.IsValid())
    {
        OutputTextBlock->SetText(NewText);
        UE_LOG(LogTemp, Display, TEXT("WindowOutputMD: Text updated"));
    }
}



//==============================================================================
//  OnMouseWheel
//==============================================================================
//
//  Ctrl + колесо — увеличивает/уменьшает размер шрифта.
//  Иначе — скроллим содержимое через ScrollBox.
//
//==============================================================================

FReply SBPR_WindowOutputMD::OnMouseWheel(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent)
{
    // Масштабирование текста
    if (MouseEvent.IsControlDown())
    {
        TextScale = FMath::Clamp(
            TextScale + MouseEvent.GetWheelDelta() * 0.1f,
            0.5f,
            3.0f
        );

        UpdateFont();
        return FReply::Handled();
    }

    // Обычный скролл
    if (ScrollBox.IsValid())
    {
        return ScrollBox->OnMouseWheel(MyGeometry, MouseEvent);
    }

    return FReply::Unhandled();
}



//==============================================================================
//  UpdateFont
//==============================================================================
//
//  Обновляет размер шрифта исходя из TextScale.
//  Изменения отображаются сразу.
//
//==============================================================================

void SBPR_WindowOutputMD::UpdateFont()
{
    if (!OutputTextBlock.IsValid())
        return;

    FSlateFontInfo FontInfo = OutputTextBlock->GetFont();
    FontInfo.Size = FMath::RoundToInt(BaseFontSize * TextScale);

    OutputTextBlock->SetFont(FontInfo);
}
