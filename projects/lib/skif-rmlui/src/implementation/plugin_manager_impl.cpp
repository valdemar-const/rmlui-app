#include <implementation/plugin_manager_impl.hpp>

#include <cassert>

namespace skif::rmlui
{

PluginManagerImpl::~PluginManagerImpl()
{
    Shutdown();
}

bool
PluginManagerImpl::Initialize() noexcept
{
    if (initialized_)
    {
        return true;
    }

    initialized_ = true;
    return true;
}

void
PluginManagerImpl::Shutdown() noexcept
{
    if (!initialized_)
    {
        return;
    }

    StopPlugins();
    plugins_.clear();
    initialized_ = false;
}

bool
PluginManagerImpl::LoadPlugin(std::string_view /*path*/)
{
    // Динамическая загрузка будет реализована позже
    assert(false && "Dynamic plugin loading not implemented yet");
    return false;
}

void
PluginManagerImpl::UnloadPlugin(std::string_view name)
{
    auto it = plugins_.find(std::string(name));
    if (it != plugins_.end())
    {
        if (it->second.started)
        {
            it->second.plugin->OnUnload();
        }
        plugins_.erase(it);
    }
}

void
PluginManagerImpl::StartPlugins()
{
    for (auto& [name, entry] : plugins_)
    {
        if (!entry.started)
        {
            entry.plugin->OnLoad(*this);
            entry.started = true;
        }
    }
}

void
PluginManagerImpl::StopPlugins() noexcept
{
    for (auto& [name, entry] : plugins_)
    {
        if (entry.started)
        {
            entry.plugin->OnUnload();
            entry.started = false;
        }
    }
}

void
PluginManagerImpl::RegisterPlugin(std::unique_ptr<IPlugin> plugin)
{
    if (!plugin)
    {
        return;
    }

    const std::string name = std::string(plugin->GetName());
    plugins_[name] = PluginEntry{std::move(plugin), false};
}

IPlugin*
PluginManagerImpl::GetPlugin(std::string_view name) const
{
    auto it = plugins_.find(std::string(name));
    if (it != plugins_.end())
    {
        return it->second.plugin.get();
    }
    return nullptr;
}

std::vector<IPlugin*>
PluginManagerImpl::GetPlugins() const
{
    std::vector<IPlugin*> result;
    result.reserve(plugins_.size());

    for (const auto& [name, entry] : plugins_)
    {
        result.push_back(entry.plugin.get());
    }

    return result;
}

} // namespace skif::rmlui