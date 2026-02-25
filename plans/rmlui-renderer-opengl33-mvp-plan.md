# План портирования базового рендерера в `RendererOpenGL33`

## 1. Цель текущей итерации

Сделать **MVP базового OpenGL33-рендерера** для `Rml::RenderInterface` в существующей шаблонной архитектуре:
- сохранить скрытие реализации в `.inl`;
- поддержать явное инстанцирование в отдельном TU;
- оставить API реентерабельным через внешний провайдер GL-контекста;
- реализовать только базовые функции до `SetScissorRegion` включительно;
- функции интерфейса после `SetScissorRegion` оставить заглушками на следующий этап.

## 2. Зафиксированные ограничения

1. Концепт для GL-контекста не должен явно зависеть от OpenGL-заголовков/типов.
2. Пользователь может передавать свой тип GL-контекста как параметр шаблона.
3. Реализация должна поддерживать паттерн explicit instantiation в отдельном TU.
4. Сборка и проверка выполняются через `cmake --build out/Debug`.
5. Приоритет — архитектурная чистота + рабочий минимум рендеринга.

## 3. Функциональный объём MVP

### Включено в MVP
- `CompileGeometry`
- `RenderGeometry`
- `ReleaseGeometry`
- `LoadTexture` (TGA 24/32 + premultiplied alpha)
- `GenerateTexture`
- `ReleaseTexture`
- `EnableScissorRegion`
- `SetScissorRegion`
- `SetTransform`

### Временно отложено
- `EnableClipMask` и далее по интерфейсу (`PushLayer`, `CompositeLayers`, фильтры, шейдеры и т.д.)

## 4. Поэтапная реализация

## Этап A — Каркас внутренних данных и базового GL-state

**Цель:** подготовить основу в `Impl` для базового рендеринга без слоёв/фильтров.

Шаги:
1. Добавить в `Impl` внутренние структуры:
   - `CompiledGeometryData` (`vao`, `vbo`, `ibo`, `draw_count`),
   - `ProgramData` для минимального набора программ (`Color`, `Texture`),
   - таблицу uniform-локаций (`_transform`, `_translate`, `_tex`),
   - runtime-состояние (`active_program`, `transform`, `projection`, `viewport`, `scissor_state`, флаг dirty для трансформа).
2. Добавить безопасные конвертеры handle↔указатель через `reinterpret_cast` в изолированных helper-функциях.
3. Добавить минимальные helper-методы для:
   - компиляции/линковки шейдеров,
   - `UseProgram`,
   - `SubmitTransformUniform`,
   - проверки GL-ошибок в debug-сборке.

Критерий готовности:
- В `Impl` есть полный несущий каркас для Geometry+Texture пути.

---

## Этап B — Геометрия и отрисовка (без пост-эффектов)

**Цель:** получить фактический вывод RmlUi-геометрии в OpenGL 3.3.

Шаги:
1. Перенести/адаптировать логику `CompileGeometry`:
   - создание VAO/VBO/IBO,
   - `VertexAttribPointer` под `Rml::Vertex` (`position`, `colour`, `tex_coord`),
   - заполнение буферов.
2. Реализовать `RenderGeometry`:
   - выбор программы `Color`/`Texture` в зависимости от texture handle,
   - биндинг текстуры,
   - загрузка трансформации и translation,
   - `glDrawElements`.
3. Реализовать `ReleaseGeometry`.

Критерий готовности:
- Приложение рендерит базовые цветные/текстурированные примитивы без падений.

---

## Этап C — Текстуры и загрузка TGA

**Цель:** поддержать полный базовый путь загрузки и генерации текстур.

Шаги:
1. Реализовать `GenerateTexture`:
   - создание `GL_TEXTURE_2D`,
   - загрузка RGBA8,
   - базовые параметры фильтрации и wrap.
2. Реализовать `LoadTexture` в стиле референса:
   - чтение через `Rml::FileInterface`,
   - парсинг TGA header,
   - поддержка uncompressed 24/32 bpp,
   - BGR→RGB,
   - вертикальный флип,
   - premultiplied alpha.
3. Реализовать `ReleaseTexture`.

Критерий готовности:
- Шрифты/изображения из RmlUi корректно отображаются, альфа-композиция без артефактов.

---

## Этап D — Scissor + Transform + viewport-связка

