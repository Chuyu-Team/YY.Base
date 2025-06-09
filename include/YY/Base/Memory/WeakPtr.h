#pragma once

#include <YY/Base/YY.h>
#include <YY/Base/Sync/Interlocked.h>
#include <YY/Base/Memory/Alloc.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Memory
        {
            template<typename _Type>
            class RefPtr;

            template<typename _Type>
            class WeakPtrRef
            {
            };

            template<typename _Type>
            class WeakPtr
            {
            private:
                _Type* p = nullptr;

            public:
                constexpr WeakPtr() noexcept = default;

                WeakPtr(_In_opt_ _Type* _pOther) noexcept
                    : p(_pOther)
                {
                    if (p)
                        p->AddWeakRef();
                }

                constexpr WeakPtr(_In_opt_ WeakPtrRef<_Type>* _pOther) noexcept
                    : p(reinterpret_cast<_Type*>(_pOther))
                {
                }

                WeakPtr(_In_ const WeakPtr& _pOther) noexcept
                    : WeakPtr(_pOther.p)
                {
                }

                WeakPtr(_In_ WeakPtr&& _pOther) noexcept
                    : WeakPtr(_pOther.Detach())
                {
                }

                ~WeakPtr()
                {
                    Reset();
                }

                inline void __YYAPI Attach(_In_opt_ WeakPtrRef<_Type>* _pOther) noexcept
                {
                    if (p)
                        p->ReleaseWeak();
                    p = reinterpret_cast<_Type*>(_pOther);
                }

                inline _Ret_maybenull_ WeakPtrRef<_Type>* __YYAPI Detach() noexcept
                {
                    auto _p = p;
                    p = nullptr;
                    return reinterpret_cast<WeakPtrRef<_Type>*>(_p);
                }

                _Ret_maybenull_ RefPtr<_Type> __YYAPI Get() const noexcept
                {
                    RefPtr<_Type> _pTmp;

                    if (p && p->TryAddRef())
                    {
                        _pTmp.Attach(p);
                    }
                    return _pTmp;
                }

                void __YYAPI Reset() noexcept
                {
                    if (p)
                    {
                        p->ReleaseWeak();
                        p = nullptr;
                    }
                }

                void __YYAPI Reset(_In_opt_ _Type* _pOther) noexcept
                {
                    if (p != _pOther)
                    {
                        if (p)
                            p->ReleaseWeak();

                        p = _pOther;

                        if (p)
                            p->AddWeakRef();
                    }
                }

                bool __YYAPI operator==(_In_opt_ _Type* _pOther) const noexcept
                {
                    return p == _pOther;
                }

                bool __YYAPI operator==(_In_opt_ const WeakPtr& _pOther) const noexcept
                {
                    return p == _pOther.p;
                }

                bool __YYAPI operator!=(_In_opt_ _Type* _pOther) const noexcept
                {
                    return p != _pOther;
                }

                bool __YYAPI operator!=(_In_opt_ const WeakPtr& _pOther) const noexcept
                {
                    return p != _pOther.p;
                }

                WeakPtr& __YYAPI operator=(std::nullptr_t) noexcept
                {
                    Reset();
                    return *this;
                }

                WeakPtr& __YYAPI operator=(_In_opt_ _Type* _pOther) noexcept
                {
                    Reset(_pOther);
                    return *this;
                }

                WeakPtr& __YYAPI operator=(const WeakPtr& _pOther) noexcept
                {
                    Reset(_pOther.p);
                    return *this;
                }

                WeakPtr& __YYAPI operator=(WeakPtr&& _pOther) noexcept
                {
                    if (&_pOther != this)
                    {
                        Attach(_pOther.Detach());
                    }

                    return *this;
                }
            };
        }
    }
}

namespace YY
{
    using namespace YY::Base::Memory;
}

#pragma pack(pop)
