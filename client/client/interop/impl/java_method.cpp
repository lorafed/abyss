#include "../java_interop.hpp"

java_method_t::java_method_t(jmethodID method_id) : method_id_(method_id)
{
    if (method_id == nullptr) {
        return;
    }

    JNIEnv* env = java_interop_manager->get_env();
    jvmtiEnv* jvmti = java_interop_manager->get_jvmti();

    if (env == nullptr)
    {
        THROW_MESSAGE_EXCEPTION("No associated JNI thread environment");
        return;
    }

    if (jvmti == nullptr)
    {
        THROW_MESSAGE_EXCEPTION("No associated JVMTI environment");
        return;
    }

    jclass declaring_class = nullptr;

    jvmtiError err = jvmti->GetMethodDeclaringClass(method_id_, &declaring_class);
    if (err != JVMTI_ERROR_NONE)
    {
        THROW_MESSAGE_EXCEPTION("Failed to retrieve method declaring class");
        return;
    }

    declaring_class_ = reinterpret_cast<jclass>(env->NewGlobalRef(declaring_class));

    char* method_name = nullptr;
    char* method_signature = nullptr;
    char* method_signature_generic = nullptr;

    err = jvmti->GetMethodName(method_id_, &method_name, &method_signature, &method_signature_generic);
    if (err != JVMTI_ERROR_NONE)
    {
        THROW_MESSAGE_EXCEPTION("Failed to retrieve method name or signature");
        return;
    }

    jint method_modifiers;
    err = jvmti->GetMethodModifiers(method_id_, &method_modifiers);
    if (err != JVMTI_ERROR_NONE)
    {
        THROW_MESSAGE_EXCEPTION("Failed to retrieve method modifiers");
        return;
    }

    is_static_ = method_modifiers & JVM_ACC_STATIC;

    method_name_ = method_name;
    method_signature_ = method_signature;

   // std::cout << "new method allocated: " << method_name_ << std::endl;

    jvmti->Deallocate(reinterpret_cast<unsigned char*>(method_name));
    jvmti->Deallocate(reinterpret_cast<unsigned char*>(method_signature));

    if (method_signature_generic != nullptr)
    {
        jvmti->Deallocate(reinterpret_cast<unsigned char*>(method_signature_generic));
    }

    method_declaring_class_name_ = java_interop_manager->get_class_name_jvmti(declaring_class_, false);

    java_method_info parsed_data = parse_method_signature(method_signature_);

    switch (parsed_data.return_type)
    {
    case java_type_t::JAVA_OBJECT:
        return_type_ = java_type_descriptor{ parsed_data.return_type, parsed_data.optional };
        break;
    case java_type_t::JAVA_ARRAY:
        return_type_ = java_type_descriptor{ parsed_data.return_type, parsed_data.optional };
        break;
    default:
        return_type_ = java_type_descriptor{ parsed_data.return_type };
        break;
    }

    for (const auto& argument : parsed_data.argument_types)
    {
        switch (argument.type)
        {
        case java_type_t::JAVA_OBJECT:
            arguments_.push_back(java_type_descriptor{ argument.type, argument.optional });
            break;
        case java_type_t::JAVA_ARRAY:
            arguments_.push_back(java_type_descriptor{ argument.type, argument.optional });
            break;
        default:
            arguments_.push_back(java_type_descriptor{ argument.type });
            break;
        }
    }

    num_arguments_ = parsed_data.num_arguments;
}

java_method_t::java_method_t(const java_method_t& other)
    : method_id_(other.method_id_),
    is_static_(other.is_static_),
    method_name_(other.method_name_),
    method_signature_(other.method_signature_),
    method_declaring_class_name_(other.method_declaring_class_name_),
    method_name_mapped_(other.method_name_mapped_),
    method_signature_mapped_(other.method_signature_mapped_),
    num_arguments_(other.num_arguments_),
    return_type_(other.return_type_),
    arguments_(other.arguments_) 
    {
        JNIEnv* env = java_interop_manager->get_env();
        declaring_class_ = static_cast<jclass>(env->NewGlobalRef(other.declaring_class_));
    }

java_method_t::java_method_t(java_method_t&& other) noexcept
    : method_id_(std::exchange(other.method_id_, nullptr)),
    declaring_class_(std::exchange(other.declaring_class_, nullptr)),
    is_static_(other.is_static_),
    method_name_(std::move(other.method_name_)),
    method_signature_(std::move(other.method_signature_)),
    method_declaring_class_name_(std::move(other.method_declaring_class_name_)),
    method_name_mapped_(std::move(other.method_name_mapped_)),
    method_signature_mapped_(std::move(other.method_signature_mapped_)),
    num_arguments_(other.num_arguments_),
    return_type_(std::move(other.return_type_)),
    arguments_(std::move(other.arguments_)) {}

bool java_method_t::operator==(std::nullptr_t) const {
    return method_id_ == nullptr;
}

