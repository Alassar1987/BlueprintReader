#include "UI/BPR_ContentBrowserAssetActions.h"
#include "ToolMenus.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "BlueprintReader.h" // модуль плагина

//==============================================================================
// Register
//==============================================================================
void FBPR_ContentBrowserAssetActions::Register()
{
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu");
    FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");

    Section.AddMenuEntry(
        "ShowBPAsMD",
        FText::FromString("Show BP as MD"),
        FText::FromString("Convert Blueprint to Markdown via BPR Core"),
        FSlateIcon(),
        FUIAction(
            FExecuteAction::CreateRaw(this, &FBPR_ContentBrowserAssetActions::OnShowBPAsMDClicked)
        )
    );
}

//==============================================================================
// Unregister
//==============================================================================
void FBPR_ContentBrowserAssetActions::Unregister()
{
    UToolMenus::UnregisterOwner(this);
}

//==============================================================================
// OnShowBPAsMDClicked
//
//  Только передаем выбранный ассет в модуль.
//==============================================================================
void FBPR_ContentBrowserAssetActions::OnShowBPAsMDClicked()
{
#if WITH_EDITOR
    UObject* SelectedObject = GetSelectedAsset();
    if (!SelectedObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("BPR: нет выбранного ассета!"));
        return;
    }

    // Передаём объект в модуль
    FBlueprintReaderModule& Module = 
        FModuleManager::LoadModuleChecked<FBlueprintReaderModule>("BluePrintReader");

    Module.HandleMenuClick(SelectedObject);
#endif
}

//==============================================================================
// GetSelectedAsset
//==============================================================================
UObject* FBPR_ContentBrowserAssetActions::GetSelectedAsset() const
{
#if WITH_EDITOR
    FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

    TArray<FAssetData> SelectedAssets;
    CBModule.Get().GetSelectedAssets(SelectedAssets);

    return SelectedAssets.Num() > 0 ? SelectedAssets[0].GetAsset() : nullptr;
#else
    return nullptr;
#endif
}
