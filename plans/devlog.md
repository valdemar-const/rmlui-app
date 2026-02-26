# DevLog: SkifRmlUi Framework

## 2026-02-26

### Известные проблемы

- **Чёрный экран при запуске**: RML документ не загружается. Требуется отладка путей и проверка загрузки шрифтов/документов.

---

### Фаза 1: Core Foundation (Завершена)

**Цель**: Создать базовую инфраструктуру приложения с абстракцией над GLFW и управлением главным циклом.

**Реализованные компоненты**:

| Компонент | Файл | Описание |
|-----------|------|----------|
| IWindow | `include/skif/rmlui/core/i_window.hpp` | Интерфейс окна |
| IWindowManager | `include/skif/rmlui/core/i_window_manager.hpp` | Менеджер окон |
| IEventLoop | `include/skif/rmlui/core/i_event_loop.hpp` | Главный цикл |
| WindowImpl | `src/implementation/window_impl.cpp` | Реализация GLFW окна |
| WindowManagerImpl | `src/implementation/window_manager_impl.cpp` | Реализация менеджера |
| EventLoopImpl | `src/implementation/event_loop_impl.cpp` | Реализация цикла |

**Ключевые решения**:
1. **Pimpl идиома** в App классе для скрытия деталей реализации
2. **Callback-based event loop** - гибкая система обратных вызовов для update/render
3. **WindowConfig** - конфигурация окна вынесена в отдельную структуру

**Проблемы и решения**:
- Изначально забыт вызов `PollEvents()` в цикле → окно зависало
- Путь к шрифтам относительный → добавлен fallback на стандартный путь

---

### Фаза 2: Plugin System (Завершена)

**Цель**: Обеспечить расширяемость через плагины (статические + динамические).

**Реализованные компоненты**:

| Компонент | Файл | Описание |
|-----------|------|----------|
| IPlugin | `include/skif/rmlui/plugin/i_plugin.hpp` | Базовый интерфейс плагина |
| IPluginRegistry | `include/skif/rmlui/plugin/i_plugin_registry.hpp` | Реестр плагинов |
| IPluginManager | `include/skif/rmlui/plugin/i_plugin_manager.hpp` | Менеджер плагинов |
| PluginManagerImpl | `src/implementation/plugin_manager_impl.cpp` | Реализация |

**Ключевые решения**:
1. **Двойной интерфейс** - IPluginManager наследует IPluginRegistry для унификации
2. **Жизненный цикл** - OnLoad/OnUnload вызываются при старте/остановке плагинов
3. **Интеграция с App** - PluginManager создаётся в App и управляется централизованно

**Ограничения**:
- Динамическая загрузка DLL/SO ещё не реализована (заглушка)
- Пока только статическая регистрация плагинов

---

### Структура проекта

```
projects/lib/skif-rmlui/
├── include/skif/rmlui/
│   ├── app.hpp
│   ├── config.hpp
│   ├── core/
│   │   ├── i_window.hpp
│   │   ├── i_window_manager.hpp
│   │   └── i_event_loop.hpp
│   └── plugin/
│       ├── i_plugin.hpp
│       ├── i_plugin_registry.hpp
│       └── i_plugin_manager.hpp
├── private/implementation/
│   ├── window_impl.hpp
│   ├── window_manager_impl.hpp
│   ├── event_loop_impl.hpp
│   └── plugin_manager_impl.hpp
└── src/
    ├── app.cpp
    └── implementation/
        ├── window_impl.cpp
        ├── window_manager_impl.cpp
        ├── event_loop_impl.cpp
        └── plugin_manager_impl.cpp
```

---

### Следующие шаги

1. **Фаза 3: View System** - система представлений с регистрацией view плагинами
2. **Фаза 4: Layout System** - система панелей как в Blender
3. **Фаза 5: Input System** - обработка ввода с интеграцией в RmlUi
4. **Фаза 6: Resource System** - управление ресурсами

---

### Технические детали

**Компилятор**: MinGW GCC (C++20)
**Сборка**: Ninja, CMake 3.30
**Зависимости**:
- GLFW 3.4
- RmlUi 6.2
- Glad 2.0.8
- Freetype 2.14.1