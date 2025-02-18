// minecraft_classes.hpp
#pragma once
#include "mapper/mapper.hpp"
#include "interop/java_interop.hpp"

class mapped_class_t
{
public:
    mapped_class_t();

    bool operator==(std::nullptr_t) const;
    bool operator!=(std::nullptr_t) const;
    bool operator!() const;

    explicit operator bool() const;

    java_object_t instance() const;
    
protected:
    java_object_t instance_object_;

    static java_method_t get_mapped_method(std::shared_ptr<java_class_t> class_ref, const std::string& mcp_name, const std::string& mcp_signature = "");
    static java_field_t get_mapped_field(std::shared_ptr<java_class_t> class_ref, const std::string& mcp_name, const std::string& mcp_signature = "");

    static jclass load_mapped_class(const std::string& mcp_class_name);

private:
};

#define INITIALIZE_CLASS(class_instance, name) \
if (class_instance == nullptr){ \
    jclass class_ref = load_mapped_class(name); \
    if (class_ref == nullptr) { \
        return; \
    } \
    class_instance = std::make_shared<java_class_t>(class_ref);\
}\

#define FIND_CLASS(class_instance, name) \
if (class_instance == nullptr){ \
    jclass class_ref = java_interop_manager->find_class_s(name); \
    if (class_ref == nullptr) { \
        return; \
    } \
    class_instance = std::make_shared<java_class_t>(class_ref);\
}\


