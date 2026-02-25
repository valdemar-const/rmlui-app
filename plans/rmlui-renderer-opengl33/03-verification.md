# Верификация реализации RendererOpenGL33 — Отчёт

## 1. Общая оценка

Реализация **архитектурно корректна** и соответствует заявленным целям MVP. Шаблонная обёртка, концепт, explicit instantiation и pimpl-паттерн работают как задумано. Базовый рендеринг текста и геометрии функционирует.

Ниже — детальный разбор найденных проблем, ранжированных по серьёзности.

---

## 2. Критические проблемы

### 2.1. BUG: TGA-заголовок без `#pragma pack(1)` — потенциально некорректный парсинг

**Файл:** [`Impl.inl`](projects/lib/rmlui-renderer-opengl33/include/RmlUi/details/RenderOpenGL33/Impl.inl:290) строки 290–304

Структура `TgaHeader` содержит `short int` поля, которые на большинстве компиляторов будут выровнены с padding. Без `#pragma pack(1)` компилятор может вставить padding-байты между `id_length`/`color_map_type`/`data_type` и `color_map_origin`, что приведёт к неправильному чтению заголовка.

Референсная реализация явно использует `#pragma pack(1)` перед определением `TGAHeader` и `#pragma pack()` после — см. [RmlUi_Renderer_GL3.cpp:1172–1188](.vibe/rmlui-details/RmlUi_Renderer_GL3.cpp:1172).

**Рекомендация:** Добавить `#pragma pack(push, 1)` перед `TgaHeader` и `#pragma pack(pop)` после. Либо использовать побайтовое чтение полей.

**Серьёзность:** 🔴 Критическая — на некоторых компиляторах/платформах TGA-файлы будут парситься некорректно. На MSVC x64 `sizeof(TgaHeader)` без pack будет 20 вместо ожидаемых 18 байт.

---

### 2.2. BUG: `RenderShader` не помечен `override`

**Файл:** [`RenderOpenGL33.hpp`](projects/lib/rmlui-renderer-opengl33/include/RmlUi/RenderOpenGL33.hpp:145) строка 145

```cpp
void RenderShader(CompiledShaderHandle shader, CompiledGeometryHandle geometry, 
                  Vector2f translation, TextureHandle texture);
```

Все остальные виртуальные методы помечены `override`, но `RenderShader` — нет. Это означает, что метод **не переопределяет** базовый виртуальный метод, а скрывает его. При вызове через указатель на `RenderInterface` будет вызвана базовая реализация, а не ваша.

**Рекомендация:** Добавить `override`.

**Серьёзность:** 🔴 Критическая — нарушен полиморфный контракт. Хотя сейчас это заглушка, при будущей реализации это приведёт к трудноуловимому багу.

---

## 3. Существенные проблемы

### 3.1. PERF: `UpdateViewportState()` вызывается на каждый `RenderGeometry`

**Файл:** [`Impl.inl`](projects/lib/rmlui-renderer-opengl33/include/RmlUi/details/RenderOpenGL33/Impl.inl:227) строка 227

Каждый вызов `RenderGeometry` выполняет `glGetIntegerv(GL_VIEWPORT, ...)`, пересчитывает проекционную матрицу и помечает все программы как dirty. Для типичного UI-фрейма с сотнями draw-вызовов это:
- сотни лишних `glGetIntegerv` — driver roundtrip;
- сотни пересчётов `ProjectOrtho`;
- сотни лишних `glUniformMatrix4fv` из-за постоянного dirty-флага.

Референсная реализация вызывает `SetViewport` **один раз** при изменении размера окна, а `projection` пересчитывается только в `SetViewport`.

**Рекомендация:** Убрать `UpdateViewportState()` из `RenderGeometry`. Вместо этого:
- Вызывать обновление viewport/projection явно при resize через отдельный публичный метод или при первом рендере фрейма.
- Либо кэшировать предыдущие значения viewport и пересчитывать проекцию только при изменении.

**Серьёзность:** 🟡 Существенная — значительная деградация производительности на сложных UI.

---

### 3.2. DESIGN: `EnableScissorRegion` и `SetScissorRegion` — расхождение с референсом

**Файл:** [`Impl.inl`](projects/lib/rmlui-renderer-opengl33/include/RmlUi/details/RenderOpenGL33/Impl.inl:429) строки 429–477

Текущая реализация `EnableScissorRegion(true)` немедленно вызывает `glEnable(GL_SCISSOR_TEST)`, а `SetScissorRegion` тоже может вызвать `glEnable`. Это приводит к двойному включению.

