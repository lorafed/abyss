#include "pch.hpp"
#include "sdk.hpp"

void windows::allocate_console()
{
    AllocConsole();
    freopen_s(reinterpret_cast<FILE**>(stdout), str_crypt("CONOUT$"), "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stdout), str_crypt("CONIN$"), "r", stdin);
    freopen_s(reinterpret_cast<FILE**>(stdout), str_crypt("CONOUT$"), "w", stderr);
}

void windows::deallocate_console()
{
    fflush(stdout);
    fflush(stderr);
    fflush(stdin);

    if (stdout) fclose(stdout);
    if (stderr) fclose(stderr);
    if (stdin) fclose(stdin);

    LI_FN(ShowWindow)(GetConsoleWindow(), SW_HIDE);
    LI_FN(FreeConsole)();
}

void windows::set_clipboard_text(const std::string text)
{
    if (!LI_FN(OpenClipboard)(nullptr)) {
        return;
    }

    if (!LI_FN(EmptyClipboard)())
    {
        LI_FN(CloseClipboard)();
        return;
    }

    HGLOBAL g_handle = LI_FN(GlobalAlloc)(GMEM_MOVEABLE, text.size() + 1);
    if (!g_handle)
    {
        CloseClipboard();
        return;
    }

    char* g_lock = static_cast<char*>(LI_FN(GlobalLock)(g_handle));
    if (!g_lock)
    {
        LI_FN(GlobalFree)(g_handle);
        LI_FN(CloseClipboard)();
        return;
    }

    memcpy(g_lock, text.c_str(), text.size() + 1);
    LI_FN(GlobalUnlock)(g_handle);

    if (!LI_FN(SetClipboardData)(CF_TEXT, g_handle))
    {
        LI_FN(GlobalFree)(g_handle);
        LI_FN(CloseClipboard)();
        return;
    }

    LI_FN(CloseClipboard)();
    return;
}

std::string windows::read_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file) {
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    file.close();

    return buffer.str();
}


std::string windows::get_appdata_path()
{
    char* app_data_path = nullptr;
    size_t len = 0;
    _dupenv_s(&app_data_path, &len, str_crypt("APPDATA"));
    std::string path(app_data_path ? app_data_path : "");
    free(app_data_path);
    return path;
}

struct enum_windows_param
{
    LPARAM param1;
    LPARAM param2;
};

std::string windows::get_main_window_title()
{
    std::string title;

    auto enum_windows = [](HWND hwnd, LPARAM param) -> int __stdcall
    {
        DWORD window_thread_pid = 0;
        GetWindowThreadProcessId(hwnd, &window_thread_pid);

        enum_windows_param* p_params = reinterpret_cast<enum_windows_param*>(param);

        if (window_thread_pid == p_params->param1)
        {
            char window_title[256];
            GetWindowTextA(hwnd, window_title, 256);

            if (strlen(window_title) == 0) {
                return TRUE;
            }

            char class_name[256];
            GetClassNameA(hwnd, class_name, 256);

            if (string::compare(class_name, "GLFW30") == 0) {
                return TRUE;
            }

            *reinterpret_cast<std::string*>(p_params->param2) = window_title;
            return FALSE;
        }

        return TRUE;
    };

    enum_windows_param params;
    params.param1 = GetCurrentProcessId();
    params.param2 = reinterpret_cast<LPARAM>(&title);

    EnumWindows(enum_windows, reinterpret_cast<LPARAM>(&params));

    return title;
}

bool windows::is_key_down(__int64 key)
{
    return (LI_FN(GetAsyncKeyState)(key) & 0x8000);
}

bool windows::is_key_down_toggle(__int64 key)
{
    return (LI_FN(GetAsyncKeyState)(key) & 0x0001);
}

bool windows::is_window_active(void* hwnd)
{
    return GetForegroundWindow() == hwnd;
}

HWND windows::get_main_window()
{
    HWND handle = nullptr;

    auto enum_windows = [](HWND hwnd, LPARAM param) -> int __stdcall
    {
        DWORD window_thread_pid = 0;
        LI_FN(GetWindowThreadProcessId)(hwnd, &window_thread_pid);

        enum_windows_param* p_params = reinterpret_cast<enum_windows_param*>(param);

        if (window_thread_pid == p_params->param1)
        {
            char window_title[256];
            LI_FN(GetWindowTextA)(hwnd, window_title, 256);

            if (strlen(window_title) == 0) {
                return TRUE;
            }

            char class_name[256];
            LI_FN(GetClassNameA)(hwnd, class_name, 256);

            if (string::compare(class_name, str_crypt("LWJGL").decrypt()) == 0) {
                return TRUE;
            }

            *reinterpret_cast<HWND*>(p_params->param2) = hwnd;
            return FALSE;
        }

        return TRUE;
    };

    enum_windows_param params;
    params.param1 = LI_FN(GetCurrentProcessId)();
    params.param2 = reinterpret_cast<LPARAM>(&handle);

    EnumWindows(enum_windows, reinterpret_cast<LPARAM>(&params));

    return handle;
}
