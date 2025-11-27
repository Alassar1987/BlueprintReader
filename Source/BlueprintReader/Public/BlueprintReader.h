#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "UI/BPR_ContentBrowserAssetActions.h"
#endif

class BPR_Core;
class BPR_OutputWindow;

class FBlueprintReaderModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Доступ к Core */
	BPR_Core* GetCoreInstance() const { return CoreInstance; }

	/** Доступ к окну */
	TSharedPtr<BPR_OutputWindow> GetOutputWindow() const { return OutputWindow; }

#if WITH_EDITOR
	/** Открыть окно вручную (если нужно) */
	void OpenOutputWindow();
#endif

private:
	/** Логика */
	BPR_Core* CoreInstance = nullptr;

	/** Окно результата */
	TSharedPtr<BPR_OutputWindow> OutputWindow;

#if WITH_EDITOR
	/** Регистрируем контекстное меню в Content Browser */
	TSharedPtr<FBPR_ContentBrowserAssetActions> ContentBrowserActions;
#endif
};
