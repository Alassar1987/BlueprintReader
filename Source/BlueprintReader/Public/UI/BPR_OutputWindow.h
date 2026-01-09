// Copyright (c) 2026 Racoon Coder. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "core/BPR_Core.h"

struct FBPR_ExtractedData;
class SBPR_TabSwitcher;

/**
* BPR_OutputWindow 
* 
* Simple Slate window for text output. 
* Inside there is SBPR_TabSwitcher with two tabs: Structure and Graph.
*/
class BLUEPRINTREADER_API BPR_OutputWindow
{
public:
	BPR_OutputWindow();
	~BPR_OutputWindow();

	/** Open a window with optional initial data */
	void Open(const TOptional<FBPR_ExtractedData>& InitialData = TOptional<FBPR_ExtractedData>());

	/** Get TabSwitcher to transfer data from Core */
	TSharedPtr<SBPR_TabSwitcher> GetTabSwitcher() const { return TabSwitcher; }

private:
	/** Link to Slate window */
	TWeakPtr<SWindow> Window;

	/** TabSwitcher for managing tabs */
	TSharedPtr<SBPR_TabSwitcher> TabSwitcher;
};
