#include "Core/BPR_Core.h"

#include "UI/BPR_TextWidget.h"
#include "Extractors/BPR_Extractor_Actor.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Blueprint/UserWidget.h"
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


//==============================================================================
//  IsSupportedAsset
//==============================================================================
//
//  Проверяет объект и определяет, можем ли мы обработать его.
//  Результат записывается в CachedType.
//
//==============================================================================

bool BPR_Core::IsSupportedAsset(UObject* Object)
{
    CachedType = EAssetType::Unknown;
    if (!Object)
        return false;

    // Blueprint-based assets
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

    // Non-Blueprint assets
    if (Cast<UMaterial>(Object))
    {
        CachedType = EAssetType::Material;
        return true;
    }

    if (Cast<UMaterialInstance>(Object))
    {
        CachedType = EAssetType::MaterialInstance;
        return true;
    }
    
    if (Cast<UMaterialFunction>(Object))
    {
        CachedType = EAssetType::MaterialFunction;
        return true;
    }
    
    if (Cast<UMaterialFunctionInstance>(Object))
    {
        CachedType = EAssetType::MaterialFunctionInstance;
        return true;
    }
    
    if (Cast<UEnum>(Object))
    {
        CachedType = EAssetType::Enum;
        return true;
    }
    
    if (Cast<UScriptStruct>(Object))
    {
        CachedType = EAssetType::Structure;
        return true;
    }
    
    if (Cast<UBlueprint>(Object))
    {
        CachedType = EAssetType::InterfaceBP;
        return true;
    }
    
    if (Cast<UStaticMesh>(Object))
    {
        CachedType = EAssetType::StaticMesh;
        return true;
    }



    return false;
}



//==============================================================================
//  ExtractorSelector
//==============================================================================
//
//  Запускает конкретный экстрактор в зависимости от определённого типа ассета.
//  Если поддержки нет — показывает заглушку.
//
//==============================================================================

void BPR_Core::ExtractorSelector(UObject* Object)
{
    switch (CachedType)
    {
    // case EAssetType::Actor:
    //     {
    //         BPR_Extractor_Actor Extractor;
    //         FText GeneratedText;
    //         Extractor.ProcessBlueprint(Object, GeneratedText);
    //         SetOutputText(GeneratedText);
    //         break;
    //     }
    //
    // case EAssetType::Material:
    //     {
    //         BPR_Extractor_Material Extractor;
    //         FText GeneratedText;
    //         Extractor.ProcessMaterial(Object, GeneratedText);
    //         SetOutputText(GeneratedText);
    //         break;
    //     }

    case EAssetType::ActorComponent:
        {
            BPR_Extractor_ActorComponent Extractor;
            FText GeneratedText;
            Extractor.ProcessComponent(Object, TextData); // предполагаемый метод
            //SetOutputText(GeneratedText);
            break;
        }
        
    // case EAssetType::MaterialInstance:
    //     {
    //         BPR_Extractor_MaterialInstance Extractor;
    //         FText GeneratedText;
    //         Extractor.ProcessMaterialInstance(Object, GeneratedText);
    //         SetOutputText(GeneratedText);
    //         break;
    //     }
    //     
    // case EAssetType::MaterialFunction:
    //     {
    //         BPR_Extractor_MaterialFunction Extractor;
    //         FText GeneratedText;
    //         Extractor.ProcessMaterialFunction(Object, GeneratedText);
    //         SetOutputText(GeneratedText);
    //         break;
    //     }
    //
    // case EAssetType::MaterialFunctionInstance:
    //     {
    //         BPR_Extractor_MFInstance Extractor;
    //         FText GeneratedText;
    //         Extractor.ProcessMFInstance(Object, GeneratedText);
    //         SetOutputText(GeneratedText);
    //         break;
    //     }
    //     
    // case EAssetType::Enum:
    //     {
    //         BPR_Extractor_Enum Extractor;
    //         FText GeneratedText;
    //         Extractor.ProcessEnum(Object, GeneratedText);
    //         SetOutputText(GeneratedText);
    //         break;
    //     }
    //     
    // case EAssetType::Structure:
    //     {
    //         BPR_Extractor_Structure Extractor;
    //         FText GeneratedText;
    //         Extractor.ProcessStructure(Object, GeneratedText); // метод, который ты реализовал
    //         SetOutputText(GeneratedText);
    //         break;
    //     }
    //     
    // case EAssetType::InterfaceBP:
    //     {
    //         BPR_Extractor_InterfaceBP Extractor;
    //         FText GeneratedText;
    //         Extractor.ProcessInterfaceBP(Object, GeneratedText); // метод, который реализуешь в экстракторе
    //         SetOutputText(GeneratedText);
    //         break;
    //     }
    //
    // case EAssetType::StaticMesh:
    //     {
    //         BPR_Extractor_StaticMesh Extractor;
    //         FText GeneratedText;
    //         Extractor.ProcessStaticMesh(Object, GeneratedText); // метод из болванки
    //         SetOutputText(GeneratedText);
    //         break;
    //     }
    //
    // case EAssetType::Widget:
    //     {
    //         BPR_Extractor_Widget Extractor;
    //         FText GeneratedText;
    //         Extractor.ProcessWidget(Object, GeneratedText); // предполагаемый метод
    //         SetOutputText(GeneratedText);
    //         break;
    //     }

    default:
        //SetOutputText(FText::FromString("No suitable extractor found."));
        break;
    }
}



//==============================================================================
//  SetOutputText
//==============================================================================
//
//  Сохраняет текст и обновляет окно вывода, если оно существует.
//
//==============================================================================


// void BPR_Core::SetOutputText(const FText& NewText)
// {
//     OutputText = NewText;
//
//     if (TSharedPtr<SBPR_TextWidget> Window = OutputWindow.Pin())
//     {
//         Window->SetText(OutputText);
//     }
// }


//==============================================================================
//  GetOutputText
//==============================================================================
//
//  Возвращает текущий текст вывода.
//
//==============================================================================

// const FText& BPR_Core::GetOutputText() const
// {
//     return OutputText;
// }

//==============================================================================
//  Новый блок для текста
//==============================================================================
//
//  
//
//==============================================================================

// void BPR_Core::SetStructureText(const FText& NewText)
// {
//     StructureText = NewText;
//     if (TSharedPtr<SBPR_TextWidget> Tab = StructureTab.Pin())
//     {
//         Tab->SetText(StructureText);
//     }
// }
//
// const FText& BPR_Core::GetStructureText() const
// {
//     return StructureText;
// }
//
// void BPR_Core::SetGraphText(const FText& NewText)
// {
//     GraphText = NewText;
//     if (TSharedPtr<SBPR_TextWidget> Tab = GraphTab.Pin())
//     {
//         Tab->SetText(GraphText);
//     }
// }
//
// const FText& BPR_Core::GetGraphText() const
// {
//     return GraphText;
// }


//==============================================================================
//  OnExtraMenuEntryClicked
//==============================================================================
//
//  Альтернативный путь запуска экстрактора.
//  Здесь создаётся отдельное плавающее окно, если вкладки ещё нет.
//
//==============================================================================

