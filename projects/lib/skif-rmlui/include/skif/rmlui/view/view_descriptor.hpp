#pragma once

#include <skif/rmlui/config.hpp>

#include <string>
#include <string_view>

namespace skif::rmlui
{

/**
 * @brief Дескриптор представления (view)
 * @note Описывает метаданные view для регистрации в системе
 */
struct ViewDescriptor
{
    std::string name;           // Уникальное имя view
    std::string rml_path;       // Путь к RML файлу
    std::string category;       // Категория (для UI, напр. "Panels", "Dialogs")
    std::string display_name;   // Отображаемое имя в UI
    std::string icon;           // Иконка (опционально)

    ViewDescriptor() = default;
    
    ViewDescriptor(
        std::string_view name,
        std::string_view rml_path,
        std::string_view category = "General",
        std::string_view display_name = {}
    )
        : name(name)
        , rml_path(rml_path)
        , category(category)
        , display_name(display_name.empty() ? name : display_name)
    {}
};

} // namespace skif::rmlui