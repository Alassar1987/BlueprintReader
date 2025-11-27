#include "BlueprintReader.h"

#include "Core/BPR_Core.h"
#include "UI/BPR_OutputWindow.h"
#include "UI/BPR_ContentBrowserAssetActions.h"

#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "Framework/Application/SlateApplication.h"
#endif

void FBlueprintReaderModule::StartupModule()
{
	// Создаём Core
	CoreInstance = new BPR_Core();

#if WITH_EDITOR
	// Создаём окно результатов
	OutputWindow = MakeShared<BPR_OutputWindow>();

	// Регистрируем действия в Content Browser
	ContentBrowserActions = MakeShared<FBPR_ContentBrowserAssetActions>();
	ContentBrowserActions->Register();
#endif
}

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

	// Удаляем Core
	if (CoreInstance)
	{
		delete CoreInstance;
		CoreInstance = nullptr;
	}
}

#if WITH_EDITOR
void FBlueprintReaderModule::OpenOutputWindow()
{
	if (OutputWindow.IsValid())
	{
		OutputWindow->Open();
	}
}
#endif

IMPLEMENT_MODULE(FBlueprintReaderModule, BlueprintReader);
