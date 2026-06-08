#pragma once
#include <YY/Base/YY.h>

#pragma pack(push, __YY_PACKING)

namespace YY {
namespace Base {
namespace Process {

/// <summary>
/// 最大限度的降级为低权启动进程。
/// 如果当前进程运行在完整管理员令牌下，则使用受限令牌（LinkedToken）通过 CreateProcessAsUserW 启动；
/// 否则退化为普通 CreateProcessW。
/// </summary>
HRESULT WINAPI LowPrivilegeCreateProcessW(
    _In_opt_ LPCWSTR _szApplicationName,
    _Inout_opt_ LPWSTR _szCommandLine,
    _In_opt_ LPSECURITY_ATTRIBUTES _pProcessAttributes,
    _In_opt_ LPSECURITY_ATTRIBUTES _pThreadAttributes,
    _In_ BOOL _bInheritHandles,
    _In_ DWORD _fCreationFlags,
    _In_opt_ LPVOID _pEnvironment,
    _In_opt_ LPCWSTR _szCurrentDirectory,
    _In_ LPSTARTUPINFOW _pStartupInfo,
    _Out_ LPPROCESS_INFORMATION _pProcessInformation
    );

} // namespace Process
} // namespace Base

using namespace YY::Base::Process;
} //    namespace YY

#pragma pack(pop)
