# Roadmap: SkifRmlUi Framework

**Последнее обновление**: 2026-03-01

---

## Текущий статус

Фреймворк переходит от простой RmlUi-обёртки к полноценной **редакторной платформе** в стиле Blender.

| Компонент | Статус | Примечание |
|-----------|--------|------------|
| Core — Window, EventLoop | ✅ Готово | |
| Plugin System | ✅ Готово | Статическая загрузка |
| Input System | ✅ Готово | |
| Signal | ⚠️ Требует исправления | Dangling pointer в Connection |
| View System | 🔄 Deprecated | Заменяется на Editor System |
| Layout System | 🔄 Deprecated | Заменяется на SplitLayout |
| **Editor System** | 🆕 В разработке | IEditor, EditorRegistry, EditorHost |
| **SplitLayout** | 🆕 В разработке | Рекурсивное дерево панелей |
| Resource System | ⏳ Отложено | |

---

## Phase 1: Ядро редакторной платформы (текущая)

### 1.1 Исправление критических проблем

- [ ] **Signal — безопасный disconnect** (P07)
  - Заменить захват `this` на `shared_ptr<State>` / `weak_ptr`
  - Connection должен быть безопасен после уничтожения Signal
  - Файл: `include/skif/rmlui/core/signal.hpp`

### 1.2 Editor System — новые интерфейсы

- [ ] **IEditor** — интерфейс редактора
  - Методы: `OnCreated`, `OnActivate`, `OnDeactivate`, `OnUpdate`, `OnDispose`
  - Файл: `include/skif/rmlui/editor/i_editor.hpp`

- [ ] **EditorDescriptor** — метаданные редактора
  - Поля: name, display_name, rml_path, icon, category, menu_entries
  - Файл: `include/skif/rmlui/editor/editor_descriptor.hpp`

- [ ] **IEditorRegistry** — contribution point
  - Методы: `RegisterEditor`, `CreateEditor`, `GetDescriptor`, `GetAllDescriptors`
  - Файл: `include/skif/rmlui/editor/i_editor_registry.hpp`

- [ ] **IEditorHost** — управление жизненным циклом
  - Создание/уничтожение экземпляров Editor
  - Привязка к RmlUi контексту
  - Файл: `include/skif/rmlui/editor/i_editor_host.hpp`

### 1.3 SplitLayout — рекурсивное дерево панелей

- [ ] **SplitNode** — узел дерева
  - Leaf: содержит editor_name
  - Split: direction + ratio + first/second
  - Файл: `include/skif/rmlui/editor/split_node.hpp`

- [ ] **ISplitLayout** — интерфейс layout
  - SetRoot, Split, Merge, Update (рекурсивный), ApplyLayout
  - Файл: `include/skif/rmlui/editor/i_split_layout.hpp`

### 1.4 Реализации

- [ ] **EditorRegistryImpl** — `private/implementation/editor_registry_impl.hpp`
- [ ] **EditorHostImpl** — `private/implementation/editor_host_impl.hpp`
- [ ] **SplitLayoutImpl** — `private/implementation/split_layout_impl.hpp`

### 1.5 Интеграция с App

- [ ] Добавить `IEditorRegistry`, `IEditorHost`, `ISplitLayout` в `App::Impl`
- [ ] Добавить `App::SetInitialLayout(std::unique_ptr<SplitNode>)`
- [ ] Добавить `App::GetEditorRegistry()`, `App::GetSplitLayout()`
- [ ] Обновить `IPluginRegistry::GetEditorRegistry()` — contribution point
- [ ] Рекурсивное обновление layout в event loop

### 1.6 Обновление sample plugin

- [ ] Переписать `SamplePanelPlugin` → `SampleEditorPlugin`
- [ ] `SamplePanelView` → `SampleEditor` (реализует `IEditor`)
- [ ] Использовать `IEditorRegistry` для регистрации
- [ ] Обновить `main.cpp` — задать начальный layout

---

## Phase 2: Улучшения качества

