#include <YY/Base/Process/Process.h>

#define WIN32_NO_STATUS
#include <YY/Base/Shared/Windows/km.h>

#include <YY/Base/Security/Token.h>
#include <YY/Base/Security/ThreadImpersonationScope.h>
#include <YY/Base/Utils/AutoCleanup.h>

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

#pragma comment(lib, "ntdll.lib")

namespace YY {
namespace Base {
namespace Process {

static TOKEN_ELEVATION_TYPE __YYAPI GetTokenElevationType(HANDLE _hProcess)
{
    Token hToken;
    if (OpenProcessToken(_hProcess, TOKEN_QUERY | TOKEN_DUPLICATE, hToken.ReleaseAndGetAddressOf()))
    {
        TOKEN_ELEVATION_TYPE _eTokenElevationType;
        if (SUCCEEDED(hToken.GetElevationType(&_eTokenElevationType)))
        {
            return _eTokenElevationType;
        }
    }

    return TOKEN_ELEVATION_TYPE::TokenElevationTypeDefault;
}

HRESULT WINAPI LowPrivilegeCreateProcessW(
    LPCWSTR _szApplicationName,
    LPWSTR _szCommandLine,
    LPSECURITY_ATTRIBUTES _pProcessAttributes,
    LPSECURITY_ATTRIBUTES _pThreadAttributes,
    BOOL _bInheritHandles,
    DWORD _fCreationFlags,
    LPVOID _pEnvironment,
    LPCWSTR _szCurrentDirectory,
    LPSTARTUPINFOW _pStartupInfo,
    LPPROCESS_INFORMATION _pProcessInformation
    )
{
    static TOKEN_ELEVATION_TYPE s_eTokenElevationType = GetTokenElevationType(GetCurrentProcess());

    do
    {
        if (s_eTokenElevationType != TokenElevationTypeFull)
        {
            break;
        }

        const auto& _hImpersonationToken = Token::GetImpersonationSystemToken();
        if (_hImpersonationToken.IsInvalid())
        {
            break;
        }

        ThreadImpersonationScope _oImpersonationScope(_hImpersonationToken.Get());

        BOOLEAN _bEnabled = FALSE;
        YY::RtlAdjustPrivilege(SE_IMPERSONATE_PRIVILEGE, TRUE, TRUE, &_bEnabled);
        YY::RtlAdjustPrivilege(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE, TRUE, TRUE, &_bEnabled);

        const auto& _hLimitedToken = Token::GetLimitedToken();
        if (_hLimitedToken.IsInvalid())
        {
            break;
        }

        Token _hPrimaryToken;
        if (!DuplicateTokenEx(
            _hLimitedToken.Get(),
            TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID,
            nullptr,
            SecurityImpersonation,
            TokenPrimary,
            _hPrimaryToken.ReleaseAndGetAddressOf()))
        {
            break;
        }

        const auto _bRet = CreateProcessAsUserW(
            _hPrimaryToken.Get(),
            _szApplicationName,
            _szCommandLine,
            _pProcessAttributes,
            _pThreadAttributes,
            _bInheritHandles,
            _fCreationFlags,
            _pEnvironment,
            _szCurrentDirectory,
            _pStartupInfo,
            _pProcessInformation);

        if (_bRet)
        {
            return S_OK;
        }

        const auto _uLastError = GetLastError();
        if (_uLastError == ERROR_ACCESS_DENIED || _uLastError == ERROR_PRIVILEGE_NOT_HELD)
        {
            // 回退到普通 CreateProcessW，可能是因为系统策略限制了受限令牌的使用。
            break;
        }

        return __HRESULT_FROM_WIN32(_uLastError);
    } while (false);

    // Fallback
    const auto _bRet = CreateProcessW(
        _szApplicationName,
        _szCommandLine,
        _pProcessAttributes,
        _pThreadAttributes,
        _bInheritHandles,
        _fCreationFlags,
        _pEnvironment,
        _szCurrentDirectory,
        _pStartupInfo,
        _pProcessInformation);

    return _bRet ? S_OK : __HRESULT_FROM_WIN32(GetLastError());
}

} // namespace Process
} // namespace Base
} // namespace YY
