#pragma once

#include "CoreMinimal.h"

class SBPR_TabManager;

/**
 * BPR_OutputWindow
 *
 * Класс, который создаёт и открывает отдельное окно Slate.
 * Внутри будет размещён TabManager с вкладками.
 */
class BLUEPRINTREADER_API BPR_OutputWindow
{
public:
	BPR_OutputWindow();
	~BPR_OutputWindow();

	/** Открыть окно */
	void Open();

private:
	/** Ссылка на окно Slate */
	TWeakPtr<SWindow> Window;

	/** Менеджер вкладок, который создаёт вкладки и хранит их виджеты */
	TSharedPtr<SBPR_TabManager> TabManager;
};
