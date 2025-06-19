#pragma once
#include <string>

class Recorder {
public:
    virtual ~Recorder() {}
    virtual bool Start(const std::wstring& filename) = 0;
    virtual void Stop() = 0;
    virtual bool IsRecording() const = 0;
};