**Цель:** довести базовый интерфейс до стабильного состояния для UI-потока.

Шаги:
1. `SetTransform`: хранение текущей матрицы, пометка dirty для uniform.
2. `SetScissorRegion`: конвертация координат RmlUi→OpenGL и `glScissor`.
3. `EnableScissorRegion`: включение/выключение `GL_SCISSOR_TEST`.
4. Уточнить источник размеров viewport:
   - либо через отдельный метод инициализации внутри `Impl`,
   - либо через lazy-чтение текущего `glViewport` при первом рендере.
5. Проверить корректность проекционной матрицы (`ProjectOrtho`) относительно текущего viewport.

Критерий готовности:
- Обрезка и transform работают предсказуемо в демо-сценарии.

---

## Этап E — Заглушки для остального интерфейса + явная диагностика

**Цель:** не ломать контракт `RenderInterface`, но явно фиксировать неполную реализацию.

Шаги:
1. Для методов после `SetScissorRegion` оставить безопасные заглушки:
   - возвращать пустые handle,
   - no-op там, где допустимо,
   - логировать `LT_WARNING` с явной пометкой `Not implemented in MVP`.
2. Убедиться, что вызовы заглушек не приводят к undefined behavior.

Критерий готовности:
- Интерфейс полный, MVP-функции рабочие, отложенные зоны явно помечены.

---

## Этап F — Стабилизация шаблонной архитектуры и explicit instantiation

**Цель:** закрепить архитектурный результат, ради которого затевался перенос.

Шаги:
1. Проверить, что implementation остаётся в `.inl` и не утекает в публичный API сверх необходимого.
2. Убедиться, что `RendererGlad33.cpp` продолжает явное инстанцирование:
   - `template class Rml::RendererOpenGL33<GladGLContext>;`
3. Сохранить `static_assert`-валидацию концепта на этапе инстанцирования.
4. Проверить, что минимум зависимостей из GL-заголовков остаётся в TU инстанцирования, а не в пользовательском API.

Критерий готовности:
- Достигаются: скрытие реализации, explicit instantiation, проверяемая типобезопасность.

## 5. Проверка и приёмка

Проверки после каждого этапа:
1. `cmake --build out/Debug`
2. Запуск `rmlui-app` и визуальная проверка:
   - базовая геометрия;
   - текстуры;
   - clipping/scissor;
   - отсутствие регрессий запуска/инициализации.

## 6. План продолжения после MVP

Следующий документ создаётся после MVP и включает:
- `EnableClipMask` + `RenderToClipMask`,
- layer stack + framebuffer pipeline,
- filters (`blur`, `drop-shadow`, `color-matrix`),
- shaders (`gradient`, `creation`),
- optional state backup/restore как в `RenderInterface_GL3`.

## 7. Чеклист статуса (для возобновления в любой момент)

- [x] Этап A завершён
- [x] Этап B завершён
- [x] Этап C завершён
- [x] Этап D завершён
- [x] Этап E завершён
- [x] Этап F завершён
- [x] Сборка `out/Debug` успешна
- [ ] Визуальная проверка `rmlui-app` пройдена

## 8. Реальный статус после текущей сессии

Сделано в коде:
- реализован MVP в `RendererOpenGL33::Impl` для:
  - `CompileGeometry`, `RenderGeometry`, `ReleaseGeometry`;
  - `LoadTexture` (TGA 24/32, BGR→RGB, vertical flip, premultiplied alpha);
  - `GenerateTexture`, `ReleaseTexture`;
  - `EnableScissorRegion`, `SetScissorRegion`, `SetTransform`.
- добавлены минимальные shader/program helper'ы и uniform-пайплайн (`_transform`, `_translate`, `_tex`).
- оставлены явные заглушки + предупреждения для функций после `SetScissorRegion`.
- сохранён explicit instantiation в отдельном TU (`RendererGlad33.cpp`) и убрана утечка реализации через compile-definition из интерфейсной библиотеки.

Технические замечания:
- текущий `OpenGL33Context` намеренно выражен без прямой зависимости от OpenGL-типов в API и проверяется на инстанцировании через `static_assert`.
- `RenderOpenGL33.hpp` теперь требует `<functional>`/`<memory>` для корректного публичного контракта.

Проверка:
- `cmake --build out/Debug` — успешно.
- Визуальный smoke-test приложения ещё не выполнен в этой сессии.
