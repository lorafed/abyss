#include "../java_interop.hpp"

java_string_t::java_string_t(jobject obj) : java_object_t(obj) {
    if (object_ != nullptr && !is_instance_of(get_string_class())) {
        cleanup_object();
        object_ = nullptr;
        class_ = nullptr;
        THROW_MESSAGE_EXCEPTION("Object is not a String");
    }
}

java_string_t::java_string_t(const std::string& str) {
    JNIEnv* env = java_interop_manager->get_env();
    jstring jstr = env->NewStringUTF(str.c_str());
    if (!jstr) {
        THROW_MESSAGE_EXCEPTION("Failed to create jstring from std::string");
    }

    *this = java_string_t(jstr);
    env->DeleteLocalRef(jstr);
}

std::string java_string_t::to_std_string() const {
    check_instance_safety();
    check_thread_safety();

    JNIEnv* env = java_interop_manager->get_env();
    const char* chars = env->GetStringUTFChars(static_cast<jstring>(object_), nullptr);
    if (!chars) {
        THROW_MESSAGE_EXCEPTION("Failed to get string characters");
    }

    std::string result(chars);
    env->ReleaseStringUTFChars(static_cast<jstring>(object_), chars);
    return result;
}

void java_string_t::set_value(const std::string& str) {
    check_thread_safety();

    JNIEnv* env = java_interop_manager->get_env();
    jstring new_jstr = env->NewStringUTF(str.c_str());
    if (!new_jstr) {
        THROW_MESSAGE_EXCEPTION("Failed to create new jstring");
    }

    java_string_t new_str(new_jstr);
    env->DeleteLocalRef(new_jstr);

    if (is_global_) {
        new_str.make_global();
    }

    *this = std::move(new_str);
}

jsize java_string_t::get_length() const {
    check_instance_safety();
    check_thread_safety();

    JNIEnv* env = java_interop_manager->get_env();
    return env->GetStringLength(static_cast<jstring>(object_));
}

jsize java_string_t::get_utf8_length() const {
    check_instance_safety();
    check_thread_safety();

    JNIEnv* env = java_interop_manager->get_env();
    return env->GetStringUTFLength(static_cast<jstring>(object_));
}

bool java_string_t::empty() const {
    return get_length() == 0;
}

java_string_t java_string_t::from_std_string(const std::string& str) {
    return java_string_t(str);
}

bool java_string_t::operator==(const java_string_t& other) const {
    return is_same_object_t(other);
}

jclass java_string_t::get_string_class() {
    static jclass stringClass = []() {
        JNIEnv* env = java_interop_manager->get_env();
        jclass localRef = env->FindClass("java/lang/String");
        if (!localRef) {
            env->ExceptionClear();
            THROW_MESSAGE_EXCEPTION("Failed to find String class");
        }
        jclass globalRef = static_cast<jclass>(env->NewGlobalRef(localRef));
        env->DeleteLocalRef(localRef);
        return globalRef;
        }();
    return stringClass;
}

bool java_string_t::operator==(const std::string& other) const {
    return to_std_string() == other;
}

java_string_t::operator std::string() const {
    return to_std_string();
}