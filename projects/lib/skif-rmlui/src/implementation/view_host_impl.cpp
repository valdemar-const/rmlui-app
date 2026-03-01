#include <implementation/view_host_impl.hpp>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Log.h>

namespace skif::rmlui
{

ViewHostImpl::ViewHostImpl(IViewRegistry& registry)
    : registry_(registry)
{}

void
ViewHostImpl::SetContext(Rml::Context* context)
{
    context_ = context;
}

bool
ViewHostImpl::AttachView(std::string_view view_name, Rml::Element* container)
{
    if (!context_)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "ViewHost: No RmlUi context set.");
        return false;
    }

    // Check if view already attached
    const std::string name_str(view_name);
    if (attached_views_.contains(name_str))
    {
        Rml::Log::Message(Rml::Log::LT_WARNING, "ViewHost: View '%s' is already attached.", view_name.data());
        return false;
    }

    // Get descriptor from registry
    const ViewDescriptor* descriptor = registry_.GetDescriptor(view_name);
    if (!descriptor)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "ViewHost: View '%s' not found in registry.", view_name.data());
        return false;
    }

    // Create view instance
    std::unique_ptr<IView> view = registry_.CreateView(view_name);
    if (!view)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "ViewHost: Failed to create view '%s'.", view_name.data());
        return false;
    }

    // Load RML document
    Rml::ElementDocument* document = context_->LoadDocument(descriptor->rml_path.c_str());
    if (!document)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "ViewHost: Failed to load RML '%s' for view '%s'.",
            descriptor->rml_path.c_str(), view_name.data());
        return false;
    }

    // Attach document to container if provided
    if (container)
    {
        // RmlUi 6.2 requires unique_ptr for AppendChild
        container->AppendChild(Rml::ElementPtr(document));
    }

    // Call OnCreated
    view->OnCreated(document);

    // Store attached view
    AttachedView attached;
    attached.view      = std::move(view);
    attached.document = document;
    attached.container = container;
    attached.visible   = false; // Start hidden

    attached_views_[name_str] = std::move(attached);

    return true;
}

void
ViewHostImpl::DetachView(Rml::Element* container)
{
    // Find view by container
    for (auto it = attached_views_.begin(); it != attached_views_.end(); ++it)
    {
        if (it->second.container == container)
        {
            // Call OnDestroyed
            it->second.view->OnDestroyed();

            // Close document
            if (it->second.document)
            {
                it->second.document->Close();
            }

            attached_views_.erase(it);
            return;
        }
    }
}

IView*
ViewHostImpl::GetActiveView() const
{
    auto it = attached_views_.find(active_view_name_);
    if (it != attached_views_.end())
    {
        return it->second.view.get();
    }
    return nullptr;
}

void
ViewHostImpl::ShowView(std::string_view name)
{
    const std::string name_str(name);
    auto it = attached_views_.find(name_str);
    if (it == attached_views_.end())
    {
        Rml::Log::Message(Rml::Log::LT_WARNING, "ViewHost: View '%s' not found.", name.data());
        return;
    }

    if (!it->second.visible)
    {
        if (it->second.document)
        {
            it->second.document->Show();
        }
        it->second.view->OnShow();
        it->second.visible = true;
    }

    active_view_name_ = name_str;
}

void
ViewHostImpl::HideView(std::string_view name)
{
    const std::string name_str(name);
    auto it = attached_views_.find(name_str);
    if (it == attached_views_.end())
    {
        return;
    }

    if (it->second.visible)
    {
        if (it->second.document)
        {
            it->second.document->Hide();
        }
        it->second.view->OnHide();
        it->second.visible = false;
    }

    if (active_view_name_ == name_str)
    {
        active_view_name_.clear();
    }
}

void
ViewHostImpl::Update(float delta_time)
{
    for (auto& [name, attached] : attached_views_)
    {
        if (attached.visible)
        {
            attached.view->OnUpdate(delta_time);
        }
    }
}

} // namespace skif::rmlui