#include "UI/BPR_TabSwitcher.h"
#include "UI/BPR_TextWidget.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h" 
#include "Core/BPR_Core.h" 


void SBPR_TabSwitcher::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SVerticalBox)

        // --- Панель кнопок вкладок ---
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .Text(FText::FromString("Structure"))
                .OnClicked(this, &SBPR_TabSwitcher::OnStructureTabClicked)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .Text(FText::FromString("Graph"))
                .OnClicked(this, &SBPR_TabSwitcher::OnGraphTabClicked)
            ]
        ]

        // --- Переключатель панелей ---
        + SVerticalBox::Slot()
        .FillHeight(1.f)
        [
            SAssignNew(TabSwitcher, SWidgetSwitcher)

            // Вкладка Structure
            + SWidgetSwitcher::Slot()
            [
                SAssignNew(StructureTextWidget, SBPR_TextWidget)
            ]

            // Вкладка Graph
            + SWidgetSwitcher::Slot()
            [
                SAssignNew(GraphTextWidget, SBPR_TextWidget)
            ]
        ]
    ];

    // По умолчанию показываем первую вкладку
    SwitchToIndex(0);
}

void SBPR_TabSwitcher::SetData(const FBPR_ExtractedData& InData)
{
    if (StructureTextWidget.IsValid())
    {
        StructureTextWidget->SetText(InData.Structure);
    }

    if (GraphTextWidget.IsValid())
    {
        GraphTextWidget->SetText(InData.Graph);
    }
}

FReply SBPR_TabSwitcher::OnStructureTabClicked()
{
    SwitchToIndex(0);
    return FReply::Handled();
}

FReply SBPR_TabSwitcher::OnGraphTabClicked()
{
    SwitchToIndex(1);
    return FReply::Handled();
}

void SBPR_TabSwitcher::SwitchToIndex(int32 Index)
{
    if (TabSwitcher.IsValid())
    {
        TabSwitcher->SetActiveWidgetIndex(Index);
    }
}
