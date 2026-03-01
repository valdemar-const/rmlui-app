# Roadmap: SkifRmlUi Framework

**Последнее обновление**: 2026-03-01

---

## Текущий статус

Фреймворк находится в стадии активной разработки. Базовая функциональность реализована и работает.

| Компонент | Статус | Примечание |
|-----------|--------|------------|
| Core (Window, EventLoop) | ✅ Готово | |
| Plugin System | ✅ Готово | Статическая загрузка |
| View System | ✅ Готово | |
| Input System | ✅ Готово | |
| Layout System | ⚠️ Базовая | Требует доработки |
| Resource System | 🔄 В процессе | |

---

## Приоритет 1: Ближайшие задачи

### Layout System — полная реализация

- [ ] Интеграция LayoutEngine с ViewHost
- [ ] Split панелей через UI (горячие углы)
- [ ] Drag-and-drop для изменения размеров
- [ ] Сохранение/загрузка layout

### Resource System

- [ ] IResourceManager интерфейс
- [ ] Поиск ресурсов в нескольких директориях
- [ ] Кэширование загруженных ресурсов

---

## Приоритет 2: Улучшения качества

### Transparent hashing для строковых ключей

```cpp
// Позволяет искать по string_view без создания std::string
std::unordered_map<std::string, Value, StringHash, std::equal_to<>> map;
map.find(std::string_view{"key"});  // Работает без аллокации
```

**Файлы**: `view_registry_impl.hpp`, `plugin_manager_impl.hpp`

### Multiple callbacks в IEventLoop

```cpp
// Текущее ограничение: один callback на событие
event_loop->OnUpdate([](float dt) { ... });  // Перезаписывает предыдущий

// Желаемое поведение: несколько callbacks
auto conn1 = event_loop->OnUpdate.Connect([](float dt) { ... });
auto conn2 = event_loop->OnUpdate.Connect([](float dt) { ... });
conn1.Disconnect();  // Отключить первый
```

---

## Приоритет 3: Мультиоконность

### WindowSession

Подготовка к поддержке нескольких окон:

```cpp
struct WindowSession
{
    std::shared_ptr<IWindow> window;
    std::unique_ptr<GladGLContext> gl;
    Rml::Context* rml_context;
    std::unique_ptr<IViewHost> view_host;
};

class App
{
    std::vector<WindowSession> sessions_;
    WindowSession* main_session_;
};
```

**Задачи**:
- [ ] Рефакторинг App для поддержки нескольких WindowSession
- [ ] Синхронизация GL контекстов между окнами
- [ ] Управление фокусом между окнами

---

## Приоритет 4: Динамическая загрузка плагинов

### Plugin DLL/SO loading

```cpp
// Текущее: только статическая регистрация
app.GetPluginManager().RegisterStaticPlugin(std::make_unique<MyPlugin>());

// Желаемое: динамическая загрузка
app.GetPluginManager().LoadPlugin("plugins/my_plugin.dll");
```

**Задачи**:
- [ ] Кроссплатформенная загрузка DLL/SO
- [ ] Версионирование ABI плагинов
- [ ] Hot-reload плагинов (опционально)

---

## Приоритет 5: Дополнительные улучшения

### Debugger интеграция

- [ ] Интеграция RmlUi Debugger
- [ ] Горячая клавиша для включения/выключения

### Тестирование

- [ ] Unit-тесты для Signal
- [ ] Unit-тесты для ViewRegistry
- [ ] Integration-тесты для App

### Документация

- [ ] Doxygen комментарии для публичных API
- [ ] Примеры создания плагинов
- [ ] Туториал по созданию View

---

## Архитектурные принципы

1. **Публичные интерфейсы минимальны** — только то, что нужно пользователям
2. **Детали реализации скрыты** — в `private/` и `Impl` классах
3. **GLFW-зависимости изолированы** — не в публичных заголовках
4. **Signal с disconnect** — для безопасного управления подписками
5. **Pimpl для ABI стабильности** — в App и других публичных классах
