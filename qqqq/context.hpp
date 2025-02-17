#pragma once

#include <atomic>


enum VERSION
{
    LUNAR,
    BADLION,
    UNKNOWN,
};

enum STATE
{
    NORMAL,
    EXCEPTION,
};

class context
{
public:
    void* main_window = nullptr;

    VERSION curr_version = VERSION::UNKNOWN;
    std::atomic<bool> unload{ false };
    std::atomic<int> interface_bind{ VK_INSERT };

    bool is_badlion() {
        return curr_version == BADLION;
    }

    bool is_obf() {
        return curr_version == BADLION;
    }

    bool is_srg() {
        return false;
    }

    bool is_mcp() {
        return curr_version == LUNAR;
    }
};

extern context ctx;