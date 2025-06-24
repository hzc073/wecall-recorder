#pragma once
#include <atomic>
#include <thread>
#include <string>

class MicLevelSampler {
public:
    MicLevelSampler();
    ~MicLevelSampler();

    // The Start method now accepts a device ID to test a specific microphone
    void Start(const std::wstring& deviceId);
    void Stop();
    float GetLevel() const; // 0.0~1.0

private:
    std::atomic<bool> running;
    std::atomic<float> level;
    std::thread worker;
    std::wstring m_deviceId;
    void SampleLoop();
};
