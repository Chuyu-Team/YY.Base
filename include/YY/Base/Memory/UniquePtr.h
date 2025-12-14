#pragma once

/*

RefPtr 是 类似于shared_ptr的智能指针。
他们不同的是 RefPtr 自生提供引用计数。

因此使用RefPtr的类必须拥有 AddRef 以及 Release

*/

/*
与 std::unique_ptr 作用相同。
*/

#pragma once
#include <YY/Base/YY.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Memory
        {
            template<typename _Type>
            class UniquePtr
            {
            private:
                _Type* p = nullptr;

            public:
                UniquePtr(const UniquePtr& _pOther) = delete;
                UniquePtr& __YYAPI operator=(_In_opt_ const UniquePtr& _pOther) = delete;

                constexpr UniquePtr() = default;

                // 此构造不安全，因此刻意explicit
                explicit constexpr UniquePtr(_Type* _pOther) noexcept
                    : p(_pOther)
                {
                }

                UniquePtr(UniquePtr&& _pOther) noexcept
                    : p(_pOther.Detach())
                {
                }

                ~UniquePtr()
                {
                    Reset();
                }

                template<typename... Args>
                static UniquePtr __YYAPI Create(Args&&... _args) noexcept
                {
                    return UniquePtr(new _Type(std::forward<Args>(_args)...));
                }

                _Ret_maybenull_ _Type* __YYAPI Get() const noexcept
                {
                    return p;
                }

                void __YYAPI Reset(_In_opt_ _Type* _pOther = nullptr) noexcept
                {
                    if (p)
                        delete p;

                    p = _pOther;
                }

                inline void __YYAPI Attach(_In_opt_ _Type* _pOther) noexcept
                {
                    Reset(_pOther);
                }

                inline _Ret_maybenull_ _Type* __YYAPI Detach() noexcept
                {
                    auto _p = p;
                    p = nullptr;
                    return _p;
                }

                inline _Ret_notnull_ _Type** __YYAPI ReleaseAndGetAddressOf() noexcept
                {
                    Reset();
                    return &p;
                }

                inline _Ret_notnull_ _Type** __YYAPI GetAddressOf() noexcept
                {
                    return &p;
                }
                
                /// <summary>
                /// 检查观察者数据是否已过期。
                /// </summary>
                /// <returns>如果对象已过期，则返回 true；否则返回 false。</returns>
                bool __YYAPI IsExpired() const noexcept
                {
                    return p == nullptr;
                }

                _Ret_maybenull_ __YYAPI operator _Type*() const noexcept
                {
                    return p;
                }

                _Ret_maybenull_ _Type* __YYAPI operator->() const noexcept
                {
                    return p;
                }

                UniquePtr& __YYAPI operator=(std::nullptr_t) noexcept
                {
                    Reset();
                    return *this;
                }

                UniquePtr& __YYAPI operator=(_In_opt_ UniquePtr&& _Other) noexcept
                {
                    if (p != _Other.p)
                    {
                        Attach(_Other.Detach());
                    }

                    return *this;
                }

                bool __YYAPI operator==(_In_opt_ const _Type* _pOther) const noexcept
                {
                    return p == _pOther;
                }

                bool __YYAPI operator!=(_In_opt_ const _Type* _pOther) const noexcept
                {
                    return p != _pOther;
                }
            };

            template<typename _Type>
            class UniquePtr<_Type[]>
            {
            private:
                _Type* p = nullptr;

            public:
                UniquePtr(const UniquePtr& _pOther) = delete;
                UniquePtr& __YYAPI operator=(_In_opt_ const UniquePtr& _pOther) = delete;

                constexpr UniquePtr() = default;

                // 此构造不安全，因此刻意explicit
                explicit constexpr UniquePtr(_Type* _pOther) noexcept
                    : p(_pOther)
                {
                }

                UniquePtr(UniquePtr&& _pOther) noexcept
                    : p(_pOther.Detach())
                {
                }

                ~UniquePtr()
                {
                    Reset();
                }

                _Type* __YYAPI Get() const noexcept
                {
                    return p;
                }

                void __YYAPI Reset(_In_opt_ _Type* _pOther = nullptr) noexcept
                {
                    if (p)
                        delete[] p;

                    p = _pOther;
                }

                inline void __YYAPI Attach(_In_opt_ _Type* _pOther) noexcept
                {
                    Reset(_pOther);
                }

                inline _Ret_maybenull_ _Type* __YYAPI Detach() noexcept
                {
                    auto _p = p;
                    p = nullptr;
                    return _p;
                }

                inline _Ret_notnull_ _Type** __YYAPI ReleaseAndGetAddressOf() noexcept
                {
                    Reset();
                    return &p;
                }

                inline _Ret_notnull_ _Type** __YYAPI GetAddressOf() noexcept
                {
                    return &p;
                }

                __YYAPI operator _Type*() const noexcept
                {
                    return p;
                }

                _Type* __YYAPI operator->() const noexcept
                {
                    return p;
                }

                UniquePtr& __YYAPI operator=(std::nullptr_t) noexcept
                {
                    Reset();
                    return *this;
                }

                UniquePtr& __YYAPI operator=(_In_opt_ UniquePtr&& _Other) noexcept
                {
                    if (p != _Other.p)
                    {
                        Attach(_Other.Detach());
                    }

                    return *this;
                }

                bool __YYAPI operator==(_In_opt_ const _Type* _pOther) const noexcept
                {
                    return p == _pOther;
                }

                bool __YYAPI operator!=(_In_opt_ const _Type* _pOther) const noexcept
                {
                    return p != _pOther;
                }
            };
        } // namespace Memory
    } // namespace Base

    using namespace YY::Base::Memory;
} // namespace YY

#pragma pack(pop)
