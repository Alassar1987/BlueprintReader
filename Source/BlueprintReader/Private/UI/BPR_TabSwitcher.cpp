#include "UI/BPR_TabSwitcher.h"
#include "UI/BPR_TextWidget.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"

void SBPR_TabSwitcher::Construct(const FArguments& InArgs)
{
	UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Construct starting"));

	if (InArgs._InitialData.IsSet())
	{
		PendingData = InArgs._InitialData.GetValue();
		UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Initial data provided"));
	}

	ChildSlot
	[
		SNew(SVerticalBox)

		// Панель кнопок вкладок
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SButton)
				.Text(FText::FromString("Structure"))
				.OnClicked(this, &SBPR_TabSwitcher::OnStructureTabClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2)
			[
				SNew(SButton)
				.Text(FText::FromString("Graph"))
				.OnClicked(this, &SBPR_TabSwitcher::OnGraphTabClicked)
			]
		]

		// Контент вкладок
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

	UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Construct finished"));
	UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: StructureTextWidget valid = %d"), StructureTextWidget.IsValid());
	UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: GraphTextWidget valid = %d"), GraphTextWidget.IsValid());

	if (PendingData.IsSet())
	{
		ApplyPendingData();
	}
}

void SBPR_TabSwitcher::SetData(const FBPR_ExtractedData& InData)
{
	PendingData = InData;

	UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: SetData called"));
	UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Data received - Structure: %d chars, Graph: %d chars"),
		InData.Structure.ToString().Len(),
		InData.Graph.ToString().Len());

	if (StructureTextWidget.IsValid() && GraphTextWidget.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Widgets ready, applying immediately"));
		ApplyPendingData();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BPR_TabSwitcher: Widgets not ready, data will be applied после Construct"));
	}
}

void SBPR_TabSwitcher::ApplyPendingData()
{
	if (!PendingData.IsSet())
	{
		UE_LOG(LogTemp, Warning, TEXT("BPR_TabSwitcher: ApplyPendingData called but no data to apply"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Applying pending data NOW"));

	if (StructureTextWidget.IsValid())
		StructureTextWidget->SetText(PendingData->Structure);
	if (GraphTextWidget.IsValid())
		GraphTextWidget->SetText(PendingData->Graph);

	PendingData.Reset();

	UE_LOG(LogTemp, Log, TEXT("BPR_TabSwitcher: Data applied successfully"));
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
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BPR_TabSwitcher: Cannot switch - TabSwitcher is invalid"));
	}
}
