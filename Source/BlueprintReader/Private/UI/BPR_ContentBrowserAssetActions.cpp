#include "UI/BPR_ContentBrowserAssetActions.h"

#include "ToolMenus.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "BlueprintReader.h"            // модуль плагина → доступ к Core
#include "Core/BPR_Core.h"



//==============================================================================
//  Register
//==============================================================================
//
//  Добавляет пункт "Show BP as MD" в меню ContentBrowser.AssetContextMenu.
//  Пункт всегда виден, проверка ассетов производится позже — в Core.
//
//==============================================================================

void FBPR_ContentBrowserAssetActions::Register()
{
    UToolMenu* Menu =
        UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu");

    FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");

    Section.AddMenuEntry(
        "ShowBPAsMD",
        FText::FromString("Show BP as MD"),
        FText::FromString("Convert Blueprint to Markdown via BPR Core"),
        FSlateIcon(),
        FUIAction(
            FExecuteAction::CreateRaw(
                this,
                &FBPR_ContentBrowserAssetActions::OnShowBPAsMDClicked
            )
        )
    );
}



//==============================================================================
//  Unregister
//==============================================================================
//
//  Удаляет пункты меню, созданные этим объектом.
//
//==============================================================================

void FBPR_ContentBrowserAssetActions::Unregister()
{
    UToolMenus::UnregisterOwner(this);
}



//==============================================================================
//  OnShowBPAsMDClicked
//==============================================================================
//
//  Основная логика обработки выбора пользователя:
//
//  1. Получаем выделенный ассет.
//  2. Получаем Core из модуля BPR.
//  3. Проверяем поддержку ассета.
//  4. Открываем окно и генерируем Markdown.
//
//==============================================================================

void FBPR_ContentBrowserAssetActions::OnShowBPAsMDClicked()
{
#if WITH_EDITOR

    UObject* SelectedObject = GetSelectedAsset();

    if (!SelectedObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("BPR: нет выбранного ассета для обработки!"));
        return;
    }

    // Получаем Core из модуля
    FBlueprintReaderModule & Module =
        FModuleManager::LoadModuleChecked<FBlueprintReaderModule>("BluePrintReader");

    if (BPR_Core* Core = Module.GetCoreInstance())
    {
        if (Core->IsSupportedAsset(SelectedObject))
        {
            Core->OnExtraMenuEntryClicked(SelectedObject);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("BPR: выбранный ассет не поддерживается!"));
        }
    }

#endif

    UE_LOG(LogTemp, Log, TEXT("BPR: Show BP as MD triggered."));
}



//==============================================================================
//  GetSelectedAsset
//==============================================================================
//
//  Возвращает первый выделенный ассет из Content Browser.
//  Если ничего не выделено — возвращает nullptr.
//
//==============================================================================

UObject* FBPR_ContentBrowserAssetActions::GetSelectedAsset() const
{
#if WITH_EDITOR

    FContentBrowserModule& CBModule =
        FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

    TArray<FAssetData> SelectedAssets;
    CBModule.Get().GetSelectedAssets(SelectedAssets);

    if (SelectedAssets.Num() > 0)
    {
        return SelectedAssets[0].GetAsset();
    }

#endif

    return nullptr;
}
