#pragma once
#include <YY/Base/YY.h>

#pragma pack(push, __YY_PACKING)

namespace YY {
namespace Base {
namespace Security {

class ThreadImpersonationScope
{
private:
    HANDLE hPreviousToken = nullptr;
    LSTATUS lStatusImpersonation = ERROR_FUNCTION_NOT_CALLED;

public:
    explicit ThreadImpersonationScope(_In_opt_ HANDLE _hImpersonationToken)
    {
        if (!OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_QUERY, TRUE, &hPreviousToken))
        {
            lStatusImpersonation = GetLastError();
            if (lStatusImpersonation != ERROR_NO_TOKEN)
            {
                return;
            }
        }

        lStatusImpersonation = SetThreadToken(NULL, _hImpersonationToken) ? ERROR_SUCCESS : GetLastError();
    }

    ThreadImpersonationScope(const ThreadImpersonationScope&) = delete;
    ThreadImpersonationScope& operator=(const ThreadImpersonationScope&) = delete;

    ~ThreadImpersonationScope()
    {
        if (lStatusImpersonation == ERROR_SUCCESS)
        {
            SetThreadToken(NULL, hPreviousToken);
        }

        if (hPreviousToken)
        {
            CloseHandle(hPreviousToken);
        }
    }

    LSTATUS __YYAPI GetImpersonationStatus() const
    {
        return lStatusImpersonation;
    }
};

} // namespace Security
} // namespace Base

using namespace YY::Base::Security;
} // namespace YY

#pragma pack(pop)
