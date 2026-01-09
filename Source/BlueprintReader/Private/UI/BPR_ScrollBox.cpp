// Copyright (c) 2026 Racoon Coder. All rights reserved.


#include "UI/BPR_ScrollBox.h"

//==============================================================================
//  Construct
//==============================================================================
void SBPR_ScrollBox::Construct(const FArguments& InArgs)
{
	// Initializing the basic SScrollBox
	SScrollBox::Construct(
		SScrollBox::FArguments()
		.Orientation(InArgs._Orientation)
	);

	// Add content if it is transferred
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
//  CRITICALLY IMPORTANT: If Ctrl is held down, return Unhandled.
// This allows the parent widget (SBPR_TextWidget) to handle the event
// to zoom text. Only if Ctrl is NOT held down, we perform normal scrolling.
//
//==============================================================================
FReply SBPR_ScrollBox::OnMouseWheel(
	const FGeometry& MyGeometry,
	const FPointerEvent& MouseEvent)
{
	// If Ctrl is held down, we DO NOT process it, pass it to the parent
	if (MouseEvent.IsControlDown())
	{
		return FReply::Unhandled();
	}

	// Otherwise - standard ScrollBox scrolling
	return SScrollBox::OnMouseWheel(MyGeometry, MouseEvent);
}