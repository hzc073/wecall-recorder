#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

void WriteLog(const wchar_t* format, ...) {
    FILE* fp = NULL;
    wchar_t logPath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, logPath, MAX_PATH);
    wchar_t* p = wcsrchr(logPath, L'\\');
    if (p) *(p + 1) = 0;
    wcscat_s(logPath, L"wechat_recorder.log");

    _wfopen_s(&fp, logPath, L"a+, ccs=UTF-8");
    if (!fp) return;

    SYSTEMTIME st;
    GetLocalTime(&st);
    fwprintf(fp, L"[%04d-%02d-%02d %02d:%02d:%02d] ",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    va_list args;
    va_start(args, format);
    vfwprintf(fp, format, args);
    va_end(args);

    fwprintf(fp, L"\n");
    fclose(fp);
}
