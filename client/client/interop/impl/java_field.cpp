#include "../java_interop.hpp"

java_field_t::java_field_t(jclass declaring_class, jfieldID field_id) : declaring_class_(declaring_class), field_id_(field_id)
{
    if (declaring_class == nullptr || field_id == nullptr) {
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

    char* field_name = nullptr;
    char* field_signature = nullptr;
    char* field_signature_generic = nullptr;

    jvmtiError err = jvmti->GetFieldName(declaring_class, field_id_, &field_name, &field_signature, &field_signature_generic);
    if (err != JVMTI_ERROR_NONE)
    {
        THROW_MESSAGE_EXCEPTION("Failed to retrieve field name or signature");
        return;
    }

    jint field_modifiers;
    err = jvmti->GetFieldModifiers(declaring_class, field_id_, &field_modifiers);
    if (err != JVMTI_ERROR_NONE)
    {
        THROW_MESSAGE_EXCEPTION("Failed to retrieve field modifiers");
        return;
    }

    is_static_ = field_modifiers & JVM_ACC_STATIC;

    field_name_ = field_name;
    field_signature_ = field_signature;


    //std::cout << "new field allocated: " << field_name_ << std::endl;

    field_declaring_class_name_ = java_interop_manager->get_class_name_jvmti(declaring_class, false);

    declaring_class_ = static_cast<jclass>(env->NewGlobalRef(declaring_class));

    jvmti->Deallocate(reinterpret_cast<unsigned char*>(field_name));
    jvmti->Deallocate(reinterpret_cast<unsigned char*>(field_signature));

    if (field_signature_generic != nullptr)
    {
        jvmti->Deallocate(reinterpret_cast<unsigned char*>(field_signature_generic));
    }

    java_field_info parsed_data = parse_field_signature(field_signature_);

    switch (parsed_data.type)
    {
    case java_type_t::JAVA_OBJECT:
        field_type_ = java_type_descriptor{ parsed_data.type, parsed_data.optional };
        break;
    case java_type_t::JAVA_ARRAY:
        field_type_ = java_type_descriptor{ parsed_data.type, parsed_data.optional };
        break;
    default:
        field_type_ = java_type_descriptor{ parsed_data.type };
        break;
    }
}

java_field_t::java_field_t(const java_field_t& other)
    : field_id_(other.field_id_),
    declaring_class_(other.declaring_class_),
    field_name_(other.field_name_),
    field_signature_(other.field_signature_),
    field_declaring_class_name_(other.field_declaring_class_name_),
    field_name_mapped_(other.field_name_mapped_),
    field_signature_mapped_(other.field_signature_mapped_),
    field_type_(other.field_type_)
{
    JNIEnv* env = java_interop_manager->get_env();
    if (env) {
        declaring_class_ = static_cast<jclass>(env->NewGlobalRef(declaring_class_));
    }
}

java_field_t::java_field_t(java_field_t&& other) noexcept
    : field_id_(other.field_id_),
    declaring_class_(other.declaring_class_),
    field_name_(std::move(other.field_name_)),
    field_signature_(std::move(other.field_signature_)),
    field_declaring_class_name_(std::move(other.field_declaring_class_name_)),
    field_name_mapped_(std::move(other.field_name_mapped_)),
    field_signature_mapped_(std::move(other.field_signature_mapped_)),
    field_type_(std::move(other.field_type_))
{
    other.declaring_class_ = nullptr;
    other.field_id_ = nullptr;
}

bool java_field_t::operator==(std::nullptr_t) const {
    return field_id_ == nullptr || declaring_class_ == nullptr;
}

bool java_field_t::operator!=(std::nullptr_t) const {
    return !(*this == nullptr);
}

bool java_field_t::operator!() const
{
    return field_id_ == nullptr || declaring_class_ == nullptr;
}

java_field_t::operator bool() const
{
    return field_id_ != nullptr && declaring_class_ != nullptr;
}

java_field_t& java_field_t::operator=(const java_field_t& other)
{
    if (this == &other) {
        return *this;
    }

    // Clean up existing resources
    JNIEnv* env = java_interop_manager->get_env();
    if (env && declaring_class_) {
        env->DeleteGlobalRef(declaring_class_);
    }

    field_id_ = other.field_id_;
    declaring_class_ = other.declaring_class_;
    field_name_ = other.field_name_;
    field_signature_ = other.field_signature_;
    field_declaring_class_name_ = other.field_declaring_class_name_;
    field_name_mapped_ = other.field_name_mapped_;
    field_signature_mapped_ = other.field_signature_mapped_;
    field_type_ = other.field_type_;

    if (env) {
        declaring_class_ = static_cast<jclass>(env->NewGlobalRef(declaring_class_));
    }

    return *this;
}

java_field_t& java_field_t::operator=(java_field_t&& other) noexcept
{
    if (this != &other) {
        return *this;
    }

    JNIEnv* env = java_interop_manager->get_env();
    if (env && declaring_class_) {
        env->DeleteGlobalRef(declaring_class_);
    }

    // Transfer ownership
    field_id_ = other.field_id_;
    declaring_class_ = other.declaring_class_;
    field_name_ = std::move(other.field_name_);
    field_signature_ = std::move(other.field_signature_);
    field_declaring_class_name_ = std::move(other.field_declaring_class_name_);
    field_name_mapped_ = std::move(other.field_name_mapped_);
    field_signature_mapped_ = std::move(other.field_signature_mapped_);
    field_type_ = std::move(other.field_type_);

    other.declaring_class_ = nullptr;
    other.field_id_ = nullptr;

    return *this;
}

bool java_field_t::is_static() const
{
    return is_static_;
}

std::string java_field_t::get_name(bool mapped) const
{
    if (mapped) return field_name_mapped_;
    return field_name_;
}

std::string java_field_t::get_signature(bool mapped) const
{
    if (mapped) return field_signature_mapped_;
    return field_signature_;
}

void java_field_t::set_mapped_name(const std::string& name)
{
    if (!name.empty()) {
        field_name_mapped_ = name;
    }
    else {
        field_name_mapped_ = field_name_;
    }
}

void java_field_t::set_mapped_signature(const std::string& signature)
{
    if (!signature.empty()) {
        field_signature_mapped_ = signature;
    }
    else {
        field_signature_mapped_ = field_signature_;
    }
}

void java_field_t::set_mappings(const std::string& name, const std::string& signature)
{
    if (!name.empty()) {
        field_name_mapped_ = name;
    }
    else {
        field_name_mapped_ = field_name_;
    }

    if (!signature.empty()) {
        field_signature_mapped_ = signature;
    }
    else {
        field_signature_mapped_ = field_signature_;
    }
}

void java_field_t::debug_print() const
{
    std::cout << "Field Debug Information:" << std::endl;
    std::cout << "--------------------------" << std::endl;
    std::cout << "Declaring Class: " << field_declaring_class_name_ << std::endl;
    std::cout << "Field Name: " << field_name_ << std::endl;
    std::cout << "Field Signature: " << field_signature_ << std::endl;
    std::cout << "Field Type: " << field_type_.to_string() << std::endl;

    if (!field_name_mapped_.empty()) {
        std::cout << "Mapped Field Name: " << field_name_mapped_ << std::endl;
    }
    else {
        std::cout << "Mapped Field Name: <none>" << std::endl;
    }

    if (!field_signature_mapped_.empty()) {
        std::cout << "Mapped Field Signature: " << field_signature_mapped_ << std::endl;
    }
    else {
        std::cout << "Mapped Field Signature: <none>" << std::endl;
    }

    std::cout << "--------------------------" << std::endl;
}

java_field_t::~java_field_t()
{
    JNIEnv* env = java_interop_manager->get_env();

    if (!declaring_class_ || !env) {
        return;
    }

    env->DeleteGlobalRef(declaring_class_);
}
