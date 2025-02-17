#pragma once

#include <jni.h>
#include <jvmti.h>

#include <map>
#include <cstdint>
#include <memory>
#include <string>
#include <filesystem>
#include <vector>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <stdexcept>

#include "sdk.hpp"
#include "context.hpp"
#include "exceptions.hpp"

#define JVM_ACC_STATIC 0x0008

class java_class_t;
class java_object_t;
class java_method_t;
class java_field_t;

enum java_type_t : uint8_t
{
    JAVA_BOOLEAN,
    JAVA_BYTE,
    JAVA_CHAR,
    JAVA_SHORT,
    JAVA_INT,
    JAVA_LONG,
    JAVA_FLOAT,
    JAVA_DOUBLE,
    JAVA_OBJECT,
    JAVA_ARRAY,
    JAVA_VOID,
    JAVA_UNKNOWN,
};

struct java_mapping 
{
    std::string name;
    std::string signature;

    java_mapping() = default;

    java_mapping(const std::string& name, const std::string& signature)
        : name(name), signature(signature) {}

    bool operator==(const java_mapping& other) const {
        return name == other.name && signature == other.signature;
    }
};

struct query_mapping
{
    std::string name;
    std::optional<std::string> signature;
};

// Custom hash function for java_mapping
struct java_mapping_hash {
    size_t operator()(const java_mapping& mapping) const {
        size_t name_hash = std::hash<std::string>{}(mapping.name);
        size_t signature_hash = std::hash<std::string>{}(mapping.signature);
        return name_hash ^ (signature_hash << 1); // Combine the hashes
    }
};

struct java_type_descriptor
{
    java_type_t type;
    std::optional<std::string> class_name; // Optional, for objects or arrays

    std::string to_string() const;
};

//Dummy type for parse_type method
struct java_argument
{
    java_type_t type;
    std::string optional; // For OBJECT or ARRAY types, holds class or array type info
};

struct java_method_info
{
    java_type_t return_type;
    int num_arguments;
    std::vector<java_argument> argument_types;

    std::string optional; // For OBJECT or ARRAY return type
};

struct java_field_info
{
    java_type_t type;
    std::string optional;
};

std::string java_type_to_string(java_type_t type);
java_type_t string_to_java_type(const std::string& string);
java_type_t parse_type(const std::string& signature, size_t& index, std::string& optional);
java_field_info parse_field_signature(const std::string& signature);
java_method_info parse_method_signature(const std::string& signature);

class java_interop
{
public:
    java_interop() = default;

    void initialize();

    jclass find_class(const std::string& name);
    jclass load_class(const std::string& name);
    jclass find_class_s(const std::string& name);
    jclass fetch_class(const std::string& name);

    //deprecated
    std::string get_class_name(jclass clazz, bool trim = false);
    std::string get_class_name_jvmti(jclass clazz, bool trim = false);

    void dump_classes();
    void attach_thread(bool daemon = false);
    void register_thread(JNIEnv* env);
    void detach_thread();
    void unregister_thread();
    void attach_jvmti();
    void destroy();

    JNIEnv* get_env() const;
    jvmtiEnv* get_jvmti();

    ~java_interop();
private:
    struct thread_info
    {
        JNIEnv* env;
        bool is_attached;
        std::optional<bool> is_daemon;
    };

    JavaVM* jvm_ = nullptr;
    jvmtiEnv* jvmti_ = nullptr;

    jobject thread_class_loader_ = nullptr;
    jmethodID find_class_method_id_ = nullptr;
    jmethodID load_class_method_id_ = nullptr;

    std::unordered_map<std::string, jclass> class_cache_;
    std::unordered_map<unsigned long long, thread_info> thread_cache_;
};

inline extern std::unique_ptr<::java_interop> java_interop_manager = std::make_unique<::java_interop>();

class java_local_frame_t {
public:
    java_local_frame_t(int capacity = 16)
    {
        JNIEnv* env = java_interop_manager->get_env();

        if (!env) {
            throw std::invalid_argument("JNIEnv cannot be null");
        }
        if (env->PushLocalFrame(capacity) < 0) {
            throw std::runtime_error("Failed to push JNI local frame");
        }
        env_ = env;
    }

    ~java_local_frame_t()
    {
        if (env_) {
            env_->PopLocalFrame(nullptr);
          //  std::cout << "frame popped" << std::endl;
        }
    }

    java_local_frame_t(const java_local_frame_t&) = delete;
    java_local_frame_t& operator=(const java_local_frame_t&) = delete;

    java_local_frame_t(java_local_frame_t&& other) noexcept : env_(other.env_) {
        other.env_ = nullptr;
    }

    java_local_frame_t& operator=(java_local_frame_t&& other) noexcept {
        if (this != &other) {
            env_ = other.env_;
            other.env_ = nullptr;
        }
        return *this;
    }

    JNIEnv* get_env() const {
        return env_;
    }

private:
    JNIEnv* env_;
};

class java_object_t
{
public: 
    java_object_t() : thread_id_(0), object_(nullptr), is_global_(false), class_(nullptr) {}
    java_object_t(jobject object);
    java_object_t(jclass clazz);

    java_object_t(const java_object_t& other);
    java_object_t(java_object_t&& other) noexcept;

    bool is_same_object_t(const java_object_t& other) const;
    bool is_same_object(const jobject& other) const;
    bool is_instance_of_t(const java_class_t& clazz) const;
    bool is_instance_of(const jclass& clazz) const;
    bool is_class() const;

    jobject object() const;

    bool operator==(std::nullptr_t) const;
    bool operator!=(std::nullptr_t) const;
    bool operator!() const;

    explicit operator bool() const;

    operator jobject() const;
    operator jclass() const;

    java_object_t& operator=(const java_object_t& other);
    java_object_t& operator=(java_object_t&& other) noexcept;

    void make_global();

    ~java_object_t();
protected:
    jobject object_; 
    jclass class_;

    bool is_class_;
    bool is_global_;
    unsigned long long thread_id_;

    void check_thread_safety() const;
    void check_instance_safety() const;
    void cleanup_object();
};

class java_method_t
{
public:
    java_method_t() : method_id_(nullptr), declaring_class_(nullptr), num_arguments_(0) {};
    java_method_t(jmethodID method_id);
    java_method_t(const java_method_t& other);
    java_method_t(java_method_t&& other) noexcept;

    bool operator==(std::nullptr_t) const;
    bool operator!=(std::nullptr_t) const;
    bool operator!() const;

    explicit operator bool() const;

    operator jmethodID() const;

    java_method_t& operator=(const java_method_t& other);
    java_method_t& operator=(java_method_t&& other) noexcept;

    std::string get_name(bool mapped = false) const;
    std::string get_signature(bool mapped = false) const;

    std::vector<java_type_descriptor> arguments() const;
    java_type_descriptor return_type() const;

    int num_arguments() const;
    bool is_static() const;

    //falls back to the real non mapped method name if the provided mapping is empty
    void set_mapped_name(const std::string& name);
    //falls back to the real non mapped method signature if the provided mapping is empty
    void set_mapped_signature(const std::string& signature);
    //falls back to the real non mapped method signature and name if the provided mapping is empty
    void set_mappings(const std::string& name, const std::string& signature);

