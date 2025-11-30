#include "UI/BPR_ScrollBox.h"

//==============================================================================
//  Construct
//==============================================================================
void SBPR_ScrollBox::Construct(const FArguments& InArgs)
{
	// Инициализируем базовый SScrollBox
	SScrollBox::Construct(
		SScrollBox::FArguments()
		.Orientation(InArgs._Orientation)
	);

	// Добавляем контент, если он передан
	AddSlot()
		.Padding(0)
		[
			InArgs._Content.Widget
		];
}

//==============================================================================
//  OnMouseWheel
//==============================================================================
//
//  КРИТИЧЕСКИ ВАЖНО: Если Ctrl зажат, возвращаем Unhandled.
//  Это позволяет родительскому виджету (SBPR_TextWidget) обработать событие
//  для зума текста. Только если Ctrl НЕ зажат - выполняем обычную прокрутку.
//
//==============================================================================
FReply SBPR_ScrollBox::OnMouseWheel(
	const FGeometry& MyGeometry,
	const FPointerEvent& MouseEvent)
{
	// Если Ctrl зажат - НЕ обрабатываем, передаем родителю
	if (MouseEvent.IsControlDown())
	{
		return FReply::Unhandled();
	}

	// Иначе - стандартная прокрутка ScrollBox
	return SScrollBox::OnMouseWheel(MyGeometry, MouseEvent);
}