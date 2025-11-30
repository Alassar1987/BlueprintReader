#include "BlueprintReader.h"
#include "Core/BPR_Core.h"
#include "UI/BPR_OutputWindow.h"
#include "UI/BPR_ContentBrowserAssetActions.h"
#include "UI/BPR_TabSwitcher.h"

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
	CoreInstance = new BPR_Core();

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
		delete CoreInstance;
		CoreInstance = nullptr;
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
// Вызывается из FBPR_ContentBrowserAssetActions при клике по ассету
#if WITH_EDITOR
void FBlueprintReaderModule::HandleMenuClick(UObject* SelectedObject)
{
	if (!SelectedObject || !CoreInstance || !OutputWindow.IsValid())
		return;

	// Проверяем, поддерживается ли ассет
	if (!CoreInstance->IsSupportedAsset(SelectedObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("BPR: выбранный ассет не поддерживается!"));
		return;
	}

	// Запускаем экстрактор, Core наполняет FBPR_ExtractedData
	CoreInstance->ExtractorSelector(SelectedObject);

	// Передаем данные в TabSwitcher окна
	if (TSharedPtr<SBPR_TabSwitcher> TabSwitcher = OutputWindow->GetTabSwitcher())
	{
		TabSwitcher->SetData(CoreInstance->GetTextData());
	}

	// Открываем окно, если оно ещё не открыто
	OutputWindow->Open();
}
#endif

//==============================================================================
// Регистрация модуля
//==============================================================================
IMPLEMENT_MODULE(FBlueprintReaderModule, BlueprintReader);