Референсная реализация работает иначе:
- `EnableScissorRegion(false)` → сбрасывает scissor в invalid;
- `EnableScissorRegion(true)` → **ничего не делает**, т.к. предполагается что сразу за ним последует `SetScissorRegion`;
- `SetScissor` сам управляет `glEnable`/`glDisable` на основе валидности региона.

Текущая реализация функционально работает, но при `SetScissorRegion` с невалидным регионом и `scissor_enabled_ == false` вызывается `glDisable` — что может быть избыточно.

**Рекомендация:** Рассмотреть упрощение по образцу референса — управлять GL-состоянием scissor только в одном месте.

**Серьёзность:** 🟡 Средняя — функционально работает, но логика запутана и может привести к багам при расширении.

---

### 3.3. DESIGN: GL-константы как шаблонные переменные в анонимном namespace

**Файл:** [`Impl.inl`](projects/lib/rmlui-renderer-opengl33/include/RmlUi/details/RenderOpenGL33/Impl.inl:17) строки 17–73

Все GL-константы обёрнуты в `template<typename GL> static constexpr unsigned int kGlXxx = GL_XXX;`. Это:
1. **Не зависит от типа GL** — шаблонный параметр не используется в значении, только для ODR-изоляции.
2. Создаёт зависимость `.inl` файла от GL-заголовков через макросы `GL_VERTEX_SHADER`, `GL_FLOAT` и т.д.

Это означает, что `.inl` файл **неявно требует** включения GL-заголовков до себя. Сейчас это работает, потому что [`RendererGlad33.cpp`](projects/lib/skif-rmlui/src/RmlUi/RendererGlad33.cpp:5) включает `<glad/gl.h>` перед `RenderOpenGL33.hpp`. Но если кто-то попытается инстанцировать шаблон без предварительного включения GL-заголовков — получит ошибку компиляции на неизвестные макросы.

**Рекомендация:** Задокументировать это требование. Или перенести константы в enum/constexpr внутри TU инстанцирования.

**Серьёзность:** 🟡 Средняя — работает, но нарушает заявленную цель «концепт не зависит от GL-заголовков». Концепт действительно не зависит, но `.inl` — зависит.

---

### 3.4. SAFETY: `LoadTexture` — отсутствие проверки размера пиксельных данных

**Файл:** [`Impl.inl`](projects/lib/rmlui-renderer-opengl33/include/RmlUi/details/RenderOpenGL33/Impl.inl:344) строка 344

```cpp
const byte *src = buffer.data() + sizeof(TgaHeader);
```

Нет проверки, что `file_size - sizeof(TgaHeader) >= width * height * bytes_per_pixel`. Если TGA-файл повреждён или обрезан, произойдёт чтение за пределами буфера.

**Рекомендация:** Добавить проверку:
```cpp
const size_t expected_data_size = width * height * bytes_per_pixel;
if (file_size - sizeof(TgaHeader) < expected_data_size) { /* error */ }
```

**Серьёзность:** 🟡 Существенная — потенциальный out-of-bounds read на повреждённых файлах.

---

## 4. Незначительные проблемы

### 4.1. STYLE: Шейдерные функции как шаблоны без причины

**Файл:** [`Impl.inl`](projects/lib/rmlui-renderer-opengl33/include/RmlUi/details/RenderOpenGL33/Impl.inl:75) строки 75–133

`VertexShaderMain<GL>()`, `FragmentShaderColor<GL>()`, `FragmentShaderTexture<GL>()` — шаблонные функции, но тип `GL` не используется в теле. Шаблонизация нужна только для ODR-изоляции в `.inl`. Это работает, но выглядит как over-engineering.

**Рекомендация:** Можно заменить на `inline const char*` функции или `constexpr` строковые литералы.

---

### 4.2. STYLE: `VerticallyFlipped` определён, но не используется

**Файл:** [`Impl.inl`](projects/lib/rmlui-renderer-opengl33/include/RmlUi/details/RenderOpenGL33/Impl.inl:845) строки 845–853

Метод `VerticallyFlipped` определён, но нигде не вызывается. В `SetScissorRegion` вертикальный flip делается inline.

**Рекомендация:** Либо использовать `VerticallyFlipped` в `SetScissorRegion`, либо удалить мёртвый код.

---

### 4.3. STYLE: `RenderGeometry` не сбрасывает текстуру после отрисовки

**Файл:** [`Impl.inl`](projects/lib/rmlui-renderer-opengl33/include/RmlUi/details/RenderOpenGL33/Impl.inl:246)

Референсная реализация после `glDrawElements` вызывает `glBindTexture(GL_TEXTURE_2D, 0)` — см. [RmlUi_Renderer_GL3.cpp:1051](.vibe/rmlui-details/RmlUi_Renderer_GL3.cpp:1051). Текущая реализация этого не делает. Это не баг, но может привести к неожиданному состоянию при расширении рендерера.

