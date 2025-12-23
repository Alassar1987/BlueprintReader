#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"



/**
 * Простое информационное окно для уведомления пользователя
 * о неподдерживаемых ассетах или других сообщениях.
 */
class BPR_InfoWindow
{
public:
	/** Параметры окна */
	struct FParams
	{
		FText Title;
		FText Message;           // Основное предупреждение, крупным шрифтом
		FText SubMessage;        // Подробное пояснение
		FString OptionalURL;     // URL для кнопки "Check for updates"
		FText OptionalButtonText; // Текст кнопки для URL
	};

	BPR_InfoWindow() = default;
	~BPR_InfoWindow();

	/** Создаёт и открывает окно с заданными параметрами */
	void Open(const FParams& Params);

private:
	/** Ссылка на окно Slate */
	TWeakPtr<SWindow> Window;

	/** Обработчик кнопки ОК */
	FReply OnOkClicked();

	/** Обработчик кнопки URL */
	FReply OnUrlClicked(const FString& Url);
};
