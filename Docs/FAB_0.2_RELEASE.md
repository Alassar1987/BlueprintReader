# BlueprintReader v0.2 — Fab release package

> Всё для публикации обновления v0.2 на Fab. Публикуешь ты — здесь готовые тексты и чек-лист.

## 1. Версия

`.uplugin`: `Version: 2`, `VersionName: "0.2"` (было `1` / `"0.1"`).

---

## 2. What's new in v0.2 (вставить в поле «What's new» на Fab)

**Native MCP tools (UE 5.8) — главное.** BlueprintReader теперь нативный MCP-мост: любой MCP-агент (Claude Code, Cursor, Codex, Gemini) читает и валидирует ассеты напрямую, без UI. 8 тулов:

- `list_blueprint_reader_types` — список поддерживаемых типов.
- `search_blueprint_reader_assets` — поиск по имени / пути / типу (различает подтипы).
- `read_blueprint_reader_asset` — структурированный JSON (поля, граф, checksum) или Markdown.
- `read_blueprint_reader_asset_section` — чтение одной секции (structure / graph / design).
- `export_blueprint_reader_asset` — экспорт в `.md`.
- `get_blueprint_reader_checksum` — saved package hash (проверка свежести).
- `get_blueprint_reader_references` — зависимости / ссылающиеся.
- `validate_blueprint_reader_asset` — статус компиляции, зависимости, вердикт.

**Structured JSON.** `read` по умолчанию отдаёт типизированный JSON (`assetPath`, `assetName`, `type`, `structure`, `graph`, `design`, `checksum`) — меньше токенов и надёжнее парсинг, чем Markdown.

**Validation.** Движок — источник истины: `validate` возвращает статус компиляции, счёт зависимостей, missing-пакеты и вердикт `Valid`.

**8 поддерживаемых типов.** Actor, ActorComponent, Widget, Material, MaterialFunction, Enum, Structure, Interface.

**Single-source 5.7 + 5.8.** Один исходник собирается на обеих версиях движка.

---

## 3. Listing description (заменить описание на Fab)

```
BlueprintReader — стандартный слой доступа между ассетами Unreal Engine и AI.
Извлекает структуру, граф и дизайн любого Blueprint в Markdown или структурированный JSON — для людей и AI-агентов.

Один правый клик или один MCP-вызов:
• ПКМ → «Read Blueprint for AI» → двухпанельное окно (Structure / Graph) → копировать или экспорт в .md.
• Нативный MCP (UE 5.8): 8 тулов — поиск / чтение / экспорт / валидация напрямую из любого MCP-агента (Claude Code, Cursor, Codex, Gemini), без UI.

Engine-grade извлечение:
• Структурированный JSON (поля + граф + checksum) — экономия токенов, надёжный парсинг.
• Валидация: статус компиляции, зависимости, missing-пакеты, вердикт.
• 8 типов ассетов: Actor, ActorComponent, Widget, Material, MaterialFunction, Enum, Structure, Interface.

Single-source: один плагин собирается на UE 5.7 и 5.8.
MIT-лицензия — бесплатно для личного и коммерческого использования.
```

---

## 4. Release checklist (что сделать на Fab)

- [ ] В Fab-портале открыть продукт `562ab219-d38e-413c-89a1-a9c1b09a5b97` → «Update».
- [ ] Загрузить новый пакет (v0.2). ⚠️ Перед `RunUAT BuildPlugin` для каждой версии движка проставить корректный `EngineVersion` (`"5.7.0"` / `"5.8.0"`) — это M6 Phase 4, отдельные пакеты под 5.7 и 5.8.
- [ ] Вставить «What's new» (секция 2 выше).
- [ ] Заменить описание (секция 3 выше).
- [ ] Обновить скриншоты/видео: добавить кадр с MCP-тулами в Inspector и пример structured JSON.
- [ ] Проверить, что `IsBetaVersion` в `.uplugin` оставлен `true` (0.2 — ещё бета) или снять флаг, если хочешь стабильный релиз.

---

## 5. Связанные артефакты

- `README.md` — обновлён (MCP-секция больше не «roadmap»).
- `Resources/Icon128.png` — иконка Fab (при необходимости обновить).
- `.docs/SmokeMCP.py` — smoke-сьют (прогнать перед публикацией, 21/21).
- `.docs/BuildMatrix.bat` — сборка 5.7 + 5.8 (прогнать перед публикацией).
