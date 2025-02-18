#include "../java_interop.hpp"

java_class_t::java_class_t(jclass clazz) : java_object_t(clazz)
{
    JNIEnv* env = java_interop_manager->get_env();
    jvmtiEnv* jvmti = java_interop_manager->get_jvmti();

    make_global();

    jvmtiPhase phase;
    jvmtiError err = jvmti->GetPhase(&phase);

    while (phase != JVMTI_PHASE_LIVE) {
        err = jvmti->GetPhase(&phase);
        Sleep(100);
    }

    class_name_ = java_interop_manager->get_class_name_jvmti(class_, true);
    class_name_full_ = java_interop_manager->get_class_name_jvmti(class_);

    err = jvmti->GetClassStatus(class_, &class_state_);
    if (err != JVMTI_ERROR_NONE)
    {
        char* msg;
        jvmti->GetErrorName(err, &msg);

        std::cout << "jvmti-> " << msg << std::endl;
        //EXCEPTION("Failed to retreive class status");
        return;
    }

    jint class_method_count;
    jmethodID* class_methods;

    err = jvmti->GetClassMethods(class_, &class_method_count, &class_methods);
    if (err != JVMTI_ERROR_NONE)
    {
        char* msg;
        jvmti->GetErrorName(err, &msg);

        std::cout << "jvmti-> " << msg << std::endl;
        //EXCEPTION("Failed to retreive class methods");
        return;
    }

    for (int i = 0; i < class_method_count; i++)
    {
        java_method_t method(class_methods[i]);

        std::string method_name = method.get_name();

        methods_[ java_mapping{ method_name, method.get_signature() } ] = method;
        methods_by_name_[ method_name ] = method;
    }

    jint class_field_count;
    jfieldID* class_fields;

    err = jvmti->GetClassFields(class_, &class_field_count, &class_fields);
    if (err != JVMTI_ERROR_NONE)
    {
        char* msg;
        jvmti->GetErrorName(err, &msg);

        std::cout << "jvmti-> " << msg << std::endl;
        //EXCEPTION("Failed to retreive class fields");
        return;
    }

    for (int i = 0; i < class_field_count; i++)
    {
        java_field_t field(clazz, class_fields[i]);

        std::string field_name = field.get_name();

        fields_[ java_mapping{ field.get_name(), field.get_signature() } ] = field;
        fields_by_name_[field_name] = field;
    }
}

java_class_t::java_class_t(const java_class_t& other)
    : java_object_t(other), class_name_(other.class_name_), class_name_full_(other.class_name_full_), class_state_(other.class_state_)
{
    JNIEnv* env = java_interop_manager->get_env();
    if (env) {
        class_ = static_cast<jclass>(env->NewGlobalRef(other.class_));
    }

    methods_ = other.methods_;
    fields_ = other.fields_;
}

java_class_t::java_class_t(java_class_t&& other) noexcept
    : java_object_t(std::move(other)), class_name_(std::move(other.class_name_)), class_name_full_(std::move(other.class_name_full_)),
    class_state_(other.class_state_), methods_(std::move(other.methods_)), fields_(std::move(other.fields_))
{
    class_ = other.class_;
    other.class_ = nullptr;  // Null out the reference in the moved-from object
}

java_class_t& java_class_t::operator=(const java_class_t& other)
{
    if (this != &other) {
        return *this;
    }

    JNIEnv* env = java_interop_manager->get_env();
    if (env && class_) {
        env->DeleteGlobalRef(class_);
    }

    java_object_t::operator=(other);
    class_name_ = other.class_name_;
    class_name_full_ = other.class_name_full_;
    class_state_ = other.class_state_;
    methods_ = other.methods_;
    fields_ = other.fields_;

    if (env) {
        class_ = static_cast<jclass>(env->NewGlobalRef(other.class_));
    }

}

java_class_t& java_class_t::operator=(java_class_t&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    JNIEnv* env = java_interop_manager->get_env();
    if (env && class_) {
        env->DeleteGlobalRef(class_);
    }

    java_object_t::operator=(std::move(other));
    class_name_ = std::move(other.class_name_);
    class_name_full_ = std::move(other.class_name_full_);
    class_state_ = other.class_state_;
    methods_ = std::move(other.methods_);
    fields_ = std::move(other.fields_);

    class_ = other.class_;
    other.class_ = nullptr;
}

void java_class_t::debug_print() const
{
    std::cout << "Class Debug Information:" << std::endl;
    std::cout << "--------------------------" << std::endl;
    std::cout << "Class Name: " << class_name_ << std::endl;
    std::cout << "Full Class Name: " << class_name_full_ << std::endl;

    std::cout << "Class State: " << class_state_ << std::endl;

    std::cout << "Methods:" << std::endl;
    if (!methods_.empty())
    {
        for (const auto& method_pair : methods_)
        {
            std::cout << "  " << method_pair.first.name << ":" << std::endl;
            method_pair.second.debug_print();  // debug_print() exists in java_method_t
        }
    }
    else {
        std::cout << "  No Methods" << std::endl;
    }

    std::cout << "Fields:" << std::endl;
    if (!fields_.empty())
    {
        for (const auto& field_pair : fields_)
        {
            std::cout << "  " << field_pair.first.name << ":" << std::endl;
            field_pair.second.debug_print();  // debug_print() exists in java_field_t
        }
    }
    else {
        std::cout << "  No Fields" << std::endl;
    }

    std::cout << "--------------------------" << std::endl;
}

std::string java_class_t::get_class_name(bool trimmed) const
{
    const std::string& source = trimmed ? class_name_ : class_name_full_;
    std::string ret = source;
    std::replace(ret.begin(), ret.end(), '.', '/');

    return ret;
}

java_method_t java_class_t::get_method(const std::string& name, const std::string& signature) const
{
    check_thread_safety();
    check_instance_safety();

    if (signature.empty()) 
    {
        auto it = methods_by_name_.find(name);
        if (it != methods_by_name_.end()) {
            return it->second;
        }

        return java_method_t();
    }

    auto it = methods_.find( java_mapping{ name, signature } );
    if (it != methods_.end()) {
        return it->second;
    }

    return java_method_t();
}

java_field_t java_class_t::get_field(const std::string& name, const std::string& signature) const
{
    check_thread_safety();
    check_instance_safety();

    if (signature.empty())
    {
        auto it = fields_by_name_.find(name);
        if (it != fields_by_name_.end()) {
            return it->second;
        }

        return java_field_t();
    }

    auto it = fields_.find(java_mapping{ name, signature });
    if (it != fields_.end()) {
        return it->second;
    }

    return java_field_t();
}

jclass java_class_t::clazz() const
{
    return class_;
}

java_class_t::~java_class_t()
{
    JNIEnv* env = java_interop_manager->get_env();

    if (!class_ || !env) {
        return;
    }

    env->DeleteGlobalRef(class_);
}