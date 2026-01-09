// Copyright (c) 2026 Racoon Coder. All rights reserved.

#pragma once

//==============================================================================
//  SBPR_ScrollBox
//==============================================================================
//
//  A wrapper over the standard SScrollBox.
//
// Differences from a regular widget:
// - Content is set via SLATE_DEFAULT_SLOT;
// - Added behavior: if the user holds Ctrl, the mouse wheel
// does not cause scrolling - the event is passed higher (for example, for zoom).
//
//==============================================================================

#include "Widgets/Layout/SScrollBox.h"

class SBPR_ScrollBox : public SScrollBox
{
public:

	SLATE_BEGIN_ARGS(SBPR_ScrollBox)
		: _Orientation(Orient_Vertical)
	{}
		
	/** Scroll direction */
	SLATE_ARGUMENT(EOrientation, Orientation)

	/** Main content of scrollbox */
	SLATE_DEFAULT_SLOT(FArguments, Content)
SLATE_END_ARGS()


//--------------------------------------------------------------------------
//  Designing a Slate widget
//--------------------------------------------------------------------------
void Construct(const FArguments& InArgs);


protected:

	//--------------------------------------------------------------------------
	//  Input behavior
	//--------------------------------------------------------------------------

	/** 
	* If Ctrl is held down, the wheel event is not processed (we return Unhandled). 
	* This allows the parent to use the mouse wheel for other purposes, such as zoom. 
	*/
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry,
								const FPointerEvent& MouseEvent) override;
};

