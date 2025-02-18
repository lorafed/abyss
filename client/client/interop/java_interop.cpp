#include "java_interop.hpp"

void java_interop::initialize()
{
    jsize jvm_count = 0;
    if (JNI_GetCreatedJavaVMs(&jvm_, 1, &jvm_count) != JNI_OK || jvm_count == 0) {
        EXCEPTION("no jvm found");
    }

    attach_thread();
    attach_jvmti();

    JNIEnv* env = get_env();

    jint count = 0;
    jthread* threads = nullptr;

    jvmtiError error = jvmti_->GetAllThreads(&count, &threads);
    if (error != JVMTI_ERROR_NONE)
    {
        THROW_MESSAGE_EXCEPTION("failed to get all threads");
        return;
    }

    if (count == 0)
    {
        THROW_MESSAGE_EXCEPTION("no java threads found");
        return;
    }

    jvmtiThreadInfo thread_info;
    for (jint i = 0; i < count; i++)
    {
        if (jvmti_->GetThreadInfo(threads[i], &thread_info) != JVMTI_ERROR_NONE) {
            continue;
        }

        if (thread_info.context_class_loader == nullptr) {
            continue;
        }

        thread_class_loader_ = thread_info.context_class_loader;
        thread_class_loader_ = env->NewGlobalRef(thread_class_loader_);

        break;
    }

    if (!thread_class_loader_)
    {
        THROW_MESSAGE_EXCEPTION("failed to find thread class loader");
        return;
    }

    find_class_method_id_ = env->GetMethodID(env->GetObjectClass(thread_class_loader_), str_crypt("findClass"), str_crypt("(Ljava/lang/String;)Ljava/lang/Class;"));
    load_class_method_id_ = env->GetMethodID(env->GetObjectClass(thread_class_loader_), str_crypt("loadClass"), str_crypt("(Ljava/lang/String;)Ljava/lang/Class;"));
}

jclass java_interop::find_class(const std::string& name)
{
    if (auto it = class_cache_.find(name); it != class_cache_.end()) {
        return it->second;
    }

    JNIEnv* env = get_env();

    jstring name_string = env->NewStringUTF(name.data());

    jclass clazz = static_cast<jclass>(env->NewGlobalRef(env->CallObjectMethod(thread_class_loader_, find_class_method_id_, name_string)));
    class_cache_[name] = clazz;

    return clazz;
}

jclass java_interop::load_class(const std::string& name)
{
    if (auto it = class_cache_.find(name); it != class_cache_.end()) {
        return it->second;
    }

    JNIEnv* env = get_env();

    jstring name_string = env->NewStringUTF(name.data());

    jclass clazz = static_cast<jclass>(env->NewGlobalRef(env->CallObjectMethod(thread_class_loader_, load_class_method_id_, name_string)));
    class_cache_[name] = clazz;

    return clazz;
}

jclass java_interop::find_class_s(const std::string& name)
{
    if (ctx.is_badlion()) {
        return load_class(name);
    }

    return find_class(name);
}

jclass java_interop::fetch_class(const std::string& name)
{
    if (auto it = class_cache_.find(name); it != class_cache_.end()) {
        return it->second;
    }

    return jclass();
}

std::string java_interop::get_class_name(jclass clazz, bool trim)
{
    JNIEnv* env = get_env();

    jclass class_clazz = find_class_s(std_crypt("java/lang/Class"));

    if (class_clazz == nullptr) {
        return {};
    }

    jmethodID get_name_method = env->GetMethodID(class_clazz, str_crypt("getName"), str_crypt("()Ljava/lang/String;"));
    if (get_name_method == nullptr)
    {
        env->DeleteLocalRef(class_clazz);
        return {};
    }

    jstring class_name = reinterpret_cast<jstring>(env->CallObjectMethod(clazz, get_name_method));
    if (class_name == nullptr)
    {
        env->DeleteLocalRef(class_clazz);
        return {};
    }

    const char* class_name_ptr = env->GetStringUTFChars(class_name, nullptr);
    std::string return_name = std::string(class_name_ptr);

    env->ReleaseStringUTFChars(class_name, class_name_ptr);
    env->DeleteLocalRef(class_name);
    env->DeleteLocalRef(class_clazz);

    if (trim)
    {
        if (size_t last_dot = return_name.find_last_of('.'); last_dot != std::string::npos) {
            return return_name.substr(last_dot + 1);
        }

        return {};
    }

    return return_name;
}

