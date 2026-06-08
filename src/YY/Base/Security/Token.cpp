#include <YY/Base/Security/Token.h>

#define WIN32_NO_STATUS
#include <YY/Base/Shared/Windows/km.h>
#include <YY/Base/Sync/Interlocked.h>

#include <tlhelp32.h>

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

namespace YY {
namespace Base {
namespace Security {

const Token& __YYAPI Token::GetSystemToken()
{
    static Token s_hSystemToken;

    do
    {
        if (s_hSystemToken.IsValid())
            break;

        auto hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnapshot == INVALID_HANDLE_VALUE)
        {
            break;
        }

        auto pPeb = ((YY::TEB*)NtCurrentTeb())->ProcessEnvironmentBlock;

        const auto _uSessionId = pPeb->SessionId;

        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);

        for (auto bSuccess = Process32FirstW(hProcessSnapshot, &pe); bSuccess; pe.dwSize = sizeof(pe), bSuccess = Process32NextW(hProcessSnapshot, &pe))
        {
            auto szFleName = wcsrchr(pe.szExeFile, L'\\');

            if (szFleName)
                ++szFleName;
            else
                szFleName = pe.szExeFile;

            if (_wcsicmp(L"winlogon.exe", szFleName) != 0)
                continue;

            Handle<Win32HandleTraits> _hProcess(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID));
            if (_hProcess.IsInvalid())
                continue;

            Token _hProcessToken;
            if (!OpenProcessToken(
                _hProcess.Get(),
                TOKEN_DUPLICATE | TOKEN_QUERY,
                _hProcessToken.ReleaseAndGetAddressOf()))
            {
                continue;
            }


            //判断与当前进程是否属于同一会话。
            DWORD _uTargetSessionID;
            if (FAILED(_hProcessToken.GetSessionId(&_uTargetSessionID)))
            {
                continue;
            }

            if (_uTargetSessionID != _uSessionId)
            {
                continue;
            }

            if (YY::Sync::CompareExchangePoint(s_hSystemToken.GetAddressOf(), _hProcessToken.Get(), Token::khInvalidHandle) == Token::khInvalidHandle)
            {
                // 已经赋值给 s_hSystemToken
                _hProcessToken.Detach();
            }
            break;
        }

        CloseHandle(hProcessSnapshot);


    } while (false);

    return s_hSystemToken;
}

const Token& __YYAPI Token::GetLimitedToken()
{
    static Token s_hLimitedToken;

    do
    {
        if (s_hLimitedToken.IsValid())
            break;

        Token _hCurrentProcessToken;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, _hCurrentProcessToken.ReleaseAndGetAddressOf()))
        {
            break;
        }

        TOKEN_ELEVATION_TYPE _eTokenElevationType;
        if (FAILED(_hCurrentProcessToken.GetElevationType(&_eTokenElevationType)))
        {
            break;
        }

        Token _hLimitedToken;
        if (_eTokenElevationType == TokenElevationTypeLimited)
        {
            // 已经是受限令牌，直接使用即可。
            _hLimitedToken = std::move(_hCurrentProcessToken);
        }
        else if (_eTokenElevationType == TokenElevationTypeFull)
        {
            // 此时 LinkedToken 就是受限令牌。
            if (FAILED(_hCurrentProcessToken.GetLinkedToken(_hLimitedToken.ReleaseAndGetAddressOf())))
            {
                break;
            }
        }
        else
        {
            break;
        }

        if (YY::Sync::CompareExchangePoint(s_hLimitedToken.GetAddressOf(), _hLimitedToken.Get(), Token::khInvalidHandle) == Token::khInvalidHandle)
        {
            // 成功赋值。
            _hLimitedToken.Detach();
            break;
        }
    } while (false);

    return s_hLimitedToken;
}

const Token& __YYAPI Token::GetImpersonationSystemToken()
{
    static Token s_hImpersonationSystemToken;
    do
    {
        if (s_hImpersonationSystemToken.IsValid())
            break;

        const Token& _hSystemToken = GetSystemToken();
        if (_hSystemToken.IsInvalid())
        {
            break;
        }

        HANDLE _hImpersonationToken;
        if (!DuplicateTokenEx(
            _hSystemToken.Get(),
            TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID | TOKEN_ADJUST_PRIVILEGES | TOKEN_IMPERSONATE,
            nullptr,
            SecurityImpersonation,
            TokenImpersonation,
            &_hImpersonationToken))
        {
            break;
        }

        if (YY::Sync::CompareExchangePoint(s_hImpersonationSystemToken.GetAddressOf(), _hImpersonationToken, Token::khInvalidHandle) != Token::khInvalidHandle)
        {
            //已经有线程设置成功，无需重复设置，关闭多余的句柄。
            CloseHandle(_hImpersonationToken);
            break;
        }
    } while (false);

    return s_hImpersonationSystemToken;
}

HRESULT __YYAPI Token::GetSessionId(DWORD* _puSessionId) const
{
    DWORD _cbReturnLength;
    if (!GetTokenInformation(Get(), TokenSessionId, _puSessionId, sizeof(DWORD), &_cbReturnLength))
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

HRESULT __YYAPI Token::GetElevationType(TOKEN_ELEVATION_TYPE* _peTokenElevationType) const
{
    DWORD _cbReturnLength;
    if (!GetTokenInformation(Get(), TokenElevationType, _peTokenElevationType, sizeof(TOKEN_ELEVATION_TYPE), &_cbReturnLength))
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

HRESULT __YYAPI Token::GetLinkedToken(HANDLE* _phLinkedToken) const
{
    TOKEN_LINKED_TOKEN _oLinkedToken;
    DWORD _cbReturnLength;
    if (!GetTokenInformation(Get(), TokenLinkedToken, &_oLinkedToken, sizeof(_oLinkedToken), &_cbReturnLength))
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }

    *_phLinkedToken = _oLinkedToken.LinkedToken;
    return S_OK;
}

HRESULT __YYAPI Token::IsElevated(BOOL* _pbTokenIsElevated) const
{
    TOKEN_ELEVATION _oTokenElevation;
    DWORD _cbReturnLength;
    if (!GetTokenInformation(Get(), TokenElevation, &_oTokenElevation, sizeof(_oTokenElevation), &_cbReturnLength))
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }

    *_pbTokenIsElevated = _oTokenElevation.TokenIsElevated;
    return S_OK;
}

} // namespace Security
} // namespace Base
} // namespace YY
