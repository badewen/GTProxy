#pragma once
#include <chrono>

// https://github.com/StudioCherno/Walnut/blob/master/Walnut/src/Walnut/Timer.h
namespace utils{

class Timer
{
public:
    Timer(float duration = 0) : m_Duration(duration)
    {
        Reset();
    }
    void Reset()
    {
        m_Start = std::chrono::high_resolution_clock::now();
    }
    float Elapsed()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
    }
    float ElapsedMillis()
    {
        return Elapsed() * 1000.0f;
    }

    bool IsDone() {
        return ElapsedMillis() >= m_Duration;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
    float m_Duration;
};
}