std::string java_interop::get_class_name_jvmti(jclass clazz, bool trim)
{
    char* signature = nullptr;
    char* generic_signature = nullptr;

    jvmtiError err = jvmti_->GetClassSignature(clazz, &signature, &generic_signature);
    if (err != JVMTI_ERROR_NONE || signature == nullptr) {
        return {};
    }

    std::string class_name(signature + 1);
    class_name.pop_back(); 
    std::replace(class_name.begin(), class_name.end(), '/', '.');

    jvmti_->Deallocate((unsigned char*)signature);
    if (generic_signature != nullptr) {
        jvmti_->Deallocate((unsigned char*)generic_signature);
    }

    if (trim)
    {
        if (size_t last_dot = class_name.find_last_of('.'); last_dot != std::string::npos) {
            return class_name.substr(last_dot + 1);
        }

        return {};
    }

    return class_name;
}

void java_interop::dump_classes()
{
    jclass* classes = nullptr;
    jint class_count = 0l;

    std::cout << "dumping" << std::endl;

    JNIEnv* env = get_env();

    while (fetch_class("net.minecraft.client.Minecraft") == nullptr)
    {
        if (GetAsyncKeyState(VK_DELETE) != 0) {
            break;
        }

        class_cache_.clear();
        jvmti_->GetLoadedClasses(&class_count, &classes);

        for (long i = 0l; i < class_count; i++)
        {
            std::string class_name = get_class_name_jvmti(classes[i]);
            if (strstr(class_name.c_str(), "sun.") || strstr(class_name.c_str(), "jdk.")) {
                continue;
            }

            //std::cout << class_name << std::endl;
            class_cache_[class_name] = classes[i];
        }

        Sleep(1);
    }

    for (auto& [name, clazz] : class_cache_) {
        clazz = (jclass)env->NewGlobalRef(clazz);
    }

    std::cout << "dumping done" << std::endl;
}

void java_interop::attach_thread(bool daemon)
{
    unsigned long long current_thread_id = GetCurrentThreadId();

    if (auto it = thread_cache_.find(current_thread_id); it != thread_cache_.end()) {
        return;
    }

    thread_info thread{};

    if (jvm_->GetEnv(reinterpret_cast<void**>(&thread.env), JNI_VERSION_1_8) != JNI_EDETACHED)
    {
        EXCEPTION("Failed to get jni environment");
        return;
    }

    if (daemon)
    {
        thread.is_daemon = true;
        if (jvm_->AttachCurrentThreadAsDaemon(reinterpret_cast<void**>(&thread.env), nullptr) != JNI_OK)
        {
            EXCEPTION("Failed to attach current thread as daemon");
            return;
        }
    }
    else
    {
        if (jvm_->AttachCurrentThread(reinterpret_cast<void**>(&thread.env), nullptr) != JNI_OK)
        {
            EXCEPTION("Failed to attach current thread as daemon");
            return;
        }
    }

    thread.is_attached = true;
    thread_cache_[current_thread_id] = thread;
}

void java_interop::register_thread(JNIEnv* env)
{
    unsigned long long current_thread_id = GetCurrentThreadId();

    if (auto it = thread_cache_.find(current_thread_id); it != thread_cache_.end()) {
        return;
    }

    thread_info thread{};
    thread.env = env;
    thread.is_attached = false;
    thread.is_daemon = false;

    thread_cache_[current_thread_id] = thread;
}

void java_interop::detach_thread()
{
    unsigned long long current_thread_id = GetCurrentThreadId();

    auto it = thread_cache_.find(current_thread_id);
    if (it == thread_cache_.end()) {
        return;
    }

    thread_info thread = it->second;

    if (!thread.is_attached) {
        return;
    }

    jvm_->DetachCurrentThread();
    thread_cache_.erase(current_thread_id);
}

void java_interop::unregister_thread()
{
    unsigned long long current_thread_id = GetCurrentThreadId();

    if (auto it = thread_cache_.find(current_thread_id); it == thread_cache_.end()) {
        return;
    }

    thread_cache_.erase(current_thread_id);
}

void java_interop::attach_jvmti()
{
    if (jvm_->GetEnv(reinterpret_cast<void**>(&jvmti_), JVMTI_VERSION_1_1) != JNI_OK) {
        EXCEPTION("Failed to get jvmti environment");
    }
}

void java_interop::destroy()
{
    std::cout << "deallocating classes" << std::endl;
    for (auto&& [name, clazz] : class_cache_) {
        if (clazz != nullptr) {
           // std::cout << " -> " << name << std::endl;
            get_env()->DeleteGlobalRef(clazz);
        }
    }

    detach_thread();

    jvmti_->DisposeEnvironment();

    for (auto&& thread : thread_cache_) {
        if (thread.second.is_attached) {
            std::cout << "dangling thread: " << thread.first << std::endl;
        }
    }

    class_cache_.clear();
    thread_cache_.clear();
}

JNIEnv* java_interop::get_env() const
{
    unsigned long long current_thread_id = GetCurrentThreadId();

    auto it = thread_cache_.find(current_thread_id);
    if (it == thread_cache_.end()) {
        return nullptr;
    }

    return it->second.env;
}

jvmtiEnv* java_interop::get_jvmti()
{
    if (jvmti_ != nullptr) {
        return jvmti_;
    }

    if (jvm_->GetEnv(reinterpret_cast<void**>(&jvmti_), JVMTI_VERSION_1_1) != JNI_OK) {
        EXCEPTION("Failed to get jvmti environment");
    }

    return jvmti_;
}

java_interop::~java_interop()
{
   /* std::cout << "interop destroy" << std::endl;
    for (auto&& [name, clazz] : class_cache_) { 
        if (clazz != nullptr) {
            get_env()->DeleteGlobalRef(clazz);
            std::cout << "Class: " << name << std::endl;
        }
    }

    class_cache_.clear();
    thread_cache_.clear();

    detach_thread();

    for (auto&& thread : thread_cache_) {
        if (thread.second.is_attached) {
            std::cout << "Thread: " << thread.first << std::endl;
        }
    }*/
    //thread_env_manager->detach_thread();
}

std::string java_type_to_string(java_type_t type)
{
    static const std::unordered_map<java_type_t, std::string> type_string_map =
    {
        { java_type_t::JAVA_BOOLEAN, "boolean" },
        { java_type_t::JAVA_BYTE, "byte" },
        { java_type_t::JAVA_CHAR, "char" },
        { java_type_t::JAVA_SHORT, "short" },
        { java_type_t::JAVA_INT, "int" },
        { java_type_t::JAVA_LONG, "long" },
        { java_type_t::JAVA_FLOAT, "float" },
        { java_type_t::JAVA_DOUBLE, "double" },
        { java_type_t::JAVA_OBJECT, "object" },
        { java_type_t::JAVA_ARRAY, "array" },
        { java_type_t::JAVA_VOID, "void" },
        { java_type_t::JAVA_UNKNOWN, "unknown" }
    };

    auto it = type_string_map.find(type);
    if (it == type_string_map.end()) {
        return "unknown";
    }

    return it->second;
}

java_type_t string_to_java_type(const std::string& string)
{
    static const std::unordered_map<std::string, java_type_t> string_type_map =
    {
        { "boolean", java_type_t::JAVA_BOOLEAN },
        { "byte", java_type_t::JAVA_BYTE },
        { "char", java_type_t::JAVA_CHAR },
        { "short", java_type_t::JAVA_SHORT },
        { "int", java_type_t::JAVA_INT },
        { "long", java_type_t::JAVA_LONG },
        { "float", java_type_t::JAVA_FLOAT },
        { "double", java_type_t::JAVA_DOUBLE },
        { "object", java_type_t::JAVA_OBJECT },
        { "array", java_type_t::JAVA_ARRAY },
        { "void", java_type_t::JAVA_VOID, },
        { "unknown", java_type_t::JAVA_UNKNOWN, }
    };

    auto it = string_type_map.find(string);
    if (it == string_type_map.end()) {
        return java_type_t::JAVA_UNKNOWN;
    }

    return it->second;
}

java_type_t parse_type(const std::string& signature, size_t& index, std::string& optional)
{
    switch (signature[index])
    {
    case 'Z': return java_type_t::JAVA_BOOLEAN;
    case 'B': return java_type_t::JAVA_BYTE;
    case 'C': return java_type_t::JAVA_CHAR;
    case 'S': return java_type_t::JAVA_SHORT;
    case 'I': return java_type_t::JAVA_INT;
    case 'J': return java_type_t::JAVA_LONG;
    case 'F': return java_type_t::JAVA_FLOAT;
    case 'D': return java_type_t::JAVA_DOUBLE;
    case 'V': return java_type_t::JAVA_VOID;
    case 'L':
    {
        // fully qualified class name ends with ';'

        size_t semicolon_pos = signature.find(';', index);
        if (semicolon_pos == std::string::npos) {
            throw std::invalid_argument("Invalid object type in signature");
        }

        optional = signature.substr(index + 1, semicolon_pos - index - 1); // class name
        index = semicolon_pos;

        return java_type_t::JAVA_OBJECT;
    }
    case '[':
    {
        index++; // Skip '['

        java_type_t element_type = parse_type(signature, index, optional);
        optional = element_type == java_type_t::JAVA_OBJECT ? optional : java_type_to_string(element_type);

        return java_type_t::JAVA_ARRAY;
    }
    default: return java_type_t::JAVA_UNKNOWN;
    }
}

java_field_info parse_field_signature(const std::string& signature)
{
    if (signature.empty()) {
        throw std::invalid_argument("Invalid field signature");
    }

    size_t index = 0;
    std::string optional;

    java_argument arg_data;
    arg_data.type = parse_type(signature, index, optional);
    arg_data.optional = optional;

    return java_field_info
    {
        arg_data.type,
        arg_data.optional
    };
}

java_method_info parse_method_signature(const std::string& signature)
{
    if (signature.empty() || signature[0] != '(') {
        throw std::invalid_argument("Invalid method signature: missing opening '('");
    }

    size_t index = 1;
    std::vector<java_argument> argument_types;
    std::string optional;

    while (index < signature.size() && signature[index] != ')')
    {
        java_argument arg_data;
        arg_data.type = parse_type(signature, index, optional);
        arg_data.optional = optional;
        argument_types.push_back(arg_data);
        index++;

        optional.clear();
    }

    if (index >= signature.size() || signature[index] != ')') {
        throw std::invalid_argument("Invalid method signature: missing closing ')'");
    }

    index++; // Skip ')'

    if (index >= signature.size()) {
        throw std::invalid_argument("Invalid method signature: missing return type");
    }

    optional.clear(); // Reset optional for return type
    java_type_t return_type = parse_type(signature, index, optional);

    return java_method_info
    {
        return_type,
        static_cast<int>(argument_types.size()),
        argument_types,
        optional
    };
}

std::string java_type_descriptor::to_string() const
{
    std::string type_str = java_type_to_string(type);

    if (class_name.has_value()) {
        type_str += " (" + class_name.value() + ")";
    }

    return type_str;
}
