#pragma once

#include "CoreMinimal.h"
#include "Core/BPR_Core.h"

class UActorComponent;
class UBlueprint;
class UEdGraph;
class UEdGraphNode;
class UEdGraphPin;

/**
 * Экстрактор для ActorComponent Blueprint.
 * Собирает переменные, функции и макросы компонента.
 * Результат возвращается в Core через FText.
 */
class BLUEPRINTREADER_API BPR_Extractor_ActorComponent
{
public:
	
	BPR_Extractor_ActorComponent();
	~BPR_Extractor_ActorComponent();
	// -------------------------------
	// Главная точка входа
	// -------------------------------
	/** Обрабатывает выбранный объект (Blueprint), формирует структуру и граф */
	void ProcessComponent(UObject* SelectedObject, FBPR_ExtractedData& OutData);

private:

	// -------------------------------
	// Логирование
	// -------------------------------
	// Простые методы для вывода сообщений разного уровня
	void LogMessage(const FString& Msg);
	void LogWarning(const FString& Msg);
	void LogError(const FString& Msg);

	// -------------------------------
	// Работа со структурой Blueprint
	// -------------------------------
	/** Общая информация о Blueprint (родительский класс, репликация) */
	void AppendBlueprintInfo(UBlueprint* Blueprint, FString& OutText);

	/** Сбор пользовательских переменных, включая тип, дефолтное значение, флаги, категорию, описание */
	void AppendVariables(UBlueprint* Blueprint, FString& OutText);

	/** Возвращает дефолтное значение свойства */
	FString GetPropertyDefaultValue(FProperty* Property, UObject* Object);

	/** Возвращает детализированный тип свойства, включая TMap/TSet/TArray и enum */
	FString GetPropertyTypeDetailed(FProperty* Property);

	/** Обрабатывает поля структурных свойств и выводит их информацию */
	void AppendStructFields(FStructProperty* StructProp, FString& OutText, int32 Indent = 0);

	/** Информация о реплицируемых переменных */
	void AppendReplicationInfo(UClass* Class, FString& OutText);

	// -------------------------------
	// Работа с графами (Event/Function/Macro)
	// -------------------------------
	/** Основная точка обхода всех графов Blueprint */
	void AppendGraphs(UBlueprint* Blueprint, FString& OutText);

	/** Обход последовательности узлов графа (exec + data flow) */
	void AppendGraphSequence(UEdGraph* Graph, FString& OutExecText, FString& OutDataText);

	/** Рекурсивная обработка узлов графа для exec- и data-flow */
	void ProcessNodeSequence(UEdGraphNode* Node, int32 IndentLevel, TSet<UEdGraphNode*>& Visited, FString& OutExecText, FString& OutDataText);

	/** Сбор подписи функции по входным/выходным пинам */
	FString GetFunctionSignature(UEdGraph* Graph);

	/** Сбор подписи макроса по Tunnel-узлам */
	FString GetMacroSignature(UEdGraph* Graph);

	/** Формирует читаемое имя узла, учитывая тип узла и параметры */
	FString GetReadableNodeName(UEdGraphNode* Node);

	/** Детали пина (значение по умолчанию или ссылка на другой узел) */
	FString GetPinDetails(UEdGraphPin* Pin);

	/** Читабельное имя пина для вывода */
	FString GetPinDisplayName(UEdGraphPin* Pin);

	// -------------------------------
	// Вспомогательные методы
	// -------------------------------
	/** Очищает имя от хвоста GUID, если он есть */
	FString CleanName(const FString& RawName);

	/** Определяет, является ли свойство пользовательской переменной (не системной) */
	bool IsUserVariable(FProperty* Property);
	
	/** Проверяет, является ли нода вычислительной (data-flow), без exec */
	bool IsComputationalNode(UEdGraphNode* Node);

	/** Проверяет, есть ли у ноды Exec-вход */
	bool HasExecInput(UEdGraphNode* Node);

};