// c_timer.hpp
#pragma once

#include <windows.h>
#include <algorithm>

class c_timer
{
private:
    DWORD start_time = 0;
    DWORD end_delay = 0;

public:
    explicit c_timer(DWORD delay)
    {
        this->start_time = static_cast<DWORD>(GetTickCount64());
        this->end_delay = delay;
    }

    explicit c_timer(int delay)
    {
        this->start_time = static_cast<DWORD>(GetTickCount64());
        this->end_delay = static_cast<int>(delay);
    }

    bool has_elapsed() const {
        return (GetTickCount64() >= static_cast<unsigned long long>(start_time + end_delay));
    }

    void reset() {
        this->start_time = static_cast<DWORD>(GetTickCount64());
    }

    void set_next_delay(DWORD delay)
    {
        reset();
        this->end_delay = delay;
    }

    int get_elapsed() const {
        return std::clamp(static_cast<int>(GetTickCount64() - start_time), 0, static_cast<int>(end_delay));
    }

    DWORD get_next_delay() const
    {
        return this->end_delay;
    }

    bool counting() const
    {
        return this->end_delay != 0;
    }
};