void BPR_Core::OnExtraMenuEntryClicked(UObject* Object)
{
    if (!Object)
        return;

    FText GeneratedText;

    switch (CachedType)
    {
    // case EAssetType::Actor:
    //     {
    //         BPR_Extractor_Actor Extractor;
    //         Extractor.ProcessBlueprint(Object, GeneratedText);
    //         break;
    //     }
        
    case EAssetType::ActorComponent:
        {
            BPR_Extractor_ActorComponent Extractor;
            Extractor.ProcessComponent(Object, TextData);
            break;
        }

    // case EAssetType::Material:
    //     {
    //         BPR_Extractor_Material Extractor;
    //         Extractor.ProcessMaterial(Object, GeneratedText);
    //         break;
    //     }
    //     
    // case EAssetType::MaterialInstance:
    //     {
    //         BPR_Extractor_MaterialInstance Extractor;
    //         Extractor.ProcessMaterialInstance(Object, GeneratedText);
    //         break;
    //     }
    //     
    // case EAssetType::MaterialFunction:
    //     {
    //         BPR_Extractor_MaterialFunction Extractor;
    //         Extractor.ProcessMaterialFunction(Object, GeneratedText);
    //         break;
    //     }
    //
    // case EAssetType::MaterialFunctionInstance:
    //     {
    //         BPR_Extractor_MFInstance Extractor;
    //         Extractor.ProcessMFInstance(Object, GeneratedText);
    //         break;
    //     }
    //     
    // case EAssetType::Enum:
    //     {
    //         BPR_Extractor_Enum Extractor;
    //         Extractor.ProcessEnum(Object, GeneratedText);
    //         break;
    //     }
    //     
    // case EAssetType::Structure:
    //     {
    //         BPR_Extractor_Structure Extractor;
    //         Extractor.ProcessStructure(Object, GeneratedText);
    //         break;
    //     }
    //     
    // case EAssetType::InterfaceBP:
    //     {
    //         BPR_Extractor_InterfaceBP Extractor;
    //         Extractor.ProcessInterfaceBP(Object, GeneratedText);
    //         break;
    //     }
    //
    // case EAssetType::StaticMesh:
    //     {
    //         BPR_Extractor_StaticMesh Extractor;
    //         Extractor.ProcessStaticMesh(Object, GeneratedText);
    //         break;
    //     }
    //
    //
    // case EAssetType::Widget:
    //     {
    //         BPR_Extractor_Widget Extractor;
    //         Extractor.ProcessWidget(Object, GeneratedText);
    //         break;
    //     }
    //     
    


    default:
        GeneratedText = FText::FromString("No suitable extractor found.");
        break;
    }

//    SetOutputText(GeneratedText);

    // // Создание окна остаётся без изменений
    // if (!OutputWindow.IsValid())
    // {
    //     TSharedRef<SBPR_TextWidget> WindowWidget = SNew(SBPR_TextWidget);
    //     WindowWidget->SetText(OutputText);
    //
    //     TSharedRef<SWindow> Window = SNew(SWindow)
    //         .Title(FText::FromString("BPR Output"))
    //         .ClientSize(FVector2D(600, 400))
    //         .SupportsMinimize(true)
    //         .SupportsMaximize(true)
    //         [
    //             WindowWidget
    //         ];
    //
    //     FSlateApplication::Get().AddWindow(Window);
    //
    //     OutputWindow = WindowWidget;
    // }
    // else
    // {
    //     OutputWindow.Pin()->SetText(OutputText);
    // }
}



//==============================================================================
//  RegisterOutputWindow
//==============================================================================
//
//  Регистрирует окно вывода, которое создаёт модуль (NomadTab).
//  Нужно для интеграции с основной UI-системой плагина.
//
//==============================================================================

// void BPR_Core::RegisterOutputWindow(TSharedPtr<SBPR_TextWidget> InWindow)
// {
//     OutputWindow = InWindow;
// }
//==============================================================================
//  Регистрация вкладок
//==============================================================================
//
//  
//
//==============================================================================

// void BPR_Core::RegisterStructureTab(TSharedPtr<SBPR_TextWidget> InTab)
// {
//     StructureTab = InTab;
//     // Сразу выставляем текущий текст
//     if (StructureTab.IsValid())
//         StructureTab.Pin()->SetText(StructureText);
// }

// void BPR_Core::RegisterGraphTab(TSharedPtr<SBPR_TextWidget> InTab)
// {
//     GraphTab = InTab;
//     if (GraphTab.IsValid())
//         GraphTab.Pin()->SetText(GraphText);
// }
