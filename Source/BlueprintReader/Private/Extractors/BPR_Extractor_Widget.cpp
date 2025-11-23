#include "Extractors/BPR_Extractor_Widget.h"
#include "Engine/Engine.h"

BPR_Extractor_Widget::BPR_Extractor_Widget() {}
BPR_Extractor_Widget::~BPR_Extractor_Widget() {}

void BPR_Extractor_Widget::ProcessWidget(UObject* SelectedObject, FText& OutText)
{
#if WITH_EDITOR
    if (!SelectedObject)
    {
        LogError(TEXT("SelectedObject is null!"));
        OutText = FText::FromString("Error: SelectedObject is null.");
        return;
    }

    UUserWidget* Widget = nullptr;

    // Если это Blueprint UserWidget, берем DefaultObject
    if (UBlueprint* WidgetBP = Cast<UBlueprint>(SelectedObject))
    {
        if (WidgetBP->GeneratedClass)
        {
            Widget = Cast<UUserWidget>(WidgetBP->GeneratedClass->GetDefaultObject());
        }
    }
    else
    {
        // Если это уже инстанс
        Widget = Cast<UUserWidget>(SelectedObject);
    }

    if (!Widget)
    {
        LogWarning(TEXT("Selected object is not a UserWidget."));
        OutText = FText::FromString("Warning: Selected object is not a UserWidget.");
        return;
    }

    FString TmpText = FString::Printf(TEXT("# Widget Export: %s\n\n"), *Widget->GetName());

    if (Widget->WidgetTree && Widget->WidgetTree->RootWidget)
    {
        AppendWidgetProperties(Widget->WidgetTree->RootWidget, TmpText);
    }
    else
    {
        TmpText += TEXT("No RootWidget found.\n");
    }

    OutText = FText::FromString(TmpText);
#endif
}


//--------------------
// Логирование
//--------------------
void BPR_Extractor_Widget::LogMessage(const FString& Msg) { UE_LOG(LogTemp, Log, TEXT("[BPRBPR_Extractor_Widget] %s"), *Msg); }
void BPR_Extractor_Widget::LogWarning(const FString& Msg) { UE_LOG(LogTemp, Warning, TEXT("[BPRBPR_Extractor_Widget] %s"), *Msg); }
void BPR_Extractor_Widget::LogError(const FString& Msg) { UE_LOG(LogTemp, Error, TEXT("[BPRBPR_Extractor_Widget] %s"), *Msg); }

//--------------------
// Рекурсивный обход виджетов
//--------------------
void BPR_Extractor_Widget::AppendWidgetProperties(UWidget* Widget, FString& OutText, int32 Depth)
{
#if WITH_EDITOR
    if (!Widget) return;

    // Markdown отступы
    FString Indent = FString::ChrN(Depth * 2, TEXT('-'));

    OutText += FString::Printf(TEXT("%s %s\n"), *Indent, *GetWidgetDescription(Widget));

    // Если это контейнер, обходим детей
    if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
    {
        for (int32 i = 0; i < Panel->GetChildrenCount(); ++i)
        {
            UWidget* Child = Panel->GetChildAt(i);
            AppendWidgetProperties(Child, OutText, Depth + 1);
        }
    }
#endif
}

//--------------------
// Получение описания виджета
//--------------------
FString BPR_Extractor_Widget::GetWidgetDescription(UWidget* Widget)
{
    if (!Widget) return TEXT("None");

    FString Desc = Widget->GetName(); // дефолтное имя

#if WITH_EDITOR
    Desc = Widget->GetClass()->GetName();

    if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
    {
        Desc += FString::Printf(TEXT(" (Text: %s)"), *TextBlock->GetText().ToString());
    }
    else if (UImage* Image = Cast<UImage>(Widget))
    {
        Desc += FString::Printf(TEXT(" (Brush: %s)"), Image->Brush.GetResourceObject() ? *Image->Brush.GetResourceObject()->GetName() : TEXT("None"));
    }
#endif

    return Desc;
}

