#include "pch.hpp"
#include "mapped_class.hpp"

mapped_class_t::mapped_class_t() {}

bool mapped_class_t::operator==(std::nullptr_t) const {
    return instance_object_ == nullptr;
}

bool mapped_class_t::operator!=(std::nullptr_t) const {
    return instance_object_ != nullptr;
}

bool mapped_class_t::operator!() const {
    return instance_object_ == nullptr;
}

mapped_class_t::operator bool() const {
    return instance_object_ != nullptr;
}

java_object_t mapped_class_t::instance() const
{
    return instance_object_;
}

java_method_t mapped_class_t::get_mapped_method(std::shared_ptr<java_class_t> class_ref, const std::string& mcp_name, const std::string& mcp_signature)
{
    if (ctx.is_mcp()) {
        return class_ref->get_method(mcp_name, mcp_signature);
    }

    std::string class_name = class_ref->get_class_name();

    if (ctx.is_obf())
    {
        std::string mcp_class_name = mapper->get_original_class_name(class_name);

        auto obf_method = mapper->get_method_mapping(
            mcp_class_name, mcp_name
        );

        return class_ref->get_method(obf_method);
    }

    return java_method_t();
}

java_field_t mapped_class_t::get_mapped_field(std::shared_ptr<java_class_t> class_ref, const std::string& mcp_name, const std::string& mcp_signature)
{
    if (ctx.is_mcp()) {
        return class_ref->get_field(mcp_name, mcp_signature);
    }

    std::string class_name = class_ref->get_class_name();

    if (ctx.is_obf())
    {
        std::string mcp_class_name = mapper->get_original_class_name(class_name);

        auto obf_field = mapper->get_field_mapping(
            mcp_class_name, mcp_name
        );

        return class_ref->get_field(obf_field);
    }

    return java_field_t();
}

jclass mapped_class_t::load_mapped_class(const std::string& mcp_class_name)
{
    std::string class_name = mcp_class_name;

    if (ctx.is_obf()) {
        std::replace(class_name.begin(), class_name.end(), '/', '.');
        class_name = mapper->get_class_mapping(mcp_class_name);
    }

    return java_interop_manager->load_class(class_name);
}
