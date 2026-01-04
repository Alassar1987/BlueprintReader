#include "BlueprintReader.h"
#include "Core/BPR_Core.h"
#include "UI/BPR_OutputWindow.h"
#include "UI/BPR_ContentBrowserAssetActions.h"
#include "UI/BPR_TabSwitcher.h"
#include "UI/BPR_InfoWindow.h"

#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "Framework/Application/SlateApplication.h"
#endif

//==============================================================================
// StartupModule
//==============================================================================
void FBlueprintReaderModule::StartupModule()
{
	// Создаём Core
	CoreInstance = MakeUnique<BPR_Core>();

#if WITH_EDITOR
	// Создаём окно вывода с TabSwitcher
	OutputWindow = MakeShared<BPR_OutputWindow>();

	// Регистрируем действия в Content Browser
	ContentBrowserActions = MakeShared<FBPR_ContentBrowserAssetActions>();
	ContentBrowserActions->Register();
#endif
}

//==============================================================================
// ShutdownModule
//==============================================================================
void FBlueprintReaderModule::ShutdownModule()
{
#if WITH_EDITOR
	if (ContentBrowserActions.IsValid())
	{
		ContentBrowserActions->Unregister();
		ContentBrowserActions.Reset();
	}

	OutputWindow.Reset();
#endif

	if (CoreInstance)
	{
		CoreInstance.Reset();
	}
}

//==============================================================================
// OpenOutputWindow
//==============================================================================
#if WITH_EDITOR
void FBlueprintReaderModule::OpenOutputWindow()
{
	if (OutputWindow.IsValid())
	{
		OutputWindow->Open();
	}
}
#endif

//==============================================================================
// HandleMenuClick
//==============================================================================
#if WITH_EDITOR
void FBlueprintReaderModule::HandleMenuClick(UObject* SelectedObject)
{
	if (!SelectedObject || !CoreInstance.IsValid() || !OutputWindow.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("BPR: Invalid state - Object=%d, Core=%d, Window=%d"), 
			SelectedObject != nullptr, CoreInstance != nullptr, OutputWindow.IsValid());
		return;
	}

	// Проверяем, поддерживается ли ассет
	if (!CoreInstance->IsSupportedAsset(SelectedObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("BPR: The selected asset is not supported!"));

		InfoWindow = MakeShared<BPR_InfoWindow>();
		InfoWindow->Open({
			FText::FromString("Blueprint Reader"),                                    // Title
			FText::FromString("Warning! This asset is not supported yet!"),           // Main warning
			FText::FromString(
				"The Blueprint Reader plugin cannot display this asset type in the current version.\n"
				"This is not an engine error, it's just support not yet implemented."), // SubMessage
			FString("https://www.google.com"),                                        // URL
			FText::FromString("Check for updates")                                    // Button text
		});

		return;
	}



	// ВАЖНО: Сначала открываем окно и даём виджетам инициализироваться
	OutputWindow->Open();

	// Получаем TabSwitcher ПОСЛЕ открытия окна
	TSharedPtr<SBPR_TabSwitcher> TabSwitcher = OutputWindow->GetTabSwitcher();
	if (!TabSwitcher.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("BPR: TabSwitcher is invalid after opening window!"));
		return;
	}

	// Запускаем экстрактор
	CoreInstance->ExtractorSelector(SelectedObject);

	// Получаем данные из Core
	const FBPR_ExtractedData& ExtractedData = CoreInstance->GetTextData();

	// Логируем для отладки
	UE_LOG(LogTemp, Log, TEXT("BPR: Setting data - Structure length: %d, Graph length: %d"),
		ExtractedData.Structure.ToString().Len(),
		ExtractedData.Graph.ToString().Len());

	// Передаем данные в TabSwitcher
	TabSwitcher->SetData(ExtractedData);
	
	UE_LOG(LogTemp, Log, TEXT("BPR: Data successfully set to TabSwitcher"));
}
#endif

//==============================================================================
// Регистрация модуля
//==============================================================================
IMPLEMENT_MODULE(FBlueprintReaderModule, BlueprintReader);