#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "Materials/Material.h"
#include "Components/ActorComponent.h"



class SBPR_WindowOutputMD;


//==============================================================================
//  BPR_Core — центральная логика извлечения данных
//==============================================================================
//
//  Этот класс отвечает за:
//  - определение типа ассета, который пытаемся обработать;
//  - выбор подходящего "экстрактора";
//  - хранение и обновление вывода (Markdown-текст);
//  - передачу данных в окно вывода.
//
//  Core не зависит от UI напрямую, но знает о Slate-виджете вывода
//  через слабую ссылку, чтобы обновлять его при появлении нового текста.
//
//==============================================================================

class BPR_Core
{
public:

    BPR_Core() = default;
    ~BPR_Core() = default;


    //--------------------------------------------------------------------------
    //  Типы поддерживаемых ассетов
    //--------------------------------------------------------------------------

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


public:

    //--------------------------------------------------------------------------
    //  Основная логика
    //--------------------------------------------------------------------------

    /** Проверяет, можем ли мы работать с данным объектом */
    bool IsSupportedAsset(UObject* Object);

    /** Запускает нужный экстрактор в зависимости от типа ассета */
    void ExtractorSelector(UObject* Object);

    /** Устанавливает текст вывода */
    void SetOutputText(const FText& NewText);

    /** Получает текущий текст вывода */
    const FText& GetOutputText() const;

    /** Обработчик кастомного пункта меню (отдельное окно) */
    void OnExtraMenuEntryClicked(UObject* Object);

    /** Регистрирует окно вывода, созданное модулем */
    void RegisterOutputWindow(TSharedPtr<SBPR_WindowOutputMD> InWindow);


private:

    //--------------------------------------------------------------------------
    //  Состояние Core
    //--------------------------------------------------------------------------

    /** Кэшированный тип выбранного ассета */
    EAssetType CachedType = EAssetType::Unknown;

    /** Текст, сгенерированный экстракторами */
    FText OutputText = FText::GetEmpty();

    /** Слабая ссылка на окно вывода Markdown */
    TWeakPtr<SBPR_WindowOutputMD> OutputWindow;
};

