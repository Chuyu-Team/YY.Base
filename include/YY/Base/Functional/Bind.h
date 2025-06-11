#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Exception.h>
#include <YY/Base/ErrorCode.h>
#include <functional>

#include <YY/Base/Memory/UniquePtr.h>
#include <YY/Base/Memory/RefPtr.h>
#include <YY/Base/Memory/ObserverPtr.h>
#include <YY/Base/Memory/WeakPtr.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Functional
        {
            /// <summary>
            /// Bind裸指针占位使用。一般来说，不应该使用这个类。请优先考虑UniquePtr、RefPtr、ObserverPtr、WeakPtr。
            /// </summary>
            /// <typeparam name="Type"></typeparam>
            template<typename Type>
            class UnsafeBindPtr
            {
            private:
                Type* p = nullptr;

            public:
                constexpr UnsafeBindPtr() noexcept = default;

                constexpr UnsafeBindPtr(_In_opt_ Type* _pThis) noexcept
                    : p(_pThis)
                {
                }

                UnsafeBindPtr(const UnsafeBindPtr&) = default;

                _Ret_maybenull_ Type* __YYAPI Get() const noexcept
                {
                    return p;
                }

                _Ret_maybenull_ __YYAPI operator Type* () noexcept
                {
                    return p;
                }

                _Ret_maybenull_ __YYAPI operator const Type* () const noexcept
                {
                    return p;
                }

                _Ret_maybenull_ Type* __YYAPI operator->() const noexcept
                {
                    return p;
                }
            };

            template<typename Function, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(Function&& _pfnFunction, InputParameters&&... _InputParameters)
            {
                return std::bind(std::forward<Function>(_pfnFunction), std::forward<InputParameters>(_InputParameters)...);
            }

#if defined(_WIN32) && defined(_X86_)
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__thiscall FunctionClass::* _pfnFunction)(FunctionParameters...), InputClass* _pThis, InputParameters&&... _InputParameters)
            {
                static_assert(false, "不安全的成员函数绑定，请对_pThis使用UniquePtr、RefPtr、ObserverPtr、WeakPtr之一。如果自行可以保证绝对内存安全，则使用UnsafeBindPtr。");
                return std::bind(_pfnFunction, _pThis, _InputParameters...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(_cdecl FunctionClass::* _pfnFunction)(FunctionParameters...), InputClass* _pThis, InputParameters&&... _InputParameters)
            {
                static_assert(false, "不安全的成员函数绑定，请对_pThis使用UniquePtr、RefPtr、ObserverPtr、WeakPtr之一。如果自行可以保证绝对内存安全，则使用UnsafeBindPtr。");
                return std::bind(_pfnFunction, _pThis, _InputParameters...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__stdcall FunctionClass::* _pfnFunction)(FunctionParameters...), InputClass* _pThis, InputParameters&&... _InputParameters)
            {
                static_assert(false, "不安全的成员函数绑定，请对_pThis使用UniquePtr、RefPtr、ObserverPtr、WeakPtr之一。如果自行可以保证绝对内存安全，则使用UnsafeBindPtr。");
                return std::bind(_pfnFunction, _pThis, _InputParameters...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__fastcall FunctionClass::* _pfnFunction)(FunctionParameters...), InputClass* _pThis, InputParameters&&... _InputParameters)
            {
                static_assert(false, "不安全的成员函数绑定，请对_pThis使用UniquePtr、RefPtr、ObserverPtr、WeakPtr之一。如果自行可以保证绝对内存安全，则使用UnsafeBindPtr。");
                return std::bind(_pfnFunction, _pThis, _InputParameters...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__thiscall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::UniquePtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__cdecl FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::UniquePtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__stdcall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::UniquePtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__fastcall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::UniquePtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__thiscall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::RefPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__cdecl FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::RefPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__stdcall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::RefPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__fastcall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::RefPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 ObserverPtr 或 WeakPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__thiscall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::ObserverPtr<InputClass> _pThisObserver, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThisObserver](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        auto _pThis = _pThisObserver.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: ObserverPtr is null."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 ObserverPtr 或 WeakPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__cdecl FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::ObserverPtr<InputClass> _pThisObserver, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThisObserver](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        auto _pThis = _pThisObserver.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: ObserverPtr is null."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 ObserverPtr 或 WeakPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__stdcall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::ObserverPtr<InputClass> _pThisObserver, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThisObserver](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        auto _pThis = _pThisObserver.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: ObserverPtr is null."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }


            /// <summary>
            /// 安全版本捕获，使用 ObserverPtr 或 WeakPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__fastcall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::ObserverPtr<InputClass> _pThisObserver, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThisObserver](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        auto _pThis = _pThisObserver.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: ObserverPtr is null."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 ObserverPtr 或 WeakPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__thiscall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::WeakPtr<InputClass> _pThisWeak, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThisWeak](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        auto _pThis = _pThisWeak.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: WeakPtr is null."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 ObserverPtr 或 WeakPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__cdecl FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::WeakPtr<InputClass> _pThisWeak, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThisWeak](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        auto _pThis = _pThisWeak.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: WeakPtr is null."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 ObserverPtr 或 WeakPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__stdcall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::WeakPtr<InputClass> _pThisWeak, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThisWeak](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        auto _pThis = _pThisWeak.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: WeakPtr is null."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 ObserverPtr 或 WeakPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__fastcall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::WeakPtr<InputClass> _pThisWeak, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThisWeak](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        auto _pThis = _pThisWeak.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: WeakPtr is null."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__thiscall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Functional::UnsafeBindPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__cdecl FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Functional::UnsafeBindPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__stdcall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Functional::UnsafeBindPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(__fastcall FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Functional::UnsafeBindPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }
#else
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(FunctionClass::* _pfnFunction)(FunctionParameters...), InputClass* _pThis, InputParameters&&... _InputParameters)
            {
                static_assert(false, "不安全的成员函数绑定，请对_pThis使用UniquePtr、RefPtr、ObserverPtr、WeakPtr之一。如果自行可以保证绝对内存安全，则使用UnsafeBindPtr。");
                return std::bind(_pfnFunction, _pThis, _InputParameters...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::UniquePtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::RefPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 ObserverPtr 或 WeakPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::ObserverPtr<InputClass> _pThisObserver, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThisObserver](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        auto _pThis = _pThisObserver.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: ObserverPtr is null."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 ObserverPtr 或 WeakPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Memory::WeakPtr<InputClass> _pThisWeak, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThisWeak](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        auto _pThis = _pThisWeak.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: WeakPtr is null."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            template<typename FunctionReturn, typename FunctionClass, typename ...FunctionParameters, typename InputClass, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionReturn(FunctionClass::* _pfnFunction)(FunctionParameters...), YY::Base::Functional::UnsafeBindPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](FunctionParameters... _Parameters) mutable->FunctionReturn
                    {
                        return (_pThis->*_pfnFunction)(std::forward<FunctionParameters>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }
#endif
        } // namespace Functional
    } // namespace Base
} // namespace YY

namespace YY
{
    using namespace YY::Base::Functional;
} // namespace YY

#pragma pack(pop)
