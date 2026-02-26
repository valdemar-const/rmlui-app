#include <implementation/view_registry_impl.hpp>

#include <unordered_set>

namespace skif::rmlui
{

void
ViewRegistryImpl::RegisterView(ViewDescriptor descriptor, ViewFactory factory)
{
    if (!factory)
    {
        return;
    }

    const std::string name = std::string(descriptor.name);
    views_[name] = ViewEntry{std::move(descriptor), std::move(factory)};
}

std::unique_ptr<IView>
ViewRegistryImpl::CreateView(std::string_view name) const
{
    auto it = views_.find(std::string(name));
    if (it != views_.end() && it->second.factory)
    {
        return it->second.factory();
    }
    return nullptr;
}

const ViewDescriptor*
ViewRegistryImpl::GetDescriptor(std::string_view name) const
{
    auto it = views_.find(std::string(name));
    if (it != views_.end())
    {
        return &it->second.descriptor;
    }
    return nullptr;
}

std::vector<const ViewDescriptor*>
ViewRegistryImpl::GetAllDescriptors() const
{
    std::vector<const ViewDescriptor*> result;
    result.reserve(views_.size());

    for (const auto& [name, entry] : views_)
    {
        result.push_back(&entry.descriptor);
    }

    return result;
}

std::vector<const ViewDescriptor*>
ViewRegistryImpl::GetDescriptorsByCategory(std::string_view category) const
{
    std::vector<const ViewDescriptor*> result;

    for (const auto& [name, entry] : views_)
    {
        if (entry.descriptor.category == category)
        {
            result.push_back(&entry.descriptor);
        }
    }

    return result;
}

std::vector<std::string>
ViewRegistryImpl::GetCategories() const
{
    std::vector<std::string> categories;
    std::unordered_set<std::string> unique_categories;

    for (const auto& [name, entry] : views_)
    {
        unique_categories.insert(entry.descriptor.category);
    }

    categories.reserve(unique_categories.size());
    for (const auto& cat : unique_categories)
    {
        categories.push_back(cat);
    }

    return categories;
}

} // namespace skif::rmlui