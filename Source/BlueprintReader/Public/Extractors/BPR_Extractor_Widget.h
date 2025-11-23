#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

/**
 * Экстрактор для UUserWidget
 * Собирает свойства виджета и его дочерних компонентов в Markdown.
 */
class BLUEPRINTREADER_API BPR_Extractor_Widget
{
public:
	BPR_Extractor_Widget();
	~BPR_Extractor_Widget();

	/** Главная точка входа */
	void ProcessWidget(UObject* SelectedObject, FText& OutText);

private:
	void LogMessage(const FString& Msg);
	void LogWarning(const FString& Msg);
	void LogError(const FString& Msg);

	/** Рекурсивная обработка виджета и всех дочерних элементов */
	void AppendWidgetProperties(UWidget* Widget, FString& OutText, int32 Depth = 0);

	/** Преобразуем конкретный виджет в читаемое описание */
	FString GetWidgetDescription(UWidget* Widget);
};
