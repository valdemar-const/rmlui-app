# Альтернативный подход: как бы я решил эту задачу

**Контекст**: Платформа-редактор поверх GLFW/RmlUi/OpenGL с плагинами и панелями в стиле Blender.

---

## Короткий ответ

Нет, я бы решил эту задачу существенно иначе. Основные расхождения:

1. **Я бы не начинал с интерфейсов** — я бы начал с работающего прототипа
2. **Я бы не делал столько абстракций на первом этапе** — YAGNI
3. **Я бы сфокусировался на Layout System первым** — это ядро Blender-подобного редактора
4. **Я бы не абстрагировал GLFW** — это не нужно для MVP

---

## Что конкретно я бы сделал иначе

### 1. Начать с Layout, а не с Window/EventLoop

Текущий подход: Core → Plugins → Views → Layout → Input → Resources

Мой подход: **Layout + Views → Plugins → остальное по необходимости**

Причина: Layout System — это **определяющая фича** Blender-подобного редактора. Без неё всё остальное — это просто обёртка над RmlUi. Нужно сначала доказать, что split-панели с горячими углами вообще реализуемы поверх RmlUi, и только потом строить инфраструктуру вокруг.

В текущей реализации Layout Engine написан, но **не подключён** — это говорит о том, что приоритеты были расставлены неверно.

### 2. Не абстрагировать GLFW

`IWindow`, `IWindowManager` — это абстракции, которые не нужны на данном этапе. Вероятность замены GLFW на SDL или что-то другое в этом проекте близка к нулю. А если и возникнет — рефакторинг будет тривиальным, потому что GLFW используется в 2-3 местах.

Вместо этого я бы сделал:

```cpp
// Просто обёртка, не интерфейс
class Window
{
public:
    Window(int width, int height, const char* title);
    ~Window();
    
    GLFWwindow* handle() const { return window_; }
    bool should_close() const;
    void swap_buffers();
    
private:
    GLFWwindow* window_;
};
```

Никаких виртуальных методов, никакого `IWindow`. KISS.

### 3. Не делать Plugin System на первом этапе

Текущая Plugin System — это по сути `std::unordered_map<string, unique_ptr<IPlugin>>` с `OnLoad`/`OnUnload`. Динамическая загрузка не реализована (заглушка с `assert(false)`).

На данном этапе достаточно:

```cpp
// В main.cpp
app.register_view("sample_panel", "assets/ui/sample_panel.rml", 
    [] { return std::make_unique<SamplePanelView>(); });
```

Plugin System имеет смысл добавлять, когда:
- Есть хотя бы 3-5 реальных view
- Понятно, какие сервисы плагинам реально нужны от фреймворка
- Нужна динамическая загрузка

### 4. Сделать App тонким, а не God Object

Вместо 260-строчного `App::run()` я бы сделал:

```cpp
int main()
{
    // Явная инициализация — пользователь видит и контролирует порядок
    auto window = skif::Window{1280, 720, "My Editor"};
    auto gl = skif::GLContext{window};
    auto rml = skif::RmlContext{window, gl};
    
    // Layout — ядро редактора
    auto layout = skif::PanelLayout{rml.context()};
    
    // Регистрация view
    layout.register_view("viewport", [] { return std::make_unique<ViewportView>(); });
    layout.register_view("properties", [] { return std::make_unique<PropertiesView>(); });
    
    // Начальная раскладка
    layout.split_root(skif::Horizontal, 0.7f, "viewport", "properties");
    
    // Main loop — явный, не спрятан за абстракцией
    while (!window.should_close())
    {
        glfwPollEvents();
        rml.update();
        
        gl.clear();
        rml.render();
        window.swap_buffers();
    }
}
```

Преимущества:
- Порядок инициализации/деинициализации очевиден
- Нет скрытого состояния
- Легко отлаживать
- Пользователь контролирует main loop

### 5. Layout System — моё ядро

Вместо дерева `LayoutNode` с `is_splitter` boolean, я бы использовал:

