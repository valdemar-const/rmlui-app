# DevLog: SkifRmlUi Framework

## 2026-02-26

### Известные проблемы

- ~~**Чёрный экран при запуске**: RML документ не загружается. Требуется отладка путей и проверка загрузки шрифтов/документов.~~ **ИСПРАВЛЕНО**

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

### Фаза 3: View System (Завершена)

**Цель**: Система представлений с регистрацией view плагинами.

**Реализованные компоненты**:

| Компонент | Файл | Описание |
|-----------|------|----------|
| IView | `include/skif/rmlui/view/i_view.hpp` | Интерфейс представления |
| IViewRegistry | `include/skif/rmlui/view/i_view_registry.hpp` | Реестр представлений |
| IViewHost | `include/skif/rmlui/view/i_view_host.hpp` | Хост представлений |
| ViewDescriptor | `include/skif/rmlui/view/view_descriptor.hpp` | Дескриптор view |
| ViewRegistryImpl | `src/implementation/view_registry_impl.cpp` | Реализация реестра |
| ViewHostImpl | `src/implementation/view_host_impl.cpp` | Реализация хоста |

**Ключевые решения**:
1. **"Глупые" view** - RML документы только для presentation, логика в C++
2. **Интеграция с Plugin System** - плагины регистрируют view через IPluginRegistry::GetViewRegistry()
3. **ViewHost** - управляет жизненным циклом view в RmlUi контексте

**Как плагины регистрируют view**:
```cpp
// В плагине
void MyPlugin::OnLoad(IPluginRegistry& registry) {
    registry.GetViewRegistry().RegisterView(
        {"my_view", "assets/ui/my_view.rml", "Panels", "My View"},
        []() { return std::make_unique<MyView>(); }
    );
}
```

---

### Исправления багов

#### Чёрный экран при запуске

**Проблема**: Окно было чёрным, хотя RML документ загружался успешно.

**Причина**: 
1. Не вызывался `glClear()` для очистки буфера
2. Контекст OpenGL не делался активным перед рендерингом

**Решение** (в `src/app.cpp`):
```cpp
pimpl_->event_loop->OnRender(
    [this, window = window.get()]()
    {
        window->MakeContextCurrent();  // Добавлено
        pimpl_->gl->Clear(GL_COLOR_BUFFER_BIT);  // Добавлено
        
        if (pimpl_->context)
        {
            pimpl_->context->Render();
        }
        window->SwapBuffers();
    }
);
```

#### Resize не обновлял разметку

**Проблема**: При изменении размера окна разметка не обновлялась.

**Причина**: Не использовались GLFW callbacks для обработки resize событий.

**Решение** (в `src/app.cpp`):
```cpp
// Структура для передачи состояния в callbacks
struct WindowState
{
    GladGLContext* gl      = nullptr;
    Rml::Context*  context = nullptr;
};

WindowState window_state;
window_state.gl = pimpl_->gl.get();
glfwSetWindowUserPointer(window->GetGlfwWindow(), &window_state);

// FramebufferSizeCallback - обновляет viewport и размер RmlUi контекста
glfwSetFramebufferSizeCallback(
    window->GetGlfwWindow(),
    [](GLFWwindow* glfw_window, int new_width, int new_height)
    {
        auto* state = static_cast<WindowState*>(glfwGetWindowUserPointer(glfw_window));
        state->gl->Viewport(0, 0, new_width, new_height);
        state->context->SetDimensions({new_width, new_height});
    }
);

// WindowRefreshCallback - принудительная перерисовка при resize
glfwSetWindowRefreshCallback(
    window->GetGlfwWindow(),
    [](GLFWwindow* glfw_window)
    {
        auto* state = static_cast<WindowState*>(glfwGetWindowUserPointer(glfw_window));
        state->gl->Clear(GL_COLOR_BUFFER_BIT);
        state->context->Update();
        state->context->Render();
        glfwSwapBuffers(glfw_window);
    }
);
```

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
│   ├── plugin/
│   │   ├── i_plugin.hpp
│   │   ├── i_plugin_registry.hpp
│   │   └── i_plugin_manager.hpp
│   └── view/
│       ├── i_view.hpp
│       ├── i_view_registry.hpp
│       ├── i_view_host.hpp
│       └── view_descriptor.hpp
├── private/implementation/
│   ├── window_impl.hpp
│   ├── window_manager_impl.hpp
│   ├── event_loop_impl.hpp
│   ├── plugin_manager_impl.hpp
│   ├── view_registry_impl.hpp
│   └── view_host_impl.hpp
└── src/
    ├── app.cpp
    └── implementation/
        ├── window_impl.cpp
        ├── window_manager_impl.cpp
        ├── event_loop_impl.cpp
        ├── plugin_manager_impl.cpp
        ├── view_registry_impl.cpp
        └── view_host_impl.cpp
```

---

### Следующие шаги

1. **Фаза 4: Layout System** - система панелей как в Blender
2. **Фаза 5: Input System** - обработка ввода с интеграцией в RmlUi
3. **Фаза 6: Resource System** - управление ресурсами

---

### Технические детали

**Компилятор**: MinGW GCC (C++20)
**Сборка**: Ninja, CMake 3.30
**Зависимости**:
- GLFW 3.4
- RmlUi 6.2
- Glad 2.0.8
- Freetype 2.14.1
