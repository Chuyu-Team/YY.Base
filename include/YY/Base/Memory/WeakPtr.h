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
            static _Ret_maybenull_ YY::WeakPtrRef<_Type>* __YYAPI GetWeakPtrRef(_In_opt_ _Type* _pWeakRefPtr) noexcept
            {
                if (!_pWeakRefPtr)
                {
                    return nullptr;
                }

                _pWeakRefPtr->AddWeakRef();
                return reinterpret_cast<YY::WeakPtrRef<_Type>*>(_pWeakRefPtr);
            }

            template<typename _Type>
            static YY::RefPtr<_Type> __YYAPI FromWeakPtr(_In_opt_ YY::WeakPtrRef<_Type>* _pWeakRefPtr) noexcept
            {
                auto _pPtr = reinterpret_cast<_Type*>(_pWeakRefPtr);
                if (_pPtr == nullptr || _pPtr->TryAddRef() == false)
                {
                    return nullptr;
                }

                return YY::RefPtr<_Type>::FromPtr(_pPtr);
            }

            template<typename _Type>
            static void __YYAPI FreeWeakPtrRef(_In_opt_ YY::WeakPtrRef<_Type>* _pWeakRefPtr) noexcept
            {
                auto _pPtr = reinterpret_cast<_Type*>(_pWeakRefPtr);
                if (_pPtr)
                {
                    _pPtr->ReleaseWeak();
                }
            }

            template<typename _Type>
            class WeakPtr
            {
            private:
                WeakPtrRef<_Type>* p = nullptr;

            public:
                constexpr WeakPtr() noexcept = default;

                WeakPtr(_In_opt_ _Type* _pOther) noexcept
                    : p(GetWeakPtrRef(_pOther))
                {
                }

                constexpr WeakPtr(_In_opt_ WeakPtrRef<_Type>* _pOther) noexcept
                    : p(_pOther)
                {
                }

                WeakPtr(_In_ const WeakPtr& _pOther) noexcept
                    : p(GetWeakPtrRef(reinterpret_cast<_Type*>(_pOther.p)))
                {
                }

                WeakPtr(_In_ WeakPtr&& _pOther) noexcept
                    : p(_pOther.p)
                {
                    _pOther.p = nullptr;
                }

                ~WeakPtr()
                {
                    FreeWeakPtrRef(p);
                }

                inline void __YYAPI Attach(_In_opt_ WeakPtrRef<_Type>* _pOther) noexcept
                {
                    FreeWeakPtrRef(p);
                    p = _pOther;
                }

                inline _Ret_maybenull_ WeakPtrRef<_Type>* __YYAPI Detach() noexcept
                {
                    auto _p = p;
                    p = nullptr;
                    return _p;
                }

                _Ret_maybenull_ RefPtr<_Type> __YYAPI Get() const noexcept
                {
                    return FromWeakPtr(p);
                }

                bool __YYAPI operator==(_In_opt_ _Type* _pOther) const noexcept
                {
                    return reinterpret_cast<void*>(p) == reinterpret_cast<void*>(_pOther);
                }

                bool __YYAPI operator==(_In_opt_ const WeakPtr& _pOther) const noexcept
                {
                    return p == _pOther.p;
                }

                bool __YYAPI operator!=(_In_opt_ _Type* _pOther) const noexcept
                {
                    return reinterpret_cast<void*>(p) != reinterpret_cast<void*>(_pOther);
                }

                bool __YYAPI operator!=(_In_opt_ const WeakPtr& _pOther) const noexcept
                {
                    return p != _pOther.p;
                }

                WeakPtr& __YYAPI operator=(std::nullptr_t) noexcept
                {
                    FreeWeakPtrRef(p);
                    p = nullptr;
                    return *this;
                }

                WeakPtr& __YYAPI operator=(_In_opt_ _Type* _pOther) noexcept
                {
                    if (reinterpret_cast<void*>(p) != reinterpret_cast<void*>(_pOther))
                    {
                        FreeWeakPtrRef(p);
                        p = GetWeakPtrRef(_pOther);
                    }
                    return *this;
                }

                WeakPtr& __YYAPI operator=(const WeakPtr& _pOther) noexcept
                {
                    return this->operator=(_pOther.p);
                }

                WeakPtr& __YYAPI operator=(WeakPtr&& _pOther) noexcept
                {
                    if (p != _pOther.p)
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
