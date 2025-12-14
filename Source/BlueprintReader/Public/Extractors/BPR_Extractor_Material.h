#pragma once

#include "CoreMinimal.h"
#include "Core/BPR_Core.h"

class UMaterial;
class UMaterialInstance;
class UMaterialExpression;

/**
 * Экстрактор для Material и MaterialInstance.
 * Преобразует Material Graph в текстовое представление:
 *  - структуру материала
 *  - параметры
 *  - граф зависимостей от Material Outputs
 */
class BLUEPRINTREADER_API BPR_Extractor_Material
{
public:

    BPR_Extractor_Material();
    ~BPR_Extractor_Material();

    // -------------------------------
    // Главная точка входа
    // -------------------------------
    /** Обрабатывает выбранный объект (Material или MaterialInstance) */
    void ProcessMaterial(UObject* SelectedObject, FBPR_ExtractedData& OutData);

private:

    // -------------------------------
    // Логирование
    // -------------------------------
    void LogMessage(const FString& Msg);
    void LogWarning(const FString& Msg);
    void LogError(const FString& Msg);

    // -------------------------------
    // Структура материала (декларативная часть)
    // -------------------------------
    /** Общая информация о материале (Domain, BlendMode, ShadingModel и т.д.) */
    void AppendMaterialInfo(UMaterial* Material, FString& OutText);

    /** Параметры материала (Scalar / Vector / Texture / Static Switch) */
    void AppendMaterialParameters(UMaterial* Material, FString& OutText);

    /** Информация о MaterialInstance и его родителе */
    void AppendMaterialInstanceInfo(UMaterialInstance* Instance, FString& OutText);

    /** Переопределённые параметры инстанса */
    void AppendMaterialInstanceOverrides(UMaterialInstance* Instance, FString& OutText);

    // -------------------------------
    // Граф материала (вычислительная часть)
    // -------------------------------
    /** Точка входа обхода графа — все Material Outputs */
    void AppendMaterialGraph(UMaterial* Material, FString& OutText);

    /** Обрабатывает конкретный Material Output (BaseColor, Normal, etc.) */
    void AppendMaterialOutput(
    const FString& OutputName,
    UMaterialExpression* RootExpression,
    FString& OutText,
    TMap<UMaterialExpression*, int32>& NodeIds,
    TMap<int32, FString>& NodeTexts,
    int32& NextId);
    
    /** Рекурсивный обход Expression-графа (data-flow) */
    void ProcessExpressionDAG(
    UMaterialExpression* Expression,
    TMap<UMaterialExpression*, int32>& NodeIds,
    TMap<int32, FString>& NodeTexts,
    int32& NextId
    );

    // -------------------------------
    // Expression / Input helpers
    // -------------------------------
    /** Читабельное имя Expression (тип + ключевые параметры) */
    FString GetReadableExpressionName(UMaterialExpression* Expression);

    /** Детализация входных пинов Expression */
    FString GetExpressionInputs(UMaterialExpression* Expression, int32 IndentLevel);

    /** Значение входа: связь или дефолт */
    FString GetInputValueDescription(const struct FExpressionInput& Input);

    /** Проверка: есть ли у Expression входящие связи */
    bool HasAnyInputs(UMaterialExpression* Expression);
    
    FString GetReadableNodeName(UMaterialExpression* Expr, int32 NodeId);
    
    void CollectNodesRecursive(
    UMaterialExpression* Expr,
    TSet<UMaterialExpression*>& Visited,
    TArray<FString>& OutNodeNames,
    TMap<UMaterialExpression*, int32>& NodeIds);


    // -------------------------------
    // Вспомогательные методы
    // -------------------------------
    /** Очистка имён от технических суффиксов */
    FString CleanName(const FString& RawName);

    /** Отступы для визуального отображения графа */
    FString MakeIndent(int32 Level);
    
    bool IsTransparentExpression(UMaterialExpression* Expr);
    
    // Возвращает первый НЕ-прозрачный expression вверх по цепочке
    // Может вернуть nullptr, если цепочка оборвана или зациклена
    UMaterialExpression* ResolveExpression(UMaterialExpression* Expr);

};