    template<typename type, typename ...args>
    type call(java_object_t obj, args ...arguments)
    {
        if (sizeof... (arguments) != num_arguments_) {
            EXCEPTION_NOCRYPT("Passed argument size does not match the expected argument size of " + std::to_string(num_arguments_) + " at " + method_name_);
        }

        if (obj == nullptr && !is_static_) {
            EXCEPTION("No instance object passed to non-static method");
        }

        JNIEnv* env = java_interop_manager->get_env();

        if constexpr (std::is_same_v<type, jobject>) 
        {
            if (return_type_.type == java_type_t::JAVA_OBJECT || return_type_.type == java_type_t::JAVA_ARRAY) 
            {
                return is_static_ ? env->CallStaticObjectMethod(obj, method_id_, arguments...) :
                    env->CallObjectMethod(obj, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jboolean> || std::is_same_v<type, bool>)
        {
            if (return_type_.type == java_type_t::JAVA_BOOLEAN) 
            {
                return is_static_ ? env->CallStaticBooleanMethod(obj, method_id_, arguments...) :
                    env->CallBooleanMethod(obj, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jbyte> || std::is_same_v<type, byte>)
        {
            if (return_type_.type == java_type_t::JAVA_BYTE) 
            {
                return is_static_ ? env->CallStaticByteMethod(obj, method_id_, arguments...) :
                    env->CallByteMethod(obj, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jchar> || std::is_same_v<type, char>)
        {
            if (return_type_.type == java_type_t::JAVA_CHAR)
            {
                return is_static_ ? env->CallStaticCharMethod(obj, method_id_, arguments...) :
                    env->CallCharMethod(obj, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jdouble> || std::is_same_v<type, double>)
        {
            if (return_type_.type == java_type_t::JAVA_DOUBLE) 
            {
                return is_static_ ? env->CallStaticDoubleMethod(obj, method_id_, arguments...) :
                    env->CallDoubleMethod(obj, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jfloat> || std::is_same_v<type, float>)
        {
            if (return_type_.type == java_type_t::JAVA_FLOAT) 
            {
                return is_static_ ? env->CallStaticFloatMethod(obj, method_id_, arguments...) :
                    env->CallFloatMethod(obj, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jint>)
        {
            if (return_type_.type == java_type_t::JAVA_INT || std::is_same_v<type, int>)
            {
                return is_static_ ? env->CallStaticIntMethod(obj, method_id_, arguments...) :
                    env->CallIntMethod(obj, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jlong> || std::is_same_v<type, long>) {
            if (return_type_.type == java_type_t::JAVA_LONG) {
                return is_static_ ? env->CallStaticLongMethod(obj, method_id_, arguments...) :
                    env->CallLongMethod(obj, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jshort> || std::is_same_v<type, short>)
        {
            if (return_type_.type == java_type_t::JAVA_SHORT)
            {
                return is_static_ ? env->CallStaticShortMethod(obj, method_id_, arguments...) :
                    env->CallShortMethod(obj, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, void>)
        {
            if (return_type_.type == java_type_t::JAVA_VOID)
            {
                if (is_static_) {
                    env->CallStaticVoidMethod(obj, method_id_, arguments...);
                }
                else {
                    env->CallVoidMethod(obj, method_id_, arguments...);
                }

                return;
            }
        }
        else {
            EXCEPTION("Unknown or unsupported return type");
        }
    }

    template<typename type, typename ...args>
    type call(args ...arguments)
    {
        if (sizeof... (arguments) != num_arguments_) {
            EXCEPTION_NOCRYPT("Passed argument size does not match the expected argument size of " + std::to_string(num_arguments_) + " at " + method_name_);
        }

        if (!is_static_) {
            EXCEPTION("No instance object passed to non-static method");
        }

        JNIEnv* env = java_interop_manager->get_env();

        if constexpr (std::is_same_v<type, jobject>)
        {
            if (return_type_.type == java_type_t::JAVA_OBJECT || return_type_.type == java_type_t::JAVA_ARRAY)
            {
                return env->CallStaticObjectMethod(declaring_class_, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jboolean> || std::is_same_v<type, bool>)
        {
            if (return_type_.type == java_type_t::JAVA_BOOLEAN)
            {
                return env->CallStaticBooleanMethod(declaring_class_, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jbyte> || std::is_same_v<type, byte>)
        {
            if (return_type_.type == java_type_t::JAVA_BYTE)
            {
                return env->CallStaticByteMethod(declaring_class_, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jchar> || std::is_same_v<type, char>)
        {
            if (return_type_.type == java_type_t::JAVA_CHAR)
            {
                return env->CallStaticCharMethod(declaring_class_, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jdouble> || std::is_same_v<type, double>)
        {
            if (return_type_.type == java_type_t::JAVA_DOUBLE)
            {
                return env->CallStaticDoubleMethod(declaring_class_, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jfloat> || std::is_same_v<type, float>)
        {
            if (return_type_.type == java_type_t::JAVA_FLOAT)
            {
                return env->CallStaticFloatMethod(declaring_class_, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jint>)
        {
            if (return_type_.type == java_type_t::JAVA_INT || std::is_same_v<type, int>)
            {
                return env->CallStaticIntMethod(declaring_class_, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jlong> || std::is_same_v<type, long>) {
            if (return_type_.type == java_type_t::JAVA_LONG) {
                return env->CallStaticLongMethod(declaring_class_, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, jshort> || std::is_same_v<type, short>)
        {
            if (return_type_.type == java_type_t::JAVA_SHORT)
            {
                return env->CallStaticShortMethod(declaring_class_, method_id_, arguments...);
            }
        }
        else if constexpr (std::is_same_v<type, void>)
        {
            if (return_type_.type == java_type_t::JAVA_VOID)
            {
                env->CallStaticVoidMethod(declaring_class_, method_id_, arguments...);
                return;
            }
        }
        else {
            EXCEPTION("Unknown or unsupported return type");
        }
    }

    void debug_print() const;

    ~java_method_t();
private:
    jmethodID method_id_;
    jclass declaring_class_;

    std::string method_name_;
    std::string method_signature_;
    std::string method_declaring_class_name_;

    std::string method_name_mapped_;
    std::string method_signature_mapped_;

    int num_arguments_;
    bool is_static_;

    java_type_descriptor return_type_;
    std::vector<java_type_descriptor> arguments_;
};

class java_field_t
{
public: 
    java_field_t() : declaring_class_(nullptr), field_id_(nullptr) {}
    java_field_t(jclass declaring_class, jfieldID field_id);
    java_field_t(const java_field_t& other);
    java_field_t(java_field_t&& other) noexcept;

    bool operator==(std::nullptr_t) const;
    bool operator!=(std::nullptr_t) const;
    bool operator!() const;

    explicit operator bool() const;


    java_field_t& operator=(const java_field_t& other);
    java_field_t& operator=(java_field_t&& other) noexcept;

    bool is_static() const;

    std::string get_name(bool mapped = false) const;
    std::string get_signature(bool mapped = false) const;

    //falls back to the real non mapped field name if the provided mapping is empty
    void set_mapped_name(const std::string& name);
    //falls back to the real non mapped field signature if the provided mapping is empty
    void set_mapped_signature(const std::string& signature);
    //falls back to the real non mapped field signature and name if the provided mapping is empty
    void set_mappings(const std::string& name, const std::string& signature);

    template<typename T>
    T get() const
    {
        if (field_id_ == nullptr || declaring_class_ == nullptr) {
            return T();
        }

        /*if (!is_static_) {
            EXCEPTION("No instance object passed to non-static field");
        }*/

        JNIEnv* env = java_interop_manager->get_env();

        if constexpr (std::is_same_v<T, jobject>) {
            if (field_type_.type == java_type_t::JAVA_OBJECT || field_type_.type == java_type_t::JAVA_ARRAY) {
                return env->GetStaticObjectField(declaring_class_, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jboolean> || std::is_same_v<T, bool>) {
            if (field_type_.type == java_type_t::JAVA_BOOLEAN) {
                return env->GetStaticBooleanField(declaring_class_, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jbyte> || std::is_same_v<T, char>) {
            if (field_type_.type == java_type_t::JAVA_BYTE) {
                return env->GetStaticByteField(declaring_class_, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jchar>) {
            if (field_type_.type == java_type_t::JAVA_CHAR) {
                return env->GetStaticCharField(declaring_class_, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jdouble> || std::is_same_v<T, double>) {
            if (field_type_.type == java_type_t::JAVA_DOUBLE) {
                return env->GetStaticDoubleField(declaring_class_, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jfloat> || std::is_same_v<T, float>) {
            if (field_type_.type == java_type_t::JAVA_FLOAT) {
                return env->GetStaticFloatField(declaring_class_, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jint> || std::is_same_v<T, int>) {
            if (field_type_.type == java_type_t::JAVA_INT) {
                return env->GetStaticIntField(declaring_class_, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jlong> || std::is_same_v<T, long>) {
            if (field_type_.type == java_type_t::JAVA_LONG) {
                return env->GetStaticLongField(declaring_class_, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jshort> || std::is_same_v<T, short>) {
            if (field_type_.type == java_type_t::JAVA_SHORT) {
                return env->GetStaticShortField(declaring_class_, field_id_);
            }
        }
        else {
            EXCEPTION("Unsupported field type for get operation");
        }

        return T{};
    }

    template<typename T>
    void set(T value) const
    {
        if (field_id_ == nullptr || declaring_class_ == nullptr) {
            return;
        }

        if (!is_static_) {
            EXCEPTION("No instance object passed to non-static field");
        }

        JNIEnv* env = java_interop_manager->get_env();

        if constexpr (std::is_same_v<T, jobject>) {
            if (field_type_.type == java_type_t::JAVA_OBJECT || field_type_.type == java_type_t::JAVA_ARRAY) {
                env->SetStaticObjectField(declaring_class_, field_id_, value);
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jboolean> || std::is_same_v<T, bool>) {
            if (field_type_.type == java_type_t::JAVA_BOOLEAN) {
                env->SetStaticBooleanField(declaring_class_, field_id_, static_cast<jboolean>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jbyte> || std::is_same_v<T, char>) {
            if (field_type_.type == java_type_t::JAVA_BYTE) {
                env->SetStaticByteField(declaring_class_, field_id_, static_cast<jbyte>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jchar>) {
            if (field_type_.type == java_type_t::JAVA_CHAR) {
                env->SetStaticCharField(declaring_class_, field_id_, value);
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jdouble> || std::is_same_v<T, double>) {
            if (field_type_.type == java_type_t::JAVA_DOUBLE) {
                env->SetStaticDoubleField(declaring_class_, field_id_, static_cast<jdouble>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jfloat> || std::is_same_v<T, float>) {
            if (field_type_.type == java_type_t::JAVA_FLOAT) {
                env->SetStaticFloatField(declaring_class_, field_id_, static_cast<jfloat>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jint> || std::is_same_v<T, int>) {
            if (field_type_.type == java_type_t::JAVA_INT) {
                env->SetStaticIntField(declaring_class_, field_id_, static_cast<jint>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jlong> || std::is_same_v<T, long>) {
            if (field_type_.type == java_type_t::JAVA_LONG) {
                env->SetStaticLongField(declaring_class_, field_id_, static_cast<jlong>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jshort> || std::is_same_v<T, short>) {
            if (field_type_.type == java_type_t::JAVA_SHORT) {
                env->SetStaticShortField(declaring_class_, field_id_, static_cast<jshort>(value));
                return;
            }
        }
    }

    template<typename T>
    T get(java_object_t obj) const
    {
        if (field_id_ == nullptr || declaring_class_ == nullptr) {
            return T();
        }

        if (obj == nullptr && !is_static_) {
            EXCEPTION("No instance object passed to non-static field");
        }

        JNIEnv* env = java_interop_manager->get_env();

        if constexpr (std::is_same_v<T, jobject>) {
            if (field_type_.type == java_type_t::JAVA_OBJECT || field_type_.type == java_type_t::JAVA_ARRAY) {
                return is_static_ ? env->GetStaticObjectField(declaring_class_, field_id_)
                    : env->GetObjectField(obj, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jboolean> || std::is_same_v<T, bool>) {
            if (field_type_.type == java_type_t::JAVA_BOOLEAN) {
                return is_static_ ? env->GetStaticBooleanField(declaring_class_, field_id_)
                    : env->GetBooleanField(obj, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jbyte> || std::is_same_v<T, char>) {
            if (field_type_.type == java_type_t::JAVA_BYTE) {
                return is_static_ ? env->GetStaticByteField(declaring_class_, field_id_)
                    : env->GetByteField(obj, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jchar>) {
            if (field_type_.type == java_type_t::JAVA_CHAR) {
                return is_static_ ? env->GetStaticCharField(declaring_class_, field_id_)
                    : env->GetCharField(obj, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jdouble> || std::is_same_v<T, double>) {
            if (field_type_.type == java_type_t::JAVA_DOUBLE) {
                return is_static_ ? env->GetStaticDoubleField(declaring_class_, field_id_)
                    : env->GetDoubleField(obj, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jfloat> || std::is_same_v<T, float>) {
            if (field_type_.type == java_type_t::JAVA_FLOAT) {
                return is_static_ ? env->GetStaticFloatField(declaring_class_, field_id_)
                    : env->GetFloatField(obj, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jint> || std::is_same_v<T, int>) {
            if (field_type_.type == java_type_t::JAVA_INT) {
                return is_static_ ? env->GetStaticIntField(declaring_class_, field_id_)
                    : env->GetIntField(obj, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jlong> || std::is_same_v<T, long>) {
            if (field_type_.type == java_type_t::JAVA_LONG) {
                return is_static_ ? env->GetStaticLongField(declaring_class_, field_id_)
                    : env->GetLongField(obj, field_id_);
            }
        }
        else if constexpr (std::is_same_v<T, jshort> || std::is_same_v<T, short>) {
            if (field_type_.type == java_type_t::JAVA_SHORT) {
                return is_static_ ? env->GetStaticShortField(declaring_class_, field_id_)
                    : env->GetShortField(obj, field_id_);
            }
        }
        else {
            EXCEPTION("Unsupported field type for get operation");
        }

        return T{}; 
    }

    template<typename T>
    void set(java_object_t obj, T value) const
    {
        if (field_id_ == nullptr || declaring_class_ == nullptr) {
            return;
        }

        if (obj == nullptr && !is_static_) {
            EXCEPTION("No instance object passed to non-static field");
        }

        JNIEnv* env = java_interop_manager->get_env();

        if constexpr (std::is_same_v<T, jobject>) {
            if (field_type_.type == java_type_t::JAVA_OBJECT || field_type_.type == java_type_t::JAVA_ARRAY) {
                is_static_ ? env->SetStaticObjectField(declaring_class_, field_id_, value)
                    : env->SetObjectField(obj, field_id_, value);
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jboolean> || std::is_same_v<T, bool>) {
            if (field_type_.type == java_type_t::JAVA_BOOLEAN) {
                is_static_ ? env->SetStaticBooleanField(declaring_class_, field_id_, static_cast<jboolean>(value))
                    : env->SetBooleanField(obj, field_id_, static_cast<jboolean>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jbyte> || std::is_same_v<T, char>) {
            if (field_type_.type == java_type_t::JAVA_BYTE) {
                is_static_ ? env->SetStaticByteField(declaring_class_, field_id_, static_cast<jbyte>(value))
                    : env->SetByteField(obj, field_id_, static_cast<jbyte>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jchar>) {
            if (field_type_.type == java_type_t::JAVA_CHAR) {
                is_static_ ? env->SetStaticCharField(declaring_class_, field_id_, value)
                    : env->SetCharField(obj, field_id_, value);
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jdouble> || std::is_same_v<T, double>) {
            if (field_type_.type == java_type_t::JAVA_DOUBLE) {
                is_static_ ? env->SetStaticDoubleField(declaring_class_, field_id_, static_cast<jdouble>(value))
                    : env->SetDoubleField(obj, field_id_, static_cast<jdouble>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jfloat> || std::is_same_v<T, float>) {
            if (field_type_.type == java_type_t::JAVA_FLOAT) {
                is_static_ ? env->SetStaticFloatField(declaring_class_, field_id_, static_cast<jfloat>(value))
                    : env->SetFloatField(obj, field_id_, static_cast<jfloat>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jint> || std::is_same_v<T, int>) {
            if (field_type_.type == java_type_t::JAVA_INT) {
                is_static_ ? env->SetStaticIntField(declaring_class_, field_id_, static_cast<jint>(value))
                    : env->SetIntField(obj, field_id_, static_cast<jint>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jlong> || std::is_same_v<T, long>) {
            if (field_type_.type == java_type_t::JAVA_LONG) {
                is_static_ ? env->SetStaticLongField(declaring_class_, field_id_, static_cast<jlong>(value))
                    : env->SetLongField(obj, field_id_, static_cast<jlong>(value));
                return;
            }
        }
        else if constexpr (std::is_same_v<T, jshort> || std::is_same_v<T, short>) {
            if (field_type_.type == java_type_t::JAVA_SHORT) {
                is_static_ ? env->SetStaticShortField(declaring_class_, field_id_, static_cast<jshort>(value))
                    : env->SetShortField(obj, field_id_, static_cast<jshort>(value));
                return;
            }
        }

        EXCEPTION("Unsupported field type for set operation");
    }

    void debug_print() const;

    ~java_field_t();
private:
    jfieldID field_id_;
    jclass declaring_class_;

    std::string field_name_;
    std::string field_signature_;
    std::string field_declaring_class_name_;

    std::string field_name_mapped_;
    std::string field_signature_mapped_;

    bool is_static_;

    java_type_descriptor field_type_;
};

//all class references are global and SHOULD only be constructed once
class java_class_t : public java_object_t
{
public:
    java_class_t() = delete;
    java_class_t(jclass clazz);
    java_class_t(const java_class_t& other);
    java_class_t(java_class_t&& other) noexcept;

    java_class_t& operator=(const java_class_t& other);
    java_class_t& operator=(java_class_t&& other) noexcept;

    void debug_print() const;

    std::string get_class_name(bool trimmed = false) const;

    java_method_t get_method(const std::string& name, const std::string& signature = {}) const;
    java_field_t get_field(const std::string& name, const std::string& signature = {}) const;

    jclass clazz() const;

    template<typename type, typename ...args>
    type call_method(const query_mapping& mapping, args ...arguments)
    {
        check_thread_safety();
        check_instance_safety();

        java_method_t method = nullptr;

        if (mapping.signature.has_value()) {
            method = get_method(mapping.name, mapping.signature.value());
        }
        else {
            method = get_method(mapping.name);
        }

        if (!method.is_static()) {
            EXCEPTION("No instance reference passed to non-static method");
        }

        if (method == nullptr) {
            return type();
        }

        JNIEnv* env = java_interop_manager->get_env();

        jobject class_ref = env->NewLocalRef(class_);

        return method.call<type>(class_ref, arguments...);
    }

    template<typename type, typename ...args>
    type call_method(java_object_t obj, const query_mapping& mapping, args ...arguments)
    {
        check_thread_safety();
        check_instance_safety();

        java_method_t method = nullptr;

        if (mapping.signature.has_value()) {
            method = get_method(mapping.name, mapping.signature.value());
        }
        else {
            method = get_method(mapping.name);
        }

        if (method.is_static()) {
            EXCEPTION("Instance reference passed to static method");
        }

        if (method == nullptr || obj == nullptr) {
            return type();
        }

        return method.call<type>(obj, arguments...);
    }
    
    template<typename type>
    type get_field(java_object_t obj, const query_mapping& mapping)
    {
        check_thread_safety();
        check_instance_safety();

        java_field_t field = nullptr;

        if (mapping.signature.has_value()) {
            field = get_field(mapping.name, mapping.signature.value());
        }
        else {
            field = get_field(mapping.name);
        }

        if (field.is_static()) {
            EXCEPTION("Instance reference passed to static method");
        }

        if (field == nullptr || obj == nullptr) {
            return type();
        }

        return field.get<type>(obj);
    }

    template<typename type>
    void set_field(java_object_t obj, const query_mapping& mapping, type val)
    {
        check_thread_safety();
        check_instance_safety();

        java_field_t field = nullptr;

        if (mapping.signature.has_value()) {
            field = get_field(mapping.name, mapping.signature.value());
        }
        else {
            field = get_field(mapping.name);
        }

        if (field.is_static()) {
            EXCEPTION("Instance reference passed to static method");
        }

        if (field == nullptr || obj == nullptr) {
            return;
        }

        return field.set(obj, val);
    }

    ~java_class_t();
private:
    std::string class_name_;
    std::string class_name_full_;

    int class_state_;

    std::unordered_map<java_mapping, java_method_t, java_mapping_hash> methods_;
    std::unordered_map<std::string, java_method_t> methods_by_name_;
    std::unordered_map<java_mapping, java_field_t, java_mapping_hash> fields_;
    std::unordered_map<std::string, java_field_t> fields_by_name_;
};

template<typename T> struct java_array_traits;

#define DEFINE_JAVA_ARRAY_TRAITS(jtype, jname, jarraytype, jgetter, jsetter) \
template<> struct java_array_traits<jtype> {                                 \
    using element_type = jtype;                                              \
    using jarray_type = jarraytype;                                          \
                                                                             \
    static jclass get_array_class(JNIEnv* env) {                             \
        return env->FindClass("[" #jname);                                   \
    }                                                                        \
                                                                             \
    static jarray_type new_array(JNIEnv* env, jsize length) {                \
        return env->New##jname##Array(length);                               \
    }                                                                        \
                                                                             \
    static element_type* get_elements(JNIEnv* env, jarray_type array,        \
                                      jboolean* is_copy) {                   \
        return env->Get##jname##ArrayElements(array, is_copy);               \
    }                                                                        \
                                                                             \
    static void release_elements(JNIEnv* env, jarray_type array,             \
                                element_type* elems, jint mode) {            \
        env->Release##jname##ArrayElements(array, elems, mode);              \
    }                                                                        \
                                                                             \
    static element_type get_element(JNIEnv* env, jarray_type array,          \
                                   jsize index) {                            \
        element_type result;                                                 \
        env->Get##jname##ArrayRegion(array, index, 1, &result);              \
        return result;                                                       \
    }                                                                        \
                                                                             \
    static void set_element(JNIEnv* env, jarray_type array, jsize index,     \
                           element_type value) {                             \
        env->Set##jname##ArrayRegion(array, index, 1, &value);               \
    }                                                                        \
                                                                             \
    static void set_element_region(JNIEnv* env, jarray_type array,           \
                           std::vector<element_type> vec) {                  \
        env->Set##jname##ArrayRegion(array, 0, vec.size(), vec.data());      \
    }                                                                        \
};

template<> struct java_array_traits<jint> {
    using element_type = jint; using jarray_type = jintArray; static jclass get_array_class(JNIEnv* env) {
        return env->FindClass("[I");
    } static jarray_type new_array(JNIEnv* env, jsize length) {
        return env->NewIntArray(length);
    } static element_type* get_elements(JNIEnv* env, jarray_type array, jboolean* is_copy) {
        return env->GetIntArrayElements(array, is_copy);
    } static void release_elements(JNIEnv* env, jarray_type array, element_type* elems, jint mode) {
        env->ReleaseIntArrayElements(array, elems, mode);
    } static element_type get_element(JNIEnv* env, jarray_type array, jsize index) {
        element_type result; env->GetIntArrayRegion(array, index, 1, &result); return result;
    } static void set_element(JNIEnv* env, jarray_type array, jsize index, element_type value) {
        env->SetIntArrayRegion(array, index, 1, &value);
    } static void set_element_region(JNIEnv* env, jarray_type array, std::vector<element_type> vec) {
        env->SetIntArrayRegion(array, 0, vec.size(), vec.data());
    }
};

template<> struct java_array_traits<jlong> {
    using element_type = jlong; using jarray_type = jlongArray; static jclass get_array_class(JNIEnv* env) {
        return env->FindClass("[L");
    } static jarray_type new_array(JNIEnv* env, jsize length) {
        return env->NewLongArray(length);
    } static element_type* get_elements(JNIEnv* env, jarray_type array, jboolean* is_copy) {
        return env->GetLongArrayElements(array, is_copy);
    } static void release_elements(JNIEnv* env, jarray_type array, element_type* elems, jint mode) {
        env->ReleaseLongArrayElements(array, elems, mode);
    } static element_type get_element(JNIEnv* env, jarray_type array, jsize index) {
        element_type result; env->GetLongArrayRegion(array, index, 1, &result); return result;
    } static void set_element(JNIEnv* env, jarray_type array, jsize index, element_type value) {
        env->SetLongArrayRegion(array, index, 1, &value);
    } static void set_element_region(JNIEnv* env, jarray_type array, std::vector<element_type> vec) {
        env->SetLongArrayRegion(array, 0, vec.size(), vec.data());
    }
};

template<> struct java_array_traits<jfloat> {
    using element_type = jfloat; using jarray_type = jfloatArray; static jclass get_array_class(JNIEnv* env) {
        return env->FindClass("[F");
    } static jarray_type new_array(JNIEnv* env, jsize length) {
        return env->NewFloatArray(length);
    } static element_type* get_elements(JNIEnv* env, jarray_type array, jboolean* is_copy) {
        return env->GetFloatArrayElements(array, is_copy);
    } static void release_elements(JNIEnv* env, jarray_type array, element_type* elems, jint mode) {
        env->ReleaseFloatArrayElements(array, elems, mode);
    } static element_type get_element(JNIEnv* env, jarray_type array, jsize index) {
        element_type result; env->GetFloatArrayRegion(array, index, 1, &result); return result;
    } static void set_element(JNIEnv* env, jarray_type array, jsize index, element_type value) {
        env->SetFloatArrayRegion(array, index, 1, &value);
    } static void set_element_region(JNIEnv* env, jarray_type array, std::vector<element_type> vec) {
        env->SetFloatArrayRegion(array, 0, vec.size(), vec.data());
    }
};

template<> struct java_array_traits<jdouble> {
    using element_type = jdouble; using jarray_type = jdoubleArray; static jclass get_array_class(JNIEnv* env) {
        return env->FindClass("[D");
    } static jarray_type new_array(JNIEnv* env, jsize length) {
        return env->NewDoubleArray(length);
    } static element_type* get_elements(JNIEnv* env, jarray_type array, jboolean* is_copy) {
        return env->GetDoubleArrayElements(array, is_copy);
    } static void release_elements(JNIEnv* env, jarray_type array, element_type* elems, jint mode) {
        env->ReleaseDoubleArrayElements(array, elems, mode);
    } static element_type get_element(JNIEnv* env, jarray_type array, jsize index) {
        element_type result; env->GetDoubleArrayRegion(array, index, 1, &result); return result;
    } static void set_element(JNIEnv* env, jarray_type array, jsize index, element_type value) {
        env->SetDoubleArrayRegion(array, index, 1, &value);
    } static void set_element_region(JNIEnv* env, jarray_type array, std::vector<element_type> vec) {
        env->SetDoubleArrayRegion(array, 0, vec.size(), vec.data());
    }
};

template<> struct java_array_traits<jboolean> {
    using element_type = jboolean; using jarray_type = jbooleanArray; static jclass get_array_class(JNIEnv* env) {
        return env->FindClass("[Z");
    } static jarray_type new_array(JNIEnv* env, jsize length) {
        return env->NewBooleanArray(length);
    } static element_type* get_elements(JNIEnv* env, jarray_type array, jboolean* is_copy) {
        return env->GetBooleanArrayElements(array, is_copy);
    } static void release_elements(JNIEnv* env, jarray_type array, element_type* elems, jint mode) {
        env->ReleaseBooleanArrayElements(array, elems, mode);
    } static element_type get_element(JNIEnv* env, jarray_type array, jsize index) {
        element_type result; env->GetBooleanArrayRegion(array, index, 1, &result); return result;
    } static void set_element(JNIEnv* env, jarray_type array, jsize index, element_type value) {
        env->SetBooleanArrayRegion(array, index, 1, &value);
    } static void set_element_region(JNIEnv* env, jarray_type array, std::vector<element_type> vec) {
        env->SetBooleanArrayRegion(array, 0, vec.size(), vec.data());
    }
};

template<> struct java_array_traits<jbyte> {
    using element_type = jbyte; using jarray_type = jbyteArray; static jclass get_array_class(JNIEnv* env) {
        return env->FindClass("[B");
    } static jarray_type new_array(JNIEnv* env, jsize length) {
        return env->NewByteArray(length);
    } static element_type* get_elements(JNIEnv* env, jarray_type array, jboolean* is_copy) {
        return env->GetByteArrayElements(array, is_copy);
    } static void release_elements(JNIEnv* env, jarray_type array, element_type* elems, jint mode) {
        env->ReleaseByteArrayElements(array, elems, mode);
    } static element_type get_element(JNIEnv* env, jarray_type array, jsize index) {
        element_type result; env->GetByteArrayRegion(array, index, 1, &result); return result;
    } static void set_element(JNIEnv* env, jarray_type array, jsize index, element_type value) {
        env->SetByteArrayRegion(array, index, 1, &value);
    } static void set_element_region(JNIEnv* env, jarray_type array, std::vector<element_type> vec) {
        env->SetByteArrayRegion(array, 0, vec.size(), vec.data());
    }
};

template<> struct java_array_traits<jshort> {
    using element_type = jshort; using jarray_type = jshortArray; static jclass get_array_class(JNIEnv* env) {
        return env->FindClass("[S");
    } static jarray_type new_array(JNIEnv* env, jsize length) {
        return env->NewShortArray(length);
    } static element_type* get_elements(JNIEnv* env, jarray_type array, jboolean* is_copy) {
        return env->GetShortArrayElements(array, is_copy);
    } static void release_elements(JNIEnv* env, jarray_type array, element_type* elems, jint mode) {
        env->ReleaseShortArrayElements(array, elems, mode);
    } static element_type get_element(JNIEnv* env, jarray_type array, jsize index) {
        element_type result; env->GetShortArrayRegion(array, index, 1, &result); return result;
    } static void set_element(JNIEnv* env, jarray_type array, jsize index, element_type value) {
        env->SetShortArrayRegion(array, index, 1, &value);
    } static void set_element_region(JNIEnv* env, jarray_type array, std::vector<element_type> vec) {
        env->SetShortArrayRegion(array, 0, vec.size(), vec.data());
    }
};

template<> struct java_array_traits<jchar> {
    using element_type = jchar; using jarray_type = jcharArray; static jclass get_array_class(JNIEnv* env) {
        return env->FindClass("[C");
    } static jarray_type new_array(JNIEnv* env, jsize length) {
        return env->NewCharArray(length);
    } static element_type* get_elements(JNIEnv* env, jarray_type array, jboolean* is_copy) {
        return env->GetCharArrayElements(array, is_copy);
    } static void release_elements(JNIEnv* env, jarray_type array, element_type* elems, jint mode) {
        env->ReleaseCharArrayElements(array, elems, mode);
    } static element_type get_element(JNIEnv* env, jarray_type array, jsize index) {
        element_type result; env->GetCharArrayRegion(array, index, 1, &result); return result;
    } static void set_element(JNIEnv* env, jarray_type array, jsize index, element_type value) {
        env->SetCharArrayRegion(array, index, 1, &value);
    } static void set_element_region(JNIEnv* env, jarray_type array, std::vector<element_type> vec) {
        env->SetCharArrayRegion(array, 0, vec.size(), vec.data());
    }
};

template<typename T>
class java_array_t : public java_object_t {
public:
    using element_type = typename java_array_traits<T>::element_type;
    using jarray_type = typename java_array_traits<T>::jarray_type;

    java_array_t() = default;

    explicit java_array_t(jobject obj) : java_object_t(obj) {
        validate_array_type();
    }

    java_array_t(const java_object_t& other) : java_object_t(other) {
        validate_array_type();
    }

    java_array_t(java_object_t&& other) noexcept : java_object_t(std::move(other)) {
        validate_array_type();
    }

    void validate_array_type() const {
        if (object_ == nullptr) return;

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();

        /*if (!env->IsArray(object_)) {
            THROW_MESSAGE_EXCEPTION("Java object is not an array");
        }*/
        jclass obj_class = env->GetObjectClass(object_);

        jclass class_class = env->FindClass("java/lang/Class");

        jmethodID is_array_method = env->GetMethodID(
            class_class,
            "isArray",
            "()Z"
        );

        jboolean is_array = env->CallBooleanMethod(obj_class, is_array_method);

        env->DeleteLocalRef(class_class);
        env->DeleteLocalRef(obj_class);

        if (!is_array) {
            THROW_MESSAGE_EXCEPTION("Java object is not an array");
        }

        jclass array_class = java_array_traits<T>::get_array_class(env);

        if (!env->IsInstanceOf(object_, array_class))
        {
            env->DeleteLocalRef(array_class);
            THROW_MESSAGE_EXCEPTION("Java array type mismatch");
        }

        env->DeleteLocalRef(array_class);
    }

    std::vector<element_type> to_vector() const {
        if (object_ == nullptr) return {};

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();

        jarray_type array = static_cast<jarray_type>(object_);
        jsize length = env->GetArrayLength(array);
        jboolean is_copy;
        element_type* elements = java_array_traits<T>::get_elements(env, array, &is_copy);

        std::vector<element_type> vec(elements, elements + length);
        java_array_traits<T>::release_elements(env, array, elements, JNI_ABORT);

        return vec;
    }

    static java_array_t from_vector(const std::vector<element_type>& vec) {
        JNIEnv* env = java_interop_manager->get_env();
        jarray_type array = java_array_traits<T>::new_array(env, vec.size());
       
        if (vec.size() > 0) {
            java_array_traits<T>::set_element_region(env, array, vec);
        }

        return java_array_t(array);
    }

    jsize length() const {
        if (object_ == nullptr) return 0;

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();

        return env->GetArrayLength(static_cast<jarray_type>(object_));
    }

    element_type get(jsize index) const {
        if (object_ == nullptr) {
            THROW_MESSAGE_EXCEPTION("Attempting to get element from null array");
        }

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();
        check_bounds(index);

        return java_array_traits<T>::get_element(env, static_cast<jarray_type>(object_), index);
    }

    // Set element at index
    void set(jsize index, element_type value) {
        if (object_ == nullptr) {
            THROW_MESSAGE_EXCEPTION("Attempting to set element in null array");
        }

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();
        check_bounds(index);

        java_array_traits<T>::set_element(env, static_cast<jarray_type>(object_), index, value);
    }

private:
    void check_bounds(jsize index) const {
        jsize len = length();
        if (index < 0 || index >= len) {
            THROW_MESSAGE_EXCEPTION("Array index out of bounds");
        }
    }
};

template<>
class java_array_t<jobject> : public java_object_t {
public:
    using element_type = jobject;
    using jarray_type = jobjectArray;

    java_array_t() : component_class_(nullptr) {}

    explicit java_array_t(jobject obj) : java_object_t(obj), component_class_(nullptr) {
        validate_array_type();
        determine_component_class();
    }

    java_array_t(const java_object_t& other) : java_object_t(other), component_class_(nullptr) {
        validate_array_type();
        determine_component_class();
    }

    java_array_t(java_object_t&& other) noexcept
        : java_object_t(std::move(other)), component_class_(nullptr) {
        validate_array_type();
        determine_component_class();
    }

    java_array_t(jarray_type array, jclass component_class)
        : java_object_t(array), component_class_(component_class) {
        validate_component_class();
        make_component_class_global();
    }

    static java_array_t<jobject> from_vector(const std::vector<java_object_t>& vec, jclass component_class) {
        JNIEnv* env = java_interop_manager->get_env();
        jsize length = static_cast<jsize>(vec.size());
        jobjectArray array = env->NewObjectArray(length, component_class, nullptr);

        for (jsize i = 0; i < length; ++i) {
            jobject element = vec[i].object();
            env->SetObjectArrayElement(array, i, element);
        }

        return java_array_t<jobject>(array, component_class);
    }

    std::vector<java_object_t> to_vector() const {
        std::vector<java_object_t> vec;
        if (object_ == nullptr) return vec;

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();

        jarray_type array = static_cast<jarray_type>(object_);
        jsize length = env->GetArrayLength(array);
        vec.reserve(length);

        for (jsize i = 0; i < length; ++i) {
            jobject element = env->GetObjectArrayElement(array, i);
            vec.emplace_back(element);
            env->DeleteLocalRef(element);
        }

        return vec;
    }

    java_object_t get(jsize index) const {
        if (object_ == nullptr) {
            THROW_MESSAGE_EXCEPTION("Attempting to get element from null array");
        }

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();
        check_bounds(index);

        jobject element = env->GetObjectArrayElement(static_cast<jarray_type>(object_), index);
        java_object_t result(element);
        env->DeleteLocalRef(element);
        return result;
    }

    void set(jsize index, const java_object_t& value) {
        if (object_ == nullptr) {
            THROW_MESSAGE_EXCEPTION("Attempting to set element in null array");
        }

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();
        check_bounds(index);

        if (value != nullptr && component_class_ != nullptr) {
            if (!env->IsInstanceOf(value.object(), component_class_)) {
                THROW_MESSAGE_EXCEPTION("Element is not an instance of the array's component type");
            }
        }

        env->SetObjectArrayElement(
            static_cast<jarray_type>(object_),
            index,
            value.object()
        );
    }

    jclass component_class() const {
        return component_class_;
    }

    jsize length() const {
        if (object_ == nullptr) return 0;

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();

        return env->GetArrayLength(static_cast<jarray_type>(object_));
    }

    ~java_array_t() {
        if (component_class_ != nullptr) {
            JNIEnv* env = java_interop_manager->get_env();
            env->DeleteGlobalRef(component_class_);
        }
    }

private:
    jclass component_class_;

    void determine_component_class() {
        if (object_ == nullptr) return;

        JNIEnv* env = java_interop_manager->get_env();
        jclass array_class = env->GetObjectClass(object_);
        jmethodID get_component_type = env->GetMethodID(
            env->FindClass("java/lang/Class"),
            "getComponentType",
            "()Ljava/lang/Class;"
        );

        jobject component_type = env->CallObjectMethod(array_class, get_component_type);
        component_class_ = static_cast<jclass>(env->NewGlobalRef(component_type));

        env->DeleteLocalRef(array_class);
        env->DeleteLocalRef(component_type);
    }

    void make_component_class_global() {
        if (component_class_ != nullptr) {
            JNIEnv* env = java_interop_manager->get_env();
            jclass global_cls = static_cast<jclass>(env->NewGlobalRef(component_class_));
            env->DeleteLocalRef(component_class_);
            component_class_ = global_cls;
        }
    }

    void validate_component_class() {
        if (component_class_ == nullptr) {
            THROW_MESSAGE_EXCEPTION("Component class cannot be null");
        }
    }

    void validate_array_type() const {
        if (object_ == nullptr) return;

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();

        jclass obj_class = env->GetObjectClass(object_);

        jclass class_class = env->FindClass("java/lang/Class");

        jmethodID is_array_method = env->GetMethodID(
            class_class,
            "isArray",
            "()Z"
        );

        jboolean is_array = env->CallBooleanMethod(obj_class, is_array_method);

        env->DeleteLocalRef(class_class);
        env->DeleteLocalRef(obj_class);

        if (!is_array) {
            THROW_MESSAGE_EXCEPTION("Java object is not an array");
        }
    }

    void check_bounds(jsize index) const 
    {
        jsize len = length();
        if (index < 0 || index >= len) {
            THROW_MESSAGE_EXCEPTION("Array index out of bounds");
        }
    }
};

class java_string_t : public java_object_t {
public:
    java_string_t(jobject obj);
    java_string_t(const std::string& str = "");

    std::string to_std_string() const;

    void set_value(const std::string& str);

    jsize get_length() const;
    jsize get_utf8_length() const;

    bool empty() const;

    static java_string_t from_std_string(const std::string& str);

    bool operator==(const java_string_t& other) const;
    bool operator==(const std::string& other) const;

    operator std::string() const;

private:
    static jclass get_string_class();
};

template<typename T>
struct java_list_traits;

#define DEFINE_JAVA_LIST_TRAITS(jtype, jname, jboxed_class, jboxed_sig, jvalue_of_sig, junbox_method) \
template<> struct java_list_traits<jtype> { \
    using element_type = jtype; \
    static jclass get_list_class(JNIEnv* env) { \
        return env->FindClass("java/util/List"); \
    } \
    static jclass get_element_class(JNIEnv* env) { \
        jclass clazz = env->FindClass(jboxed_class); \
        return clazz; \
    } \
    static jobject box(JNIEnv* env, jtype value) { \
        jclass clazz = get_element_class(env); \
        jmethodID valueOf = env->GetStaticMethodID(clazz, "valueOf", jvalue_of_sig); \
        jobject obj = env->CallStaticObjectMethod(clazz, valueOf, value); \
        env->DeleteLocalRef(clazz); \
        return obj; \
    } \
    static jtype unbox(JNIEnv* env, jobject obj) { \
        jclass clazz = get_element_class(env); \
        jmethodID unbox = env->GetMethodID(clazz, junbox_method, "()" jboxed_sig); \
        jtype result = env->Call##jname##Method(obj, unbox); \
        env->DeleteLocalRef(clazz); \
        return result; \
    } \
    static jmethodID get_add_method(JNIEnv* env) { \
        jclass listClass = get_list_class(env); \
        jmethodID mid = env->GetMethodID(listClass, "add", "(Ljava/lang/Object;)Z"); \
        env->DeleteLocalRef(listClass); \
        return mid; \
    } \
    static jmethodID get_get_method(JNIEnv* env) { \
        jclass listClass = get_list_class(env); \
        jmethodID mid = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;"); \
        env->DeleteLocalRef(listClass); \
        return mid; \
    } \
    static jmethodID get_size_method(JNIEnv* env) { \
        jclass listClass = get_list_class(env); \
        jmethodID mid = env->GetMethodID(listClass, "size", "()I"); \
        env->DeleteLocalRef(listClass); \
        return mid; \
    } \
    static jobject new_list(JNIEnv* env) { \
        jclass arrayListClass = env->FindClass("java/util/ArrayList"); \
        jmethodID constructor = env->GetMethodID(arrayListClass, "<init>", "()V"); \
        jobject list = env->NewObject(arrayListClass, constructor); \
        env->DeleteLocalRef(arrayListClass); \
        return list; \
    } \
};

DEFINE_JAVA_LIST_TRAITS(jint, Int, "java/lang/Integer", "I", "(I)Ljava/lang/Integer;", "intValue")
DEFINE_JAVA_LIST_TRAITS(jlong, Long, "java/lang/Long", "J", "(J)Ljava/lang/Long;", "longValue")
DEFINE_JAVA_LIST_TRAITS(jfloat, Float, "java/lang/Float", "F", "(F)Ljava/lang/Float;", "floatValue")
DEFINE_JAVA_LIST_TRAITS(jdouble, Double, "java/lang/Double", "D", "(D)Ljava/lang/Double;", "doubleValue")
DEFINE_JAVA_LIST_TRAITS(jboolean, Boolean, "java/lang/Boolean", "Z", "(Z)Ljava/lang/Boolean;", "booleanValue")
DEFINE_JAVA_LIST_TRAITS(jbyte, Byte, "java/lang/Byte", "B", "(B)Ljava/lang/Byte;", "byteValue")
DEFINE_JAVA_LIST_TRAITS(jshort, Short, "java/lang/Short", "S", "(S)Ljava/lang/Short;", "shortValue")
DEFINE_JAVA_LIST_TRAITS(jchar, Char, "java/lang/Character", "C", "(C)Ljava/lang/Character;", "charValue")

template<>
struct java_list_traits<jobject> {
    using element_type = jobject;
    jclass element_class;

    java_list_traits(jclass clazz) : element_class(clazz) {}

    static jclass get_list_class(JNIEnv* env) {
        return env->FindClass("java/util/List");
    }

    jclass get_element_class(JNIEnv* env) const {
        return element_class;
    }

    static jobject box(JNIEnv* env, jobject value) {
        return value; // No boxing needed
    }

    static jobject unbox(JNIEnv* env, jobject obj) {
        return obj; // No unboxing needed
    }

    static jmethodID get_add_method(JNIEnv* env) {
        jclass listClass = get_list_class(env);
        jmethodID mid = env->GetMethodID(listClass, "add", "(Ljava/lang/Object;)Z");
        env->DeleteLocalRef(listClass);
        return mid;
    }

    static jmethodID get_get_method(JNIEnv* env) {
        jclass listClass = get_list_class(env);
        jmethodID mid = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
        env->DeleteLocalRef(listClass);
        return mid;
    }

    static jmethodID get_size_method(JNIEnv* env) {
        jclass listClass = get_list_class(env);
        jmethodID mid = env->GetMethodID(listClass, "size", "()I");
        env->DeleteLocalRef(listClass);
        return mid;
    }

    static jobject new_list(JNIEnv* env) {
        jclass arrayListClass = env->FindClass("java/util/ArrayList");
        jmethodID constructor = env->GetMethodID(arrayListClass, "<init>", "()V");
        jobject list = env->NewObject(arrayListClass, constructor);
        env->DeleteLocalRef(arrayListClass);
        return list;
    }
};

template<typename T>
class java_list_t : public java_object_t {
public:
    using element_type = typename java_list_traits<T>::element_type;

    java_list_t() : element_class_(nullptr) {}

    explicit java_list_t(jobject obj) : java_object_t(obj), element_class_(nullptr) {
        validate_list_type();
        if constexpr (std::is_same_v<T, jobject>) {
            THROW_MESSAGE_EXCEPTION("Must specify element class for jobject list");
        }
        else {
            determine_element_class();
        }
    }

    java_list_t(jobject obj, jclass element_class) : java_object_t(obj), element_class_(element_class) {
        validate_list_type();
        if constexpr (std::is_same_v<T, jobject>) {
            if (element_class_ == nullptr) {
                THROW_MESSAGE_EXCEPTION("Element class cannot be null for jobject list");
            }
            element_class_ = static_cast<jclass>(java_interop_manager->get_env()->NewGlobalRef(element_class_));
        }
    }

    java_list_t(jobject obj, const std::string& element_class) : java_object_t(obj), element_class_(java_interop_manager->find_class_s(element_class)) {
        validate_list_type();
        if constexpr (std::is_same_v<T, jobject>) {
            if (element_class_ == nullptr) {
                THROW_MESSAGE_EXCEPTION("Element class cannot be null for jobject list");
            }
            element_class_ = static_cast<jclass>(java_interop_manager->get_env()->NewGlobalRef(element_class_));
        }
    }

    static java_list_t<T> create() {
        JNIEnv* env = java_interop_manager->get_env();
        jobject list = java_list_traits<T>::new_list(env);
        return java_list_t<T>(list);
    }

    static java_list_t<T> create(jclass element_class) {
        JNIEnv* env = java_interop_manager->get_env();
        jobject list = java_list_traits<jobject>::new_list(env);
        return java_list_t<T>(list, element_class);
    }

    ~java_list_t() {
        if (element_class_ != nullptr) {
            JNIEnv* env = java_interop_manager->get_env();
            env->DeleteGlobalRef(element_class_);
            element_class_ = nullptr;
        }
    }

    void add(element_type value) {
        if (object_ == nullptr) {
            THROW_MESSAGE_EXCEPTION("Attempting to add to null list");
        }

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();

        jobject boxed = java_list_traits<T>::box(env, value);
        if constexpr (std::is_same_v<T, jobject>) {
            if (!env->IsInstanceOf(boxed, element_class_)) {
                env->DeleteLocalRef(boxed);
                THROW_MESSAGE_EXCEPTION("Element is not an instance of the list's element type");
            }
        }

        jmethodID addMethod = java_list_traits<T>::get_add_method(env);
        env->CallBooleanMethod(object_, addMethod, boxed);
        env->DeleteLocalRef(boxed);

        // Check for exceptions
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            THROW_MESSAGE_EXCEPTION("Failed to add element to list");
        }
    }

    element_type get(jsize index) {
        if (object_ == nullptr) {
            THROW_MESSAGE_EXCEPTION("Attempting to get from null list");
        }

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();
        check_bounds(index);

        jmethodID getMethod = java_list_traits<T>::get_get_method(env);
        jobject element = env->CallObjectMethod(object_, getMethod, index);

        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            THROW_MESSAGE_EXCEPTION("Failed to get element from list");
        }

        element_type result = java_list_traits<T>::unbox(env, element);
        env->DeleteLocalRef(element);

        return result;
    }

    jsize size() const {
        if (object_ == nullptr) return 0;

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();

        jmethodID sizeMethod = java_list_traits<T>::get_size_method(env);
        return env->CallIntMethod(object_, sizeMethod);
    }

    std::vector<element_type> to_vector() const {
        std::vector<element_type> vec;
        if (object_ == nullptr) return vec;

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();

        jsize len = size();
        vec.reserve(len);

        for (jsize i = 0; i < len; ++i) {
            jobject element = env->CallObjectMethod(object_, java_list_traits<T>::get_get_method(env), i);
            vec.push_back(java_list_traits<T>::unbox(env, element));
            //env->DeleteLocalRef(element);
        }

        return vec;
    }

    static java_list_t<T> from_vector(const std::vector<element_type>& vec) {
        java_list_t<T> list = create();
        for (const auto& elem : vec) {
            list.add(elem);
        }
        return list;
    }

    static java_list_t<T> from_vector(const std::vector<java_object_t>& vec, jclass element_class) {
        java_list_t<T> list = create(element_class);
        JNIEnv* env = java_interop_manager->get_env();
        for (const auto& elem : vec) {
            list.add(elem.object());
        }
        return list;
    }

private:
    jclass element_class_;

    void validate_list_type() const {
        if (object_ == nullptr) return;

        JNIEnv* env = java_interop_manager->get_env();
        check_thread_safety();
        check_instance_safety();

        jclass listClass = java_list_traits<T>::get_list_class(env);
        jboolean isList = env->IsInstanceOf(object_, listClass);
        env->DeleteLocalRef(listClass);

        if (!isList) {
            THROW_MESSAGE_EXCEPTION("Java object is not a List");
        }
    }

    void determine_element_class() {
        if constexpr (!std::is_same_v<T, jobject>) {
            JNIEnv* env = java_interop_manager->get_env();
            jclass elementClass = java_list_traits<T>::get_element_class(env);
            element_class_ = static_cast<jclass>(env->NewGlobalRef(elementClass));
            env->DeleteLocalRef(elementClass);
        }
    }

    void check_bounds(jsize index) const {
        jsize len = size();
        if (index < 0 || index >= len) {
            THROW_MESSAGE_EXCEPTION("List index out of bounds");
        }
    }
};