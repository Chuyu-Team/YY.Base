#pragma once
#include <YY/Base/Utils/Handle.h>

#pragma pack(push, __YY_PACKING)

namespace YY {
namespace Base {
namespace Security {

class Token : public Handle<YY::Base::Utils::Win32HandleTraits>
{
public:
    constexpr explicit Token(HANDLE _hToken = khInvalidHandle) noexcept
        : Handle(_hToken)
    {
    }

    Token(const Token& _hToken) = default;

    Token(Token&& _hToken) noexcept = default;

    static const Token& __YYAPI GetSystemToken();

    static const Token& __YYAPI GetLimitedToken();

    static const Token& __YYAPI GetImpersonationSystemToken();

    HRESULT __YYAPI GetSessionId(_Out_ DWORD* _puSessionId) const;

    HRESULT __YYAPI GetElevationType(_Out_ TOKEN_ELEVATION_TYPE* _peTokenElevationType) const;

    HRESULT __YYAPI GetLinkedToken(_Out_ HANDLE* _phLinkedToken) const;

    HRESULT __YYAPI IsElevated(_Out_ BOOL* _pbTokenIsElevated) const;

    Token& __YYAPI operator=(HANDLE _hToken)
    {
        Handle::operator=(_hToken);
        return *this;
    }

    Token& __YYAPI operator=(const Token& _hToken) = default;

    Token& __YYAPI operator=(Token&& _hToken) noexcept = default;
};

} // namespace Security
} // namespace Base

using namespace YY::Base::Security;
} // namespace YY

#pragma pack(pop)
