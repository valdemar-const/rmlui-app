#pragma once

#include <skif/rmlui/config.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace skif::rmlui
{

/**
 * @brief Запись меню редактора
 */
struct MenuEntry
{
    std::string label;     // Отображаемый текст: "Save", "Undo"
    std::string action_id; // Идентификатор действия: "editor.save"
    std::string shortcut;  // Горячая клавиша: "Ctrl+S", "F5"

    MenuEntry() = default;

    MenuEntry(std::string_view label, std::string_view action_id, std::string_view shortcut = {})
        : label(label)
        , action_id(action_id)
        , shortcut(shortcut)
    {
    }
};

/**
 * @brief Дескриптор редактора
 * @note Описывает метаданные редактора для регистрации в системе.
 *       Каждый Editor — самодостаточная единица с view, menu и логикой.
 */
struct EditorDescriptor
{
    std::string name;         // Уникальный идентификатор: "3d_viewport", "properties"
    std::string display_name; // Отображаемое имя: "3D Viewport", "Properties"
    std::string rml_path;     // Путь к RML файлу
    std::string rcss_path;    // Путь к RCSS файлу стилей (опционально)
    std::string icon;         // Иконка для меню выбора редактора (опционально)
    std::string category;     // Категория: "General", "Animation", "Modeling"

    /// Записи меню редактора (header bar)
    std::vector<MenuEntry> menu_entries;

    EditorDescriptor() = default;

    EditorDescriptor(
            std::string_view name,
            std::string_view display_name,
            std::string_view rml_path,
            std::string_view category = "General"
    )
        : name(name)
        , display_name(display_name.empty() ? std::string(name) : std::string(display_name))
        , rml_path(rml_path)
        , category(category)
    {
    }
};

} // namespace skif::rmlui
