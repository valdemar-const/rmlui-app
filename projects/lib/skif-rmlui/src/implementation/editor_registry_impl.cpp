#include <implementation/editor_registry_impl.hpp>

#include <unordered_set>

namespace skif::rmlui
{

void
EditorRegistryImpl::RegisterEditor(EditorDescriptor descriptor, EditorFactory factory)
{
    if (!factory || descriptor.name.empty())
    {
        return;
    }

    const std::string name = descriptor.name;
    editors_[name]         = EditorEntry {std::move(descriptor), std::move(factory)};
}

std::unique_ptr<IEditor>
EditorRegistryImpl::CreateEditor(std::string_view name) const
{
    auto it = editors_.find(std::string(name));
    if (it != editors_.end() && it->second.factory)
    {
        return it->second.factory();
    }
    return nullptr;
}

const EditorDescriptor *
EditorRegistryImpl::GetDescriptor(std::string_view name) const
{
    auto it = editors_.find(std::string(name));
    if (it != editors_.end())
    {
        return &it->second.descriptor;
    }
    return nullptr;
}

std::vector<const EditorDescriptor *>
EditorRegistryImpl::GetAllDescriptors() const
{
    std::vector<const EditorDescriptor *> result;
    result.reserve(editors_.size());

    for (const auto &[name, entry] : editors_)
    {
        result.push_back(&entry.descriptor);
    }

    return result;
}

std::vector<const EditorDescriptor *>
EditorRegistryImpl::GetDescriptorsByCategory(std::string_view category) const
{
    std::vector<const EditorDescriptor *> result;

    for (const auto &[name, entry] : editors_)
    {
        if (entry.descriptor.category == category)
        {
            result.push_back(&entry.descriptor);
        }
    }

    return result;
}

std::vector<std::string>
EditorRegistryImpl::GetCategories() const
{
    std::vector<std::string>        categories;
    std::unordered_set<std::string> unique_categories;

    for (const auto &[name, entry] : editors_)
    {
        unique_categories.insert(entry.descriptor.category);
    }

    categories.reserve(unique_categories.size());
    for (auto &cat : unique_categories)
    {
        categories.push_back(std::move(cat));
    }

    return categories;
}

} // namespace skif::rmlui
