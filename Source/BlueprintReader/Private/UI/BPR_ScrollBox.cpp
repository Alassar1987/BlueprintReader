#include "UI/BPR_ScrollBox.h"

//==============================================================================
//  Construct
//==============================================================================
//
//  Создаёт SScrollBox и добавляет в него переданный внешний контент.
//  Эта реализация повторяет поведение родительского виджета, но позволяет
//  задать контент через SLATE_DEFAULT_SLOT.
//
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
//  Блокирует прокрутку, если зажат Ctrl, чтобы колесо можно было использовать,
//  например, для изменения масштаба.
//
//==============================================================================

FReply SBPR_ScrollBox::OnMouseWheel(
	const FGeometry& MyGeometry,
	const FPointerEvent& MouseEvent)
{
	// Если Ctrl зажат — скролл отключён
	if (MouseEvent.IsControlDown())
	{
		return FReply::Unhandled();
	}

	// В остальных случаях — стандартная прокрутка
	return SScrollBox::OnMouseWheel(MyGeometry, MouseEvent);
}
