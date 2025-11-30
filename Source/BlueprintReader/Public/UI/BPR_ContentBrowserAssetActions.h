#pragma once

#include "CoreMinimal.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetData.h"

//==============================================================================
//  BPR_ContentBrowserAssetActions
//
//  Только ловит клик и передаёт ассет в модуль.
//==============================================================================

class FBPR_ContentBrowserAssetActions
{
public:
    /** Регистрирует пункт "Show BP as MD" в контекстном меню Content Browser */
    void Register();

    /** Удаляет пункты меню, принадлежащие этому классу */
    void Unregister();

private:
    /** Обработчик клика на пункт "Show BP as MD" */
    void OnShowBPAsMDClicked();

    /** Получает первый выделенный ассет */
    UObject* GetSelectedAsset() const;
};