### EventLoop — множественные callbacks

```cpp
// Текущее: один callback
event_loop->OnUpdate([](float dt) { ... });

// Желаемое: Signal-based
auto conn = event_loop->OnUpdate.Connect([](float dt) { ... });
conn.Disconnect();
```

- [ ] Заменить `std::optional<Callback>` на `Signal<Args...>` в EventLoop
- [ ] Обновить App для использования нового API

### Transparent hashing

```cpp
struct StringHash
{
    using is_transparent = void;
    size_t operator()(std::string_view sv) const;
    size_t operator()(const std::string& s) const;
};
std::unordered_map<std::string, Value, StringHash, std::equal_to<>> map;
```

- [ ] Применить в EditorRegistryImpl, PluginManagerImpl

### Исправление мелких проблем из верификации

- [ ] Добавить проверки nullptr в WindowImpl
- [ ] Убрать static_cast к WindowImpl из App — использовать GetNativeHandle
- [ ] Перенести приватные методы App в Impl (полный Pimpl)
- [ ] Добавить override к PluginManagerImpl::SetViewRegistry

---

## Phase 3: Интерактивный layout

### Split/Merge через UI

- [ ] Горячие углы панелей — drag для split
- [ ] Drag разделителей для изменения пропорций
- [ ] Контекстное меню панели — выбор типа редактора
- [ ] Анимация split/merge

### Сериализация layout

```cpp
// Сохранение
std::string json = split_layout->Serialize();

// Загрузка
auto root = SplitNode::Deserialize(json);
split_layout->SetRoot(std::move(root));
```

- [ ] JSON сериализация SplitNode дерева
- [ ] Сохранение/загрузка layout из файла

---

## Phase 4: Мультиоконность

### WindowSession

```cpp
struct WindowSession
{
    std::shared_ptr<IWindow> window;
    std::unique_ptr<GladGLContext> gl;
    Rml::Context* rml_context;
    std::unique_ptr<ISplitLayout> layout;
};
```

- [ ] Рефакторинг App для нескольких WindowSession
- [ ] Синхронизация GL контекстов
- [ ] Управление фокусом между окнами

---

## Phase 5: Динамическая загрузка плагинов

```cpp
// Текущее: статическая регистрация
app.GetPluginManager().RegisterPlugin(std::make_unique<MyPlugin>());

// Желаемое: динамическая загрузка
app.GetPluginManager().LoadPlugin("plugins/my_plugin.dll");
```

- [ ] Кроссплатформенная загрузка DLL/SO
- [ ] Версионирование ABI
- [ ] Hot-reload (опционально)

---

## Phase 6: Дополнительные улучшения

### Command System

- [ ] ICommandRegistry — регистрация команд
- [ ] Привязка горячих клавиш к командам
- [ ] Undo/Redo стек

### Menu System

- [ ] Глобальное меню приложения
- [ ] Меню редактора (header bar)
- [ ] Contribution point для расширения меню из плагинов

### Тестирование

- [ ] Unit-тесты для Signal
- [ ] Unit-тесты для EditorRegistry
- [ ] Unit-тесты для SplitLayout (split/merge/serialize)
- [ ] Integration-тесты для App

### Документация

- [ ] Doxygen комментарии для публичных API
- [ ] Примеры создания плагинов с редакторами
- [ ] Туториал по созданию Editor

---

## Архитектурные принципы

1. **Публичные интерфейсы минимальны** — только то, что нужно пользователям
2. **Детали реализации скрыты** — в `private/` и `Impl` классах
3. **GLFW-зависимости изолированы** — не в публичных заголовках
4. **Signal с безопасным disconnect** — shared_ptr для предотвращения dangling pointers
5. **Pimpl для ABI стабильности** — в App и других публичных классах
6. **Contribution points** — плагины расширяют систему через регистрацию
7. **Рекурсивный layout** — обновление сверху вниз, каждый Editor независим
8. **Editor = самодостаточная единица** — view + menu + логика
