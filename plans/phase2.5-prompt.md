# Промпт: Phase 2.5 — Интерактивный Layout с реальной загрузкой RML

Проанализируй текущую кодовую базу C++ проекта SkifRmlUi и реализуй интерактивную систему Blender-подобных панелей. Проект использует GLFW/OpenGL 3.3/RmlUi 6.2, C++20, CMake 3.30, MinGW/GCC.

## Контекст проблемы

Сейчас в проекте два параллельных механизма загрузки RML:

1. **EditorHostImpl::CreateEditor()** — загружает RML документ каждого Editor отдельно через `context->LoadDocument(rml_path)`. Каждый Editor получает свой `Rml::ElementDocument*`.

2. **SplitLayoutImpl::GenerateRML()** — генерирует RML-строку с panel container (header bar, editor switcher `<select>`, hot corners, content area, status bar), но **не загружает её** — только логирует.

Эти два механизма не связаны: layout RML генерируется, но не используется; Editor RML загружается напрямую без обёртки panel container.

## Задача

Объединить эти механизмы так, чтобы:

1. **Layout RML стал единым документом-контейнером**, загружаемым через `context->LoadDocumentFromMemory()`. Этот документ содержит всю структуру split-панелей с dividers, panel containers, header bars и т.д.

2. **RML каждого Editor загружался внутрь `panel-content` div** соответствующего panel container. Варианты:
   - Загрузить Editor RML как отдельный документ и вставить его body-контент в panel-content через `AppendChild`
   - Или использовать RmlUi `<template>` / inline RML

3. **PanelContainerController** — C++ класс, привязывающий event listeners к элементам panel container после загрузки layout документа:
   - `<select class="editor-switcher">` → при `change` вызывать `ISplitLayout::SwitchEditor()`
   - `.hot-corner[data-action=split]` → при mousedown+drag определять направление и вызывать `ISplitLayout::Split()`
   - `.hot-corner[data-action=merge]` → при drag на соседнюю панель вызывать `ISplitLayout::Merge()`
   - `.split-divider` → при mousedown+drag обновлять `SplitNode::ratio` и пересчитывать CSS размеры панелей
   - Визуальная обратная связь: preview overlay при drag, изменение курсора

4. **Стили** из `assets/ui/panel_container.rcss` должны подключаться к layout документу (через `<link>` или inline `<style>`)

5. **Обратная совместимость**: существующие плагины (SampleEditor) должны работать без изменений — их `OnCreated(document)` должен получать корректный `Rml::ElementDocument*`

## Ключевые файлы для изучения

### Планы и документация
- `plans/architecture.md` — архитектура проекта
- `plans/phase2-interactive-layout.md` — детальный план Phase 2 с RML шаблонами, CSS, диаграммами
- `plans/roadmap.md` — общий roadmap
- `plans/architecture-verification.md` — отчёт верификации с найденными проблемами

### Публичные интерфейсы (include/skif/rmlui/editor/)
- `i_split_layout.hpp` — ISplitLayout с методами SetRoot, Split, Merge, SwitchEditor, Update, ApplyLayout, GenerateRML, SetEditorHost, SetEditorRegistry
- `i_editor_host.hpp` — IEditorHost: CreateEditor, DestroyEditor, ActivateEditor, DeactivateEditor, UpdateEditor
- `i_editor_registry.hpp` — IEditorRegistry: RegisterEditor, CreateEditor, GetDescriptor, GetAllDescriptors
- `i_editor.hpp` — IEditor: OnCreated, OnActivate, OnDeactivate, OnUpdate, OnDispose, GetStatusText
- `editor_descriptor.hpp` — EditorDescriptor с MenuEntry
- `split_node.hpp` — SplitNode: MakeLeaf, MakeSplit, IsLeaf, IsSplit

### Реализации (private/ и src/)
- `split_layout_impl.hpp/.cpp` — SplitLayoutImpl: GenerateRML уже генерирует panel container с header/switcher/content/statusbar/hot-corners
- `editor_host_impl.hpp/.cpp` — EditorHostImpl: создаёт Editor через registry, загружает RML через context->LoadDocument
- `editor_registry_impl.hpp/.cpp` — EditorRegistryImpl: хранит EditorDescriptor + EditorFactory

### Интеграция
- `src/app.cpp` — App::Impl: создаёт все компоненты, настраивает event loop (Update → split_layout->Update(dt)), render loop
- `include/skif/rmlui/app.hpp` — публичный API App

### Стили и ассеты
- `assets/ui/panel_container.rcss` — CSS стили для panel container, split dividers, hot corners
- `assets/ui/sample_panel.rml` — RML документ SampleEditor
- `assets/ui/basic.rml` — fallback RML

### Плагин-пример
- `plugins/sample_panel.hpp/.cpp` — SampleEditor (реализует IEditor), SamplePanelPlugin (регистрирует через IEditorRegistry)
- `main.cpp` — создаёт App, регистрирует плагин, задаёт SetInitialLayout(SplitNode::MakeLeaf("sample_panel"))

### Утилиты
- `include/skif/rmlui/view/lambda_event_listener.hpp` — LambdaEventListener + BindEvent() для привязки C++ обработчиков к RmlUi элементам
- `include/skif/rmlui/core/signal.hpp` — Signal с безопасным disconnect (weak_ptr)

## Ограничения

- **RmlUi 6.2 API** — использовать `LoadDocument`, `LoadDocumentFromMemory`, `AppendChild`, `AddEventListener`
- **C++20** — можно использовать concepts, structured bindings, std::string_view и т.д.
- **Кроссплатформенность** — MSVC, GCC, Clang
- **Вся логика на C++** — никакого JavaScript/Lua
- **Не ломать существующие плагины** — SampleEditor::OnCreated(document) должен работать

## Ожидаемый результат

После реализации:
1. При запуске приложения видна панель с header bar (dropdown "Sample Panel" + menu items "Increment", "Reset")
2. Content area содержит RML из sample_panel.rml (счётчик с кнопками)
3. Status bar внизу панели
4. Dropdown позволяет переключить тип редактора (если зарегистрировано несколько)
5. Разделители между панелями (если layout содержит split) можно перетаскивать
6. Горячие углы позволяют split/merge панелей
7. Проект собирается без ошибок: `cmake --build out/Debug`
