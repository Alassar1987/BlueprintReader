#pragma once

#include "CoreMinimal.h"
#include "ToolMenus.h"
#include "AssetRegistry/AssetData.h"

//==============================================================================
//  BPR_ContentBrowserAssetActions
//
//  It only catches the click and transfers the asset to the module.
//==============================================================================

class FBPR_ContentBrowserAssetActions
{
public:
    /** Registers the "Show BP as MD" item in the Content Browser context menu */
    void Register();

    /** Removes menu items belonging to this class */
    void Unregister();

private:
    /** Click handler for the item "Show BP as MD" */
    void OnShowBPAsMDClicked();

    /** Gets the first selected asset */
    UObject* GetSelectedAsset() const;
};
