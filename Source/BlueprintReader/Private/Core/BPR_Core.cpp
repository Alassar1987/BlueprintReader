#include "Core/BPR_Core.h"

#include "UI/BPR_TextWidget.h"
#include "Extractors/BPR_Extractor_Actor.h"
#include "Extractors/BPR_Extractor_ActorComponent.h"
#include "Extractors/BPR_Extractor_Material.h"
#include "Extractors/BPR_Extractor_MaterialInstance.h"
#include "Extractors/BPR_Extractor_MaterialFunction.h"
#include "Extractors/BPR_Extractor_MFInstance.h"
#include "Extractors/BPR_Extractor_Widget.h"
#include "Extractors/BPR_Extractor_Enum.h"
#include "Extractors/BPR_Extractor_Structure.h"
#include "Extractors/BPR_Extractor_InterfaceBP.h"
#include "Extractors/BPR_Extractor_StaticMesh.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"

//==============================================================================
//  IsSupportedAsset
//==============================================================================
bool BPR_Core::IsSupportedAsset(UObject* Object)
{
    CachedType = EAssetType::Unknown;
    if (!Object) return false;

    if (UBlueprint* BP = Cast<UBlueprint>(Object))
    {
        if (BP->GeneratedClass)
        {
            if (BP->GeneratedClass->IsChildOf(AActor::StaticClass()))
            {
                CachedType = EAssetType::Actor;
                return true;
            }
            else if (BP->GeneratedClass->IsChildOf(UActorComponent::StaticClass()))
            {
                CachedType = EAssetType::ActorComponent;
                return true;
            }
            else if (BP->GeneratedClass->IsChildOf(UUserWidget::StaticClass()))
            {
                CachedType = EAssetType::Widget;
                return true;
            }
        }
    }

    if (Cast<UMaterial>(Object)) { CachedType = EAssetType::Material; return true; }
    if (Cast<UMaterialInstance>(Object)) { CachedType = EAssetType::MaterialInstance; return true; }
    if (Cast<UMaterialFunction>(Object)) { CachedType = EAssetType::MaterialFunction; return true; }
    if (Cast<UMaterialFunctionInstance>(Object)) { CachedType = EAssetType::MaterialFunctionInstance; return true; }
    if (Cast<UEnum>(Object)) { CachedType = EAssetType::Enum; return true; }
    if (Cast<UScriptStruct>(Object)) { CachedType = EAssetType::Structure; return true; }
    if (Cast<UBlueprint>(Object)) { CachedType = EAssetType::InterfaceBP; return true; }
    if (Cast<UStaticMesh>(Object)) { CachedType = EAssetType::StaticMesh; return true; }

    return false;
}

//==============================================================================
//  ExtractorSelector
//==============================================================================
void BPR_Core::ExtractorSelector(UObject* Object)
{
    switch (CachedType)
    {
    case EAssetType::ActorComponent:
        {
            BPR_Extractor_ActorComponent Extractor;
            Extractor.ProcessComponent(Object, TextData);
            break;
        }

    // Остальные экстракторы пока закомментированы
    // case EAssetType::Actor: ...
    // case EAssetType::Material: ...
    // case EAssetType::Widget: ...
    default:
        break;
    }
}

//==============================================================================
//  OnExtraMenuEntryClicked
//==============================================================================
void BPR_Core::OnExtraMenuEntryClicked(UObject* Object)
{
    if (!Object) return;

    switch (CachedType)
    {
    case EAssetType::ActorComponent:
        {
            BPR_Extractor_ActorComponent Extractor;
            Extractor.ProcessComponent(Object, TextData);
            break;
        }

    default:
        TextData.Structure = FText::FromString("No suitable extractor found.");
        TextData.Graph     = FText::FromString("No suitable extractor found.");
        break;
    }

    // Старое окно закомментировано
    // if (!OutputWindow.IsValid()) ...
}

//==============================================================================
//  Регистрация окон и вкладок (закомментировано)
//==============================================================================
// void BPR_Core::RegisterOutputWindow(TSharedPtr<SBPR_TextWidget> InWindow) { OutputWindow = InWindow; }
// void BPR_Core::RegisterStructureTab(TSharedPtr<SBPR_TextWidget> InTab) { StructureTab = InTab; if (StructureTab.IsValid()) StructureTab.Pin()->SetText(StructureText); }
// void BPR_Core::RegisterGraphTab(TSharedPtr<SBPR_TextWidget> InTab) { GraphTab = InTab; if (GraphTab.IsValid()) GraphTab.Pin()->SetText(GraphText); }