bool java_method_t::operator!=(std::nullptr_t) const {
    return !(*this == nullptr);
}

bool java_method_t::operator!() const
{
    return method_id_ == nullptr;
}

java_method_t::operator bool() const
{
    return method_id_ != nullptr;
}

java_method_t::operator jmethodID() const
{
    return method_id_;
}

java_method_t& java_method_t::operator=(const java_method_t& other)
{
    if (this == &other) {
        return *this;
    }

    method_id_ = other.method_id_; 
    
    JNIEnv* env = java_interop_manager->get_env();
    env->DeleteGlobalRef(declaring_class_); 
    declaring_class_ = static_cast<jclass>(env->NewGlobalRef(other.declaring_class_));

    is_static_ = other.is_static_;
    method_name_ = other.method_name_;
    method_signature_ = other.method_signature_;
    method_declaring_class_name_ = other.method_declaring_class_name_;
    method_name_mapped_ = other.method_name_mapped_;
    method_signature_mapped_ = other.method_signature_mapped_;
    num_arguments_ = other.num_arguments_;
    return_type_ = other.return_type_;
    arguments_ = other.arguments_;

    return *this;
}

java_method_t& java_method_t::operator=(java_method_t&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    method_id_ = std::exchange(other.method_id_, nullptr);
    declaring_class_ = std::exchange(other.declaring_class_, nullptr);
    is_static_ = other.is_static_;
    method_name_ = std::move(other.method_name_);
    method_signature_ = std::move(other.method_signature_);
    method_declaring_class_name_ = std::move(other.method_declaring_class_name_);
    method_name_mapped_ = std::move(other.method_name_mapped_);
    method_signature_mapped_ = std::move(other.method_signature_mapped_);
    num_arguments_ = other.num_arguments_;
    return_type_ = std::move(other.return_type_);
    arguments_ = std::move(other.arguments_);

    return *this;
}

std::string java_method_t::get_name(bool mapped) const
{
    if (mapped) return method_name_mapped_;
    return method_name_;
}

std::string java_method_t::get_signature(bool mapped) const
{
    if (mapped) return method_signature_mapped_;
    return method_signature_;
}

std::vector<java_type_descriptor> java_method_t::arguments() const
{
    return arguments_;
}

java_type_descriptor java_method_t::return_type() const
{
    return return_type_;
}

int java_method_t::num_arguments() const
{
    return num_arguments_;
}

bool java_method_t::is_static() const
{
    return is_static_;
}

void java_method_t::set_mapped_name(const std::string& name)
{
    if (!name.empty()) {
        method_name_mapped_ = name;
    }
    else {
        method_name_mapped_ = method_name_;
    }
}

void java_method_t::set_mapped_signature(const std::string& signature)
{
    if (!signature.empty()) {
        method_signature_mapped_ = signature;
    }
    else {
        method_signature_mapped_ = method_signature_;
    }
}

void java_method_t::set_mappings(const std::string& name, const std::string& signature)
{
    if (!name.empty()) {
        method_name_mapped_ = name;
    }
    else {
        method_name_mapped_ = method_name_;
    }

    if (!signature.empty()) {
        method_signature_mapped_ = signature;
    }
    else {
        method_signature_mapped_ = method_signature_;
    }
}

void java_method_t::debug_print() const
{
    std::cout << "Method Debug Information:" << std::endl;
    std::cout << "--------------------------" << std::endl;
    std::cout << "Declaring Class: " << method_declaring_class_name_ << std::endl;
    std::cout << "Method Name: " << method_name_ << std::endl;
    std::cout << "Method Signature: " << method_signature_ << std::endl;
    std::cout << "Return Type: " << return_type_.to_string() << std::endl;
    std::cout << "Number of Arguments: " << num_arguments_ << std::endl;

    if (!arguments_.empty())
    {
        std::cout << "Arguments:" << std::endl;
        for (size_t i = 0; i < arguments_.size(); ++i)
        {
            std::cout << "  [" << i + 1 << "] " << arguments_[i].to_string() << std::endl;
        }
    }
    else
    {
        std::cout << "Arguments: None" << std::endl;
    }


    if (!method_name_mapped_.empty()) {
        std::cout << "Mapped Field Name: " << method_name_mapped_ << std::endl;
    }
    else {
        std::cout << "Mapped Field Name: <none>" << std::endl;
    }

    if (!method_signature_mapped_.empty()) {
        std::cout << "Mapped Field Signature: " << method_signature_mapped_ << std::endl;
    }
    else {
        std::cout << "Mapped Field Signature: <none>" << std::endl;
    }

    std::cout << "--------------------------" << std::endl;
}

java_method_t::~java_method_t()
{
    JNIEnv* env = java_interop_manager->get_env();

    if (!declaring_class_ || !env) {
        return;
    }

    env->DeleteGlobalRef(declaring_class_);
}
