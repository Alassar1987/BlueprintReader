#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"

class SBPR_TabSwitcher;

/**
 * BPR_OutputWindow
 *
 * Простое окно Slate для вывода текста.
 * Внутри размещён SBPR_TabSwitcher с двумя вкладками: Structure и Graph.
 */
class BLUEPRINTREADER_API BPR_OutputWindow
{
public:
	BPR_OutputWindow();
	~BPR_OutputWindow();

	/** Открыть окно */
	void Open();

	/** Получить TabSwitcher для передачи данных из Core */
	TSharedPtr<SBPR_TabSwitcher> GetTabSwitcher() const { return TabSwitcher; }

private:
	/** Ссылка на окно Slate */
	TWeakPtr<SWindow> Window;

	/** TabSwitcher для управления вкладками */
	TSharedPtr<SBPR_TabSwitcher> TabSwitcher;
};
