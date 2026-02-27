# DevLog: SkifRmlUi Framework

## 2026-02-27

### Известные проблемы

- ~~**Чёрный экран при запуске**: RML документ не загружается. Требуется отладка путей и проверка загрузки шрифтов/документов.~~ **ИСПРАВЛЕНО**
- ~~**Кнопки не работают**: Требуется реализация Input System (Фаза 5)~~ **ИСПРАВЛЕНО**
- **Панель не на весь экран**: Фиксированный размер в RML

---

### Фаза 5: Input System (Завершена)

**Цель**: Обработка ввода с интеграцией в RmlUi.

**Реализованные компоненты**:

| Компонент | Файл | Описание |
|-----------|------|----------|
| KeyCode | `include/skif/rmlui/input/key_codes.hpp` | Перечисление клавиш |
| MouseButton | `include/skif/rmlui/input/mouse_buttons.hpp` | Перечисление кнопок мыши |
| IInputManager | `include/skif/rmlui/input/i_input_manager.hpp` | Интерфейс менеджера ввода |
| InputManagerImpl | `private/implementation/input_manager_impl.hpp` | Реализация |

**Ключевые решения**:

1. **GLFW callbacks** - KeyCallback, MouseButtonCallback, MouseMoveCallback, MouseWheelCallback
2. **Состояние ввода** - массивы для key_states, mouse_button_states, mouse_position, mouse_delta
3. **Signal-based events** - OnKeyDown, OnKeyUp, OnMouseDown, OnMouseUp, OnMouseMove
4. **RmlUi интеграция** - SetContext() для передачи RmlUi контекста
5. **Инъекция событий** - InjectKeyDown/Up, InjectMouseMove/Down/Up для отправки в RmlUi

**Интерфейс IInputManager**:
```cpp
class IInputManager
{
public:
    virtual ~IInputManager() = default;
    
    // Keyboard
    virtual bool IsKeyDown(KeyCode key) const = 0;
    virtual bool IsKeyPressed(KeyCode key) const = 0;
    
    // Mouse
    virtual Vector2f GetMousePosition() const = 0;
    virtual Vector2f GetMouseDelta() const = 0;
    virtual bool IsMouseButtonDown(MouseButton button) const = 0;
    virtual float GetMouseWheel() const = 0;
    
    // RmlUi integration
    virtual void InjectKeyDown(int key, int modifiers) = 0;
    virtual void InjectKeyUp(int key, int modifiers) = 0;
    virtual void InjectMouseMove(int x, int y, int dx, int dy) = 0;
    virtual void InjectMouseDown(int x, int y, int button, int modifiers) = 0;
    virtual void InjectMouseUp(int x, int y, int button, int modifiers) = 0;
    
    // Setup
    virtual void SetWindow(GLFWwindow* window) = 0;
    virtual void SetContext(Rml::Context* context) = 0;
    virtual void Update() = 0;
    
    // Signals
    Signal<KeyCode> OnKeyDown;
    Signal<KeyCode> OnKeyUp;
    Signal<MouseButton> OnMouseDown;
    Signal<MouseButton> OnMouseUp;
    Signal<Vector2f> OnMouseMove;
};
```

**Интеграция с App**:
```cpp
// Инициализация InputManager
pimpl_->input_manager = std::make_unique<InputManagerImpl>();

// Установка GLFW окна
pimpl_->input_manager->SetWindow(window->GetGlfwWindow());

// Установка RmlUi контекста (после создания)
pimpl_->input_manager->SetContext(pimpl_->context);
```

**Как работает**:
1. GLFW callbacks регистрируются в SetWindow()
2. При событии ввода - callback обновляет состояние и вызывает signal
3. Callback также вызывает Inject методы для отправки в RmlUi
4. RmlUi::Context::ProcessKeyDown/Up, ProcessMouseMove и т.д. вызываются для обработки UI событий

---

### Пример плагина: Sample Panel (Завершён)

Создан пример плагина для демонстрации workflow разработки UI:

| Файл | Описание |
|------|----------|
| `projects/bin/rmlui-app/plugins/sample_panel.hpp` | Заголовочный файл плагина |
| `projects/bin/rmlui-app/plugins/sample_panel.cpp` | Реализация плагина |
| `projects/bin/rmlui-app/assets/ui/sample_panel.rml` | RML разметка панели |

**Как использовать**:

1. **Создай RML файл** - разметка и стили
2. **Создай класс View** - наследуй `IView`, реализуй методы
3. **Создай класс Plugin** - наследуй `IPlugin`, зарегистрируй View
4. **Зарегистрируй плагин** в `main.cpp`:

```cpp
#include "plugins/sample_panel.cpp"

int main(int argc, char* argv[])
{
    App app {argc, argv};
    app.GetPluginManager().RegisterPlugin(std::make_unique<sample::SamplePanelPlugin>());
    return app.run();
}
```

