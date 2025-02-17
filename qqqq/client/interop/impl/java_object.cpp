#include "../java_interop.hpp"

java_object_t::java_object_t(jobject object)
: thread_id_(GetCurrentThreadId()), object_(nullptr), class_(nullptr), is_global_(false), is_class_(true)
{
    if (!object) return;

    JNIEnv* env = java_interop_manager->get_env();
    jobjectRefType ref_type = env->GetObjectRefType(object);

    if (ref_type == JNIGlobalRefType) 
    {
        object_ = env->NewGlobalRef(object);
        is_global_ = true;
    }
    else if (ref_type == JNILocalRefType) 
    {
        object_ = env->NewLocalRef(object);
        is_global_ = false;
    }
    else {
        THROW_MESSAGE_EXCEPTION("Unsupported reference type");
    }

    jclass tmp = env->GetObjectClass(object_);
    class_ = is_global_
    ? static_cast<jclass>(env->NewGlobalRef(tmp))
    : static_cast<jclass>(env->NewLocalRef(tmp));
    env->DeleteLocalRef(tmp);
}

java_object_t::java_object_t(jclass clazz)
: thread_id_(GetCurrentThreadId()), object_(nullptr), class_(nullptr), is_global_(false), is_class_(true)
{
    if (!clazz) return;

    JNIEnv* env = java_interop_manager->get_env();
    jobjectRefType ref_type = env->GetObjectRefType(clazz);

    if (ref_type == JNIGlobalRefType)
    {
        object_ = env->NewGlobalRef(clazz);
        is_global_ = true;
    }
    else if (ref_type == JNILocalRefType)
    {
        object_ = env->NewLocalRef(clazz);
        is_global_ = false;
    }
    else {
        THROW_MESSAGE_EXCEPTION("Unsupported reference type");
    }

    class_ = is_global_
    ? static_cast<jclass>(env->NewGlobalRef(clazz))
    : static_cast<jclass>(env->NewLocalRef(clazz));

    is_class_ = true;
}

java_object_t::java_object_t(const java_object_t& other)
    : object_(nullptr), is_class_(other.is_class_), is_global_(other.is_global_), thread_id_(other.thread_id_)
{
    if (other.object_ == nullptr || other.class_ == nullptr) {
        return;
    }

    check_thread_safety();

    JNIEnv* env = java_interop_manager->get_env();

    if (is_global_) 
    {
        object_ = env->NewGlobalRef(other.object_);
        class_ = static_cast<jclass>(env->NewGlobalRef(other.class_));
    }
    else 
    {
        object_ = env->NewLocalRef(other.object_);
        class_ = static_cast<jclass>(env->NewLocalRef(other.class_));
    }
}

java_object_t::java_object_t(java_object_t&& other) noexcept
    : object_(other.object_), class_(other.class_),
    is_class_(other.is_class_), is_global_(other.is_global_), thread_id_(other.thread_id_)
{
    other.object_ = nullptr;
    other.class_ = nullptr;
}

bool java_object_t::is_same_object_t(const java_object_t& other) const
{
    check_instance_safety();
    check_thread_safety();

    JNIEnv* env = java_interop_manager->get_env();

    return env->IsSameObject(object_, other.object_);
}

bool java_object_t::is_same_object(const jobject& other) const
{
    check_instance_safety();
    check_thread_safety();

    JNIEnv* env = java_interop_manager->get_env();

    return env->IsSameObject(object_, other);
}

bool java_object_t::is_instance_of_t(const java_class_t& clazz) const
{
    check_instance_safety();
    check_thread_safety();

    JNIEnv* env = java_interop_manager->get_env();

    return env->IsInstanceOf(object_, clazz.clazz());
}

bool java_object_t::is_instance_of(const jclass& clazz) const
{
    check_instance_safety();
    check_thread_safety();

    JNIEnv* env = java_interop_manager->get_env();

    return env->IsInstanceOf(object_, clazz);
}

bool java_object_t::is_class() const
{
    return is_class_;
}

jobject java_object_t::object() const
{
    return object_;
}

bool java_object_t::operator==(std::nullptr_t) const {
    return object_ == nullptr;
}

bool java_object_t::operator!=(std::nullptr_t) const {
    return !(*this == nullptr);
}

bool java_object_t::operator!() const
{
    return object_ == nullptr;
}

java_object_t::operator bool() const
{
    return object_ != nullptr;
}

java_object_t::operator jobject() const
{
    return object_;
}

java_object_t::operator jclass() const
{
    return class_;
}

java_object_t& java_object_t::operator=(const java_object_t& other)
{
    if (this != &other) 
    {
        if (object_ != nullptr)
        {
            check_thread_safety();
            cleanup_object();
        }

        JNIEnv* env = java_interop_manager->get_env();

        is_class_ = other.is_class_;
        is_global_ = other.is_global_;
        thread_id_ = other.thread_id_;

        if (other.object_ != nullptr)
        {
            if (is_global_)
            {
                object_ = env->NewGlobalRef(other.object_);
                class_ = static_cast<jclass>(env->NewGlobalRef(other.class_));
            }
            else
            {
                object_ = env->NewLocalRef(other.object_);
                class_ = static_cast<jclass>(env->NewLocalRef(other.class_));
            }
        }
    }

    return *this;
}

java_object_t& java_object_t::operator=(java_object_t&& other) noexcept
{
    if (this != &other) 
    {
        if (object_ != nullptr)
        {
            check_thread_safety();
            cleanup_object();
        }

        object_ = other.object_;
        class_ = other.class_;
        is_class_ = other.is_class_;
        is_global_ = other.is_global_;
        thread_id_ = other.thread_id_;

        other.object_ = nullptr;
        other.class_ = nullptr;
    }
    return *this;
}

void java_object_t::make_global()
{
    check_thread_safety();

    if (is_global_) {
        return;
    }

    JNIEnv* env = java_interop_manager->get_env();
    object_ = env->NewGlobalRef(object_);
    class_ = static_cast<jclass>(env->NewGlobalRef(class_));
    is_global_ = true;
}

java_object_t::~java_object_t()
{
    if (object_ == nullptr) {
        return;
    }

    if (is_global_)
    {
        JNIEnv* env = java_interop_manager->get_env();
        env->DeleteGlobalRef(object_);
        env->DeleteGlobalRef(class_);
    }
}

void java_object_t::check_thread_safety() const
{
    if (is_global_ || GetCurrentThreadId() == thread_id_) {
        return;
    }

    THROW_MESSAGE_EXCEPTION("Local references can only be accessed from the thread that created them.");
}

void java_object_t::check_instance_safety() const
{
    if (object_ != nullptr && class_ != nullptr) {
        return;
    }

    THROW_MESSAGE_EXCEPTION("Invalid object reference.");
}

void java_object_t::cleanup_object()
{
    if (object_ == nullptr) {
        return;
    }

    JNIEnv* env = java_interop_manager->get_env();

    if (is_global_) 
    {
        env->DeleteGlobalRef(object_);
        env->DeleteGlobalRef(class_);
    }
    else
    {
        env->DeleteLocalRef(object_);
        env->DeleteLocalRef(class_);
    }

    object_ = nullptr;
}