---

### 4.4. INFO: `protected` секция в `Impl` с `get_gl_`

**Файл:** [`Impl.hpp`](projects/lib/rmlui-renderer-opengl33/include/RmlUi/details/RenderOpenGL33/Impl.hpp:145) строки 145–150

`Impl` — вложенная структура, от которой никто не наследуется. `protected` секция не имеет смысла — можно заменить на `private`.

---

## 5. Архитектурная оценка

### 5.1. Шаблонная архитектура ✅

- Концепт `OpenGL33Context` корректно описывает минимальный контракт GL-контекста без зависимости от GL-типов.
- `OpenGL33ContextForward` как forward-constraint — элегантное решение для разделения объявления и проверки.
- `static_assert` в конструкторе — правильное место для проверки полного концепта.

### 5.2. Explicit instantiation ✅

- [`RendererGlad33.cpp`](projects/lib/skif-rmlui/src/RmlUi/RendererGlad33.cpp) — корректный TU для explicit instantiation.
- [`RendererGlad33.hpp`](projects/lib/skif-rmlui/private/RmlUi/RendererGlad33.hpp) — корректный `extern template` + type alias.
- Реализация скрыта в `.inl`, не утекает в пользовательский API.

### 5.3. Pimpl через `unique_ptr<Impl>` ✅

- Корректное разделение интерфейса и реализации.
- Деструктор определён в `.inl` — `unique_ptr` сможет удалить `Impl`.

### 5.4. Реентерабельность через `Gl_Context_Provider` ✅

- Провайдер вызывается на каждом GL-вызове — обеспечивает корректный контекст.
- Лямбда в [`app.cpp`](projects/lib/skif-rmlui/src/app.cpp:92) вызывает `glfwMakeContextCurrent` — правильный паттерн.

---

## 6. Проблемы в app.cpp

### 6.1. Утечка ресурсов при ошибке загрузки документа

**Файл:** [`app.cpp`](projects/lib/skif-rmlui/src/app.cpp:184) строки 184–191

При переходе на `fail_load_document` вызывается `document->Close()` — но если `document == nullptr`, это UB. Хотя `goto fail_load_document` происходит только из строки 124, где `document` проверен на null, и переход идёт мимо `document->Close()` — но `Rml::RemoveContext` всё равно вызывается. Логика goto-цепочки хрупкая.

### 6.2. `render_impl` не уничтожается при ошибках

При переходе на `fail_init_rmlui` или `fail_load_document`, `render_impl` уничтожается автоматически через `unique_ptr`, но `Rml::Shutdown()` вызывается до этого. Если `Shutdown` пытается использовать render interface — это потенциальный use-after-free. Порядок: сначала `Rml::Shutdown()`, потом деструктор `render_impl` — корректен, если `Shutdown` не вызывает render.

---

## 7. Сводная таблица

| # | Серьёзность | Проблема | Файл |
|---|-------------|----------|------|
| 2.1 | 🔴 Критическая | TGA header без `#pragma pack` | `Impl.inl:290` |
| 2.2 | 🔴 Критическая | `RenderShader` без `override` | `RenderOpenGL33.hpp:145` |
| 3.1 | 🟡 Существенная | `UpdateViewportState` на каждый draw | `Impl.inl:227` |
| 3.2 | 🟡 Средняя | Двойная логика scissor enable | `Impl.inl:429` |
| 3.3 | 🟡 Средняя | `.inl` зависит от GL-макросов | `Impl.inl:17` |
| 3.4 | 🟡 Существенная | Нет проверки размера TGA-данных | `Impl.inl:344` |
| 4.1 | 🟢 Стиль | Шаблонные шейдерные функции | `Impl.inl:75` |
| 4.2 | 🟢 Стиль | Неиспользуемый `VerticallyFlipped` | `Impl.inl:845` |
| 4.3 | 🟢 Стиль | Нет unbind текстуры после draw | `Impl.inl:246` |
| 4.4 | 🟢 Стиль | `protected` в `Impl` без наследников | `Impl.hpp:145` |

---

## 8. Рекомендуемый порядок исправлений

1. **Добавить `#pragma pack(push, 1)` / `#pragma pack(pop)` вокруг `TgaHeader`** — критический баг
2. **Добавить `override` к `RenderShader`** — критический баг
3. **Добавить проверку размера пиксельных данных в `LoadTexture`** — безопасность
4. **Оптимизировать `UpdateViewportState`** — производительность
5. Остальное — по приоритету и желанию
