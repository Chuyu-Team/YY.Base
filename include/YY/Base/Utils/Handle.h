#pragma once
#include <YY/Base/YY.h>
#include <type_traits>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Utils
        {
#if defined(_WIN32)
            struct Win32HandleTraits
            {
                using HandleType = HANDLE;

                static constexpr HandleType khInvalidHandle = NULL;

                static inline HRESULT __YYAPI CloseHandle(HandleType _hHandle)
                {
                    if(!::CloseHandle(_hHandle))
                        return __HRESULT_FROM_WIN32(GetLastError());

                    return S_OK;
                }

                // 可选实现 DuplicateHandle 方法以支持复制句柄功能
                static inline HRESULT __YYAPI DuplicateHandle(HandleType _hHandle, HandleType* _phNewHandle)
                {
                    if(!::DuplicateHandle(GetCurrentProcess(), _hHandle, GetCurrentProcess(), _phNewHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
                        return __HRESULT_FROM_WIN32(GetLastError());

                    return S_OK;
                }
            };
#endif

            /// <summary>
            /// 原始句柄的简易包装类。注意此类不可拷贝，如果需要拷贝请确保 HandleTraits 定义了 DuplicateHandle 方法。
            /// </summary>
            /// <typeparam name="HandleTraits">需要定义HandleType等信息，可参考 Win32HandleTraits 实现。</typeparam>
            template<typename HandleTraits, typename = void>
            class Handle
            {
            public:
                using HandleType = typename HandleTraits::HandleType;

                static constexpr HandleType khInvalidHandle = HandleTraits::khInvalidHandle;

            private:
                HandleType hHandle = khInvalidHandle;

            public:
                constexpr Handle() noexcept = default;

                constexpr explicit Handle(HandleType _hHandle) noexcept
                    : hHandle(_hHandle)
                {
                }

                Handle(const Handle& _hHandle) = delete;
                Handle& __YYAPI operator=(const Handle& _hOther) = delete;

                constexpr Handle(Handle&& _hHandle) noexcept
                    : hHandle(_hHandle.Detach())
                {
                }

                ~Handle()
                {
                    Release();
                }

                constexpr bool __YYAPI IsValid() const noexcept
                {
                    return hHandle != khInvalidHandle;
                }

                constexpr bool __YYAPI IsInvalid() const noexcept
                {
                    return hHandle == khInvalidHandle;
                }

                constexpr HandleType __YYAPI Get() const noexcept
                {
                    return hHandle;
                }

                _Ret_notnull_ HandleType* __YYAPI ReleaseAndGetAddressOf()
                {
                    Release();
                    return &hHandle;
                }

                void __YYAPI Attach(HandleType _hHandle) noexcept
                {
                    if (hHandle == _hHandle)
                        return;

                    Release();
                    hHandle = _hHandle;
                }

                constexpr HandleType __YYAPI Detach() noexcept
                {
                    HandleType _hHandle = hHandle;
                    hHandle = khInvalidHandle;
                    return _hHandle;
                }

                void __YYAPI Release()
                {
                    if (!IsValid())
                        return;

                    HandleTraits::CloseHandle(hHandle);
                    hHandle = khInvalidHandle;
                }

                Handle& __YYAPI operator=(HandleType _hOther)
                {
                    Attach(_hOther);
                    return *this;
                }

                Handle& __YYAPI operator=(Handle&& _hOther) noexcept
                {
                    Attach(_hOther.Detach());
                    return *this;
                }

                constexpr bool __YYAPI operator==(const Handle& _hOther) const noexcept
                {
                    return hHandle == _hOther.hHandle;
                }

                constexpr bool __YYAPI operator==(HandleType _hOther) const noexcept
                {
                    return hHandle == _hOther;
                }

                constexpr bool __YYAPI operator!=(const Handle& _hOther) const noexcept
                {
                    return hHandle != _hOther.hHandle;
                }

                constexpr bool __YYAPI operator!=(HandleType _hOther) const noexcept
                {
                    return hHandle != _hOther;
                }
            };

            /// <summary>
            /// 原始句柄的简易包装类。并且允许复制句柄。
            /// </summary>
            /// <typeparam name="HandleTraits">需要定义HandleType等信息，可参考 Win32HandleTraits 实现。</typeparam>
            template<typename HandleTraits>
            class Handle<HandleTraits, std::void_t<decltype(&HandleTraits::DuplicateHandle)>>
            {
            public:
                using HandleType = typename HandleTraits::HandleType;

                static constexpr HandleType khInvalidHandle = HandleTraits::khInvalidHandle;

            private:
                HandleType hHandle = khInvalidHandle;

            public:
                constexpr Handle() noexcept = default;

                constexpr explicit Handle(HandleType _hHandle) noexcept
                    : hHandle(_hHandle)
                {
                }

                Handle(const Handle& _hHandle)
                    : hHandle(_hHandle.Clone())
                {
                }

                constexpr Handle(Handle&& _hHandle) noexcept
                    : hHandle(_hHandle.Detach())
                {
                }

                ~Handle()
                {
                    Release();
                }

                constexpr bool __YYAPI IsValid() const noexcept
                {
                    return hHandle != khInvalidHandle;
                }

                constexpr bool __YYAPI IsInvalid() const noexcept
                {
                    return hHandle == khInvalidHandle;
                }

                constexpr HandleType __YYAPI Get() const noexcept
                {
                    return hHandle;
                }

                _Ret_notnull_ HandleType* __YYAPI GetAddressOf()
                {
                    return &hHandle;
                }

                _Ret_notnull_ HandleType* __YYAPI ReleaseAndGetAddressOf()
                {
                    Release();
                    return &hHandle;
                }

                void __YYAPI Attach(HandleType _hHandle) noexcept
                {
                    if(hHandle == _hHandle)
                        return;

                    Release();
                    hHandle = _hHandle;
                }

                constexpr HandleType __YYAPI Detach() noexcept
                {
                    HandleType _hHandle = hHandle;
                    hHandle = khInvalidHandle;
                    return _hHandle;
                }

                HandleType __YYAPI Clone() const
                {
                    if(!IsValid())
                        return khInvalidHandle;

                    HandleType _hNewHandle = khInvalidHandle;
                    if (SUCCEEDED(HandleTraits::DuplicateHandle(hHandle, &_hNewHandle)))
                        return _hNewHandle;

                    return khInvalidHandle;
                }

                void __YYAPI Release()
                {
                    if (!IsValid())
                        return;

                    HandleTraits::CloseHandle(hHandle);
                    hHandle = khInvalidHandle;
                }

                Handle& __YYAPI operator=(const Handle& _hOther)
                {
                    Attach(_hOther.Clone());
                    return *this;
                }

                Handle& __YYAPI operator=(HandleType _hOther)
                {
                    Attach(_hOther);
                    return *this;
                }

                Handle& __YYAPI operator=(Handle&& _hOther) noexcept
                {
                    Attach(_hOther.Detach());
                    return *this;
                }

                constexpr bool __YYAPI operator==(const Handle& _hOther) const noexcept
                {
                    return hHandle == _hOther.hHandle;
                }

                constexpr bool __YYAPI operator==(HandleType _hOther) const noexcept
                {
                    return hHandle == _hOther;
                }

                constexpr bool __YYAPI operator!=(const Handle& _hOther) const noexcept
                {
                    return hHandle != _hOther.hHandle;
                }

                constexpr bool __YYAPI operator!=(HandleType _hOther) const noexcept
                {
                    return hHandle != _hOther;
                }
            };
        }
    }

    using namespace YY::Base::Utils;

} // namespace YY

#pragma pack(pop)