```cpp
// Type-safe дерево раскладки
struct Panel
{
    std::string view_id;
};

struct Split
{
    enum Direction { Horizontal, Vertical };
    Direction direction;
    float ratio = 0.5f;
    float min_size = 50.0f;
    std::unique_ptr<LayoutTree> first;
    std::unique_ptr<LayoutTree> second;
};

using LayoutTree = std::variant<Panel, Split>;
```

И ключевой вопрос — **как это рендерить через RmlUi**:

```cpp
class PanelLayout
{
public:
    // Генерирует RML с абсолютным позиционированием панелей
    // Каждая панель — это div с position:absolute и вычисленными координатами
    void apply(Rml::Context* ctx);
    
    // Обработка горячих углов — это просто invisible элементы в углах панелей
    // При drag — пересчитываем дерево и перегенерируем RML
    void handle_corner_drag(Rml::Element* corner, float dx, float dy);
    
    // Split — создаёт новый Split узел, заменяя Panel
    void split(const std::string& panel_id, Split::Direction dir);
    
    // Merge — убирает Split, оставляя одну из панелей
    void merge(const std::string& panel_id);
};
```

### 6. Сигналы — не изобретать велосипед

Вместо наивного `Signal<T>` без disconnect я бы:
- На раннем этапе: просто `std::function` callbacks
- Когда нужны множественные подписчики: взять `sigslot` (header-only, ~200 строк) или `boost::signals2`

### 7. Input — не абстрагировать

GLFW callbacks → напрямую в RmlUi context. Без промежуточного `IInputManager`. Это 30 строк кода:

```cpp
void setup_input(GLFWwindow* window, Rml::Context* ctx)
{
    struct Ctx { Rml::Context* rml; };
    auto* c = new Ctx{ctx}; // или static, или member
    glfwSetWindowUserPointer(window, c);
    
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int sc, int act, int mods) {
        auto* c = static_cast<Ctx*>(glfwGetWindowUserPointer(w));
        if (act == GLFW_PRESS) c->rml->ProcessKeyDown(...);
        else if (act == GLFW_RELEASE) c->rml->ProcessKeyUp(...);
    });
    
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
        auto* c = static_cast<Ctx*>(glfwGetWindowUserPointer(w));
        c->rml->ProcessMouseMove(int(x), int(y), 0);
    });
    
    // ... аналогично для mouse buttons и scroll
}
```

Когда понадобится абстракция (например, для тестирования или для поддержки gamepad) — тогда и добавим.

---

## Общая философия

Текущий подход — **architecture-first**: сначала спроектировать все интерфейсы, потом реализовать. Это привело к:
- 15+ заголовочных файлов с интерфейсами
- Мёртвому коду (Config, BindEvent, LayoutEngine)
- Несоответствию плана и реализации
- Сложности без необходимости

Мой подход — **feature-first**: сначала сделать работающую фичу (split-панели), потом извлечь абстракции из работающего кода. Это даёт:
- Меньше кода
- Каждая строка проверена использованием
- Абстракции появляются из реальных потребностей, а не из предположений
- Быстрее видимый результат

---

## Что из текущего подхода я бы сохранил

1. **Pimpl для App** — если App остаётся, Pimpl правильный выбор
2. **ViewDescriptor + Factory** — хороший паттерн
3. **Namespace `skif::rmlui`** — правильная организация
4. **Структура директорий** public/private/src — чистая и понятная
5. **CMake с CPM** — хороший выбор для управления зависимостями
6. **C++20** — правильный выбор стандарта

---

## Резюме

| Аспект | Текущий подход | Мой подход |
|--------|---------------|------------|
| Фокус | Инфраструктура | Ключевая фича - Layout |
| Абстракции | Заранее, много | По необходимости, мало |
| GLFW | Абстрагирован через IWindow | Используется напрямую |
| Plugin System | С первого дня | Когда появятся 3+ view |
| Input | IInputManager с Signal | Прямые GLFW callbacks |
| Event Loop | IEventLoop с callbacks | Явный while loop в main |
| Количество файлов | ~30 | ~10-12 |
| Время до работающего split | Не достигнуто | Первый milestone |
