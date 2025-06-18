#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Exception.h>
#include <YY/Base/Sync/Interlocked.h>
#include <YY/Base/Containers/ConstructorPolicy.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Containers
        {
            template<typename _Type>
            class Optional
            {
            private:
                union
                {
                    char oValueBuffer[sizeof(_Type)] = {};
                    _Type oValue;
                };

                bool bHasValue = false;

            public:
                constexpr Optional() = default;

                constexpr Optional(const _Type& _oValue)
                    : oValue(_oValue)
                    , bHasValue(true)
                {
                }
                
                constexpr Optional(_Type&& _oValue)
                    : oValue(std::move(_oValue))
                    , bHasValue(true)
                {
                }

                constexpr Optional(const Optional& _oOther)
                {
                    if (_oOther.bHasValue)
                    {
                        new (&oValue) _Type(_oOther.oValue);
                        bHasValue = true;
                    }
                }

                constexpr Optional(Optional&& _oOther) noexcept
                {
                    if (_oOther.bHasValue)
                    {
                        new (&oValue) _Type(std::move(_oOther.oValue));
                        _oOther.Reset();
                        bHasValue = true;
                    }
                }

                ~Optional()
                {
                    Reset();
                }

                inline bool __YYAPI HasValue() const noexcept
                {
                    return bHasValue;
                }

                _Ret_maybenull_ _Type* __YYAPI GetValuePtr() noexcept
                {
                    return bHasValue ? &oValue : nullptr;
                }

                _Ret_maybenull_ inline const _Type* __YYAPI GetValuePtr() const noexcept
                {
                    return const_cast<Optional*>(this)->GetValuePtr();
                }

                inline _Type& __YYAPI GetValue()
                {
                    return *GetValuePtr();
                }

                inline const _Type& __YYAPI GetValue() const
                {
                    return *GetValuePtr();
                }

                template<typename... Args>
                _Type& __YYAPI Emplace(Args&&... oArgs)
                {
                    Reset();
                    new (&oValue) _Type(std::forward<Args>(oArgs)...);
                    bHasValue = true;
                    return oValue;
                }

                void __YYAPI Reset()
                {
                    if (bHasValue)
                    {
                        oValue.~_Type();
                        bHasValue = false;
                    }
                }

                Optional& __YYAPI operator=(const _Type& _oValue)
                {
                    if (bHasValue)
                    {
                        if (&oValue != &_oValue)
                        {
                            oValue = _oValue;
                        }
                    }
                    else
                    {
                        new (&oValue) _Type(_oValue);
                        bHasValue = true;
                    }

                    return *this;
                }

                Optional& __YYAPI operator=(_Type&& _oValue)
                {
                    if (bHasValue)
                    {
                        if (&oValue != &_oValue)
                        {
                            oValue = std::move(_oValue);
                        }
                    }
                    else
                    {
                        new (&oValue) _Type(std::move(_oValue));
                        bHasValue = true;
                    }

                    return *this;
                }

                Optional& __YYAPI operator=(const Optional& _oOther)
                {
                    if (_oOther.HasValue())
                    {
                        operator=(_oOther.GetValue());
                    }
                    else
                    {
                        Reset();
                    }

                    return *this;
                }

                Optional& __YYAPI operator=(Optional&& _oOther)
                {
                    if(this == &_oOther)
                        return *this;

                    if (_oOther.HasValue())
                    {
                        operator=(std::move(_oOther.GetValue()));
                        _oOther.Reset();
                    }
                    else
                    {
                        Reset();
                    }

                    return *this;
                }
                
                bool __YYAPI operator==(const Optional& _oOther) const noexcept
                {
                    if (HasValue() != _oOther.HasValue())
                        return false;

                    if (!HasValue())
                    {
                        return true;
                    }

                    return GetValue() == _oOther.GetValue();
                }

#if !defined(_HAS_CXX20) || _HAS_CXX20 == 0
                bool __YYAPI operator!=(const Optional& _oOther) const noexcept
                {
                    if (HasValue() != _oOther.HasValue())
                        return true;

                    if (!HasValue())
                    {
                        return false;
                    }

                    return GetValue() != _oOther.GetValue();
                }
#endif
            };
        }
    } // namespace Base

    using namespace YY::Base::Containers;
} // namespace YY

#pragma pack(pop)
