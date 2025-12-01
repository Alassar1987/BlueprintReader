#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "Materials/Material.h"
#include "Components/ActorComponent.h"

struct FBPR_ExtractedData
{
    FText Structure = FText::FromString(TEXT("No Data found"));
    FText Graph     = FText::FromString(TEXT("No Data found"));
};

class SBPR_TextWidget;

//==============================================================================
//  BPR_Core — центральная логика извлечения данных
//  (минимальная, только для ActorComponent и Actor на первом шаге)
//==============================================================================
class BPR_Core
{
public:

    BPR_Core() = default;
    ~BPR_Core() = default;

    //---------------------------------------------------------------------- 
    //  Типы поддерживаемых ассетов
    //---------------------------------------------------------------------- 
    enum class EAssetType : uint8
    {
        Unknown,
        Actor,
        Material,
        MaterialInstance,
        MaterialFunction,
        MaterialFunctionInstance,
        ActorComponent,
        Enum,
        Structure,
        InterfaceBP,
        Widget,
        StaticMesh
    };

    //---------------------------------------------------------------------- 
    //  Основная логика
    //---------------------------------------------------------------------- 
    bool IsSupportedAsset(UObject* Object);
    void ExtractorSelector(UObject* Object);
    void OnExtraMenuEntryClicked(UObject* Object);

    // --- Доступ к данным для UI ---
    const FBPR_ExtractedData& GetTextData() const { return TextData; }

private:

    //---------------------------------------------------------------------- 
    //  Состояние Core
    //---------------------------------------------------------------------- 
    EAssetType CachedType = EAssetType::Unknown;

    TWeakPtr<SBPR_TextWidget> OutputWindow;
    TWeakPtr<SBPR_TextWidget> StructureTab;
    TWeakPtr<SBPR_TextWidget> GraphTab;

    FBPR_ExtractedData TextData;
};
