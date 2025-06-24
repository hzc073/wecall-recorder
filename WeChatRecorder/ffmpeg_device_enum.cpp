#include "ffmpeg_device_enum.h"
#include "string_util.h"
#include "log.h"

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
}

static void InitFFmpegOnce()
{
    static bool inited = false;
    if (!inited) {
        avdevice_register_all();
        inited = true;
    }
}

static std::wstring ToW(const char* s) {
    return MultiByteToWide(s);
}

static std::vector<FFmpegDeviceInfo> EnumDevices(bool wantInput)
{
    InitFFmpegOnce();
    std::vector<FFmpegDeviceInfo> listOut;

    const AVInputFormat* ifmt = av_find_input_format("dshow");
    if (!ifmt) {
        WriteLog(L"[Enum] 找不到 dshow 输入格式");
        return listOut;
    }

    AVDeviceInfoList* devList = nullptr;
    int ret = avdevice_list_input_sources(ifmt, NULL, NULL, &devList);
    if (ret < 0 || !devList) {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        WriteLog(L"[Enum] avdevice_list_input_sources 失败, ret=%d (%hs)", ret, errbuf);
        return listOut;
    }

    WriteLog(L"[Enum] 找到 %d 个设备", devList->nb_devices);

    // 旧版 FFmpeg 没有设备类型信息
    // 假设所有设备都是输入设备（旧版 FFmpeg 只返回输入设备）
    for (int i = 0; i < devList->nb_devices; ++i) {
        AVDeviceInfo* device = devList->devices[i];

        FFmpegDeviceInfo info;
        info.id = ToW(device->device_name);
        info.name = ToW(device->device_description);

        // 在旧版 FFmpeg 中，所有设备都被视为输入设备
        if (wantInput) {
            listOut.push_back(info);
            WriteLog(L"  - 输入设备 %d: %s (ID: %s)",
                i, info.name.c_str(), info.id.c_str());
        }
    }

    avdevice_free_list_devices(&devList);

    WriteLog(L"[Enum] %s 设备枚举到 %d 个",
        wantInput ? L"输入" : L"输出",
        (int)listOut.size());
    return listOut;
}

std::vector<FFmpegDeviceInfo> FFmpegEnumInputDevices() {
    return EnumDevices(true);
}

std::vector<FFmpegDeviceInfo> FFmpegEnumOutputDevices() {
    // 在旧版 FFmpeg 中，输出设备无法通过 dshow 枚举
    // 返回空列表或尝试其他方法
    return std::vector<FFmpegDeviceInfo>();
}