**Что реализовано**:
- Плагин регистрирует View через `IViewRegistry`
- `AttachView` + `ShowView` загружают и отображают панель
- RML стили работают (шрифт "IBM Plex Mono" в кавычках)
- **Кнопки работают!** - добавлен `LambdaEventListener` для обработки событий

**Как работают кнопки**:
```cpp
// В OnCreated() привязываем обработчики событий
auto* increment_btn = document_->GetElementById("increment-button");
increment_btn->AddEventListener("click",
    new LambdaEventListener([this](Rml::Event& event)
    {
        counter_++;
        UpdateCounterDisplay();
    })
);
```

**LambdaEventListener** - обёртка над `std::function` для RmlUi EventListener API:
```cpp
class LambdaEventListener : public Rml::EventListener
{
public:
    using Callback = std::function<void(Rml::Event&)>;
    explicit LambdaEventListener(Callback callback) : callback_(std::move(callback)) {}
    void ProcessEvent(Rml::Event& event) override {
        if (callback_) callback_(event);
    }
private:
    Callback callback_;
};
```

---

### Фаза 4: Layout System (Завершена)

**Цель**: Система панелей как в Blender - split, drag-and-drop "горячие углы".

**Реализованные компоненты**:

| Компонент | Файл | Описание |
|-----------|------|----------|
| LayoutNode | `include/skif/rmlui/layout/layout_node.hpp` | Узел раскладки |
| ILayoutEngine | `include/skif/rmlui/layout/i_layout_engine.hpp` | Интерфейс движка |
| LayoutEngineImpl | `src/implementation/layout_engine_impl.cpp` | Реализация |

**Ключевые решения**:
1. **SplitDirection** - перечисление для направления разделения (Horizontal/Vertical)
2. **LayoutNode** - структура с ratio, min_size, вложенными узлами first/second
3. **Фабричные методы** - CreateSplitter() и CreatePanel() для удобного создания узлов

**Интерфейс ILayoutEngine**:
```cpp
class ILayoutEngine
{
public:
    virtual ~ILayoutEngine() = default;
    
    virtual void SetRoot(std::unique_ptr<LayoutNode> root) = 0;
    virtual void SplitPanel(Rml::Element* panel, SplitDirection direction, float ratio = 0.5f) = 0;
    virtual void MergePanels(Rml::Element* first, Rml::Element* second) = 0;
    virtual void BeginDrag(Rml::Element* splitter) = 0;
    virtual void UpdateDrag(Vector2f mouse_pos) = 0;
    virtual void EndDrag() = 0;
    virtual std::string GenerateRML() const = 0;
};
```

**Ограничения**:
- Базовая реализация - требуется доработка для полноценной работы с drag-and-drop

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
│   ├── view/
│   │   ├── i_view.hpp
│   │   ├── i_view_registry.hpp
│   │   ├── i_view_host.hpp
│   │   └── view_descriptor.hpp
│   ├── layout/
│   │   ├── i_layout_engine.hpp
│   │   └── layout_node.hpp
│   └── input/
│       ├── i_input_manager.hpp
│       ├── key_codes.hpp
│       └── mouse_buttons.hpp
├── private/implementation/
│   ├── window_impl.hpp
│   ├── window_manager_impl.hpp
│   ├── event_loop_impl.hpp
│   ├── plugin_manager_impl.hpp
│   ├── view_registry_impl.hpp
│   ├── view_host_impl.hpp
│   ├── layout_engine_impl.hpp
│   └── input_manager_impl.hpp
└── src/
    ├── app.cpp
    └── implementation/
        ├── window_impl.cpp
        ├── window_manager_impl.cpp
        ├── event_loop_impl.cpp
        ├── plugin_manager_impl.cpp
        ├── view_registry_impl.cpp
        ├── view_host_impl.cpp
        ├── layout_engine_impl.cpp
        └── input_manager_impl.cpp

projects/bin/rmlui-app/
├── main.cpp                    # Регистрация плагина
├── plugins/
│   ├── sample_panel.hpp        # Заголовочный файл плагина
│   └── sample_panel.cpp        # Реализация плагина
└── assets/ui/
    ├── basic.rml               # Базовая страница (Hello)
    └── sample_panel.rml        # Панель плагина
```

---

### Следующие шаги

1. ~~**Фаза 5: Input System** - обработка ввода с интеграцией в RmlUi~~ **ЗАВЕРШЕНО**
2. **Фаза 6: Resource System** - управление ресурсами

---

### Технические детали

**Компилятор**: MinGW GCC (C++20)
**Сборка**: Ninja, CMake 3.30
**Зависимости**:
- GLFW 3.4
- RmlUi 6.2
- Glad 2.0.8
- Freetype 2.14.1
