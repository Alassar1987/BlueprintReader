#include "UI/BPR_TabSwitcher.h"
#include "UI/BPR_TextWidget.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"

void SBPR_TabSwitcher::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SVerticalBox)

        // Панель кнопок вкладок
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(5.0f)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(FText::FromString("Structure"))
                .OnClicked(this, &SBPR_TabSwitcher::OnStructureTabClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2.0f)
            [
                SNew(SButton)
                .Text(FText::FromString("Graph"))
                .OnClicked(this, &SBPR_TabSwitcher::OnGraphTabClicked)
            ]
        ]

        // Переключатель панелей
        + SVerticalBox::Slot()
        .FillHeight(1.f)
        [
            SAssignNew(TabSwitcher, SWidgetSwitcher)

            + SWidgetSwitcher::Slot()
            [
                SAssignNew(StructureTextWidget, SBPR_TextWidget)
            ]

            + SWidgetSwitcher::Slot()
            [
                SAssignNew(GraphTextWidget, SBPR_TextWidget)
            ]
        ]
    ];

    SwitchToIndex(0);
    UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Constructed successfully"));
}

void SBPR_TabSwitcher::SetData(const FBPR_ExtractedData& InData)
{
    // Сохраняем данные
    PendingData = InData;
    
    UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Data received (Structure: %d chars, Graph: %d chars), scheduling application"),
        InData.Structure.ToString().Len(),
        InData.Graph.ToString().Len());
    
    // Регистрируем таймер на следующий кадр
    // 0.f = задержка перед первым вызовом (сразу на следующем тике)
    RegisterActiveTimer(0.f, 
        FWidgetActiveTimerDelegate::CreateSP(this, &SBPR_TabSwitcher::ApplyDataOnNextTick));
}

EActiveTimerReturnType SBPR_TabSwitcher::ApplyDataOnNextTick(double InCurrentTime, float InDeltaTime)
{
    if (!PendingData.IsSet())
    {
        UE_LOG(LogTemp, Warning, TEXT("BPR_TabSwitcher: No pending data to apply"));
        return EActiveTimerReturnType::Stop;
    }

    // Проверяем готовность виджетов
    if (!StructureTextWidget.IsValid() || !GraphTextWidget.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("BPR_TabSwitcher: Widgets not ready yet, will retry next tick"));
        // Продолжаем тикать до готовности (максимум несколько кадров)
        return EActiveTimerReturnType::Continue;
    }

    // Виджеты готовы, применяем данные
    UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Applying data NOW"));

    StructureTextWidget->SetText(PendingData->Structure);
    GraphTextWidget->SetText(PendingData->Graph);
    
    PendingData.Reset();
    
    // Останавливаем таймер
    return EActiveTimerReturnType::Stop;
}

FReply SBPR_TabSwitcher::OnStructureTabClicked()
{
    UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Structure tab clicked"));
    SwitchToIndex(0);
    return FReply::Handled();
}

FReply SBPR_TabSwitcher::OnGraphTabClicked()
{
    UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Graph tab clicked"));
    SwitchToIndex(1);
    return FReply::Handled();
}

void SBPR_TabSwitcher::SwitchToIndex(int32 Index)
{
    if (TabSwitcher.IsValid())
    {
        TabSwitcher->SetActiveWidgetIndex(Index);
        UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Switched to tab %d"), Index);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BPR_TabSwitcher: Cannot switch - TabSwitcher is invalid!"));
    }
}