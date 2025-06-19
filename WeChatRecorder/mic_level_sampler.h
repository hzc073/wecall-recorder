#pragma once
#include <atomic>
#include <thread>

class MicLevelSampler {
public:
    MicLevelSampler();
    ~MicLevelSampler();

    void Start();
    void Stop();
    float GetLevel() const; // 0.0~1.0

private:
    std::atomic<bool> running;
    std::atomic<float> level;
    std::thread worker;
    void SampleLoop();
};
#pragma once
