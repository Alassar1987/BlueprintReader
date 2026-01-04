#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"



/**
 * Simple information window to notify the user
 * about unsupported assets or other messages.
 */
class BPR_InfoWindow
{
public:
	/** Параметры окна */
	struct FParams
	{
		FText Title;
		FText Message;           // General warning, in large print
		FText SubMessage;        // Detailed Explanation
		FString OptionalURL;     // URL for the "Check for updates" button
		FText OptionalButtonText; // Button text for URL
	};

	BPR_InfoWindow() = default;
	~BPR_InfoWindow();

	/** Creates and opens a window with the specified parameters */
	void Open(const FParams& Params);

private:
	/** Link to Slate window */
	TWeakPtr<SWindow> Window;

	/** OK button handler */
	FReply OnOkClicked();

	/** URL Button Handler */
	FReply OnUrlClicked(const FString& Url);
};
