#pragma once

//==============================================================================
//  SBPR_WindowOutputMD
//==============================================================================
//
//  Главное окно вывода Markdown-текста.
//  Функции:
//  - отображение многострочного текста;
//  - масштабирование текста с помощью Ctrl + колесо мыши;
//  - прокрутка через кастомный ScrollBox.
//
//==============================================================================

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/SMultiLineEditableText.h"


class SBPR_WindowOutputMD : public SCompoundWidget
{
public:

    SLATE_BEGIN_ARGS(SBPR_WindowOutputMD)
        : _InitialText(FText::FromString("Hello UE 5.7 RichText!"))
    {}
        /** Текст, отображаемый после создания окна */
        SLATE_ARGUMENT(FText, InitialText)
    SLATE_END_ARGS()


    //--------------------------------------------------------------------------
    //  Конструирование
    //--------------------------------------------------------------------------
    void Construct(const FArguments& InArgs);

    /** Обновляет отображаемый текст */
    void SetText(const FText& NewText);


protected:

    //--------------------------------------------------------------------------
    //  Управление вводом
    //--------------------------------------------------------------------------

    /**
     * Ctrl + колесо мыши — масштабирует текст.
     * Обычное колесо — передаём в ScrollBox.
     */
    virtual FReply OnMouseWheel(const FGeometry& MyGeometry,
                                const FPointerEvent& MouseEvent) override;


private:

    //--------------------------------------------------------------------------
    //  Внутренние данные
    //--------------------------------------------------------------------------

    /** Многострочный текстовый блок */
    TSharedPtr<SMultiLineEditableText> OutputTextBlock;

    /** ScrollBox для прокрутки */
    TSharedPtr<SScrollBox> ScrollBox;

    /** Базовый размер шрифта */
    int32 BaseFontSize = 12;

    /** Масштаб текста (1.0 = нормальный размер) */
    float TextScale = 1.0f;


    //--------------------------------------------------------------------------
    //  Внутренние функции
    //--------------------------------------------------------------------------

    /** Применяет текущий масштаб к OutputTextBlock */
    void UpdateFont();
};
