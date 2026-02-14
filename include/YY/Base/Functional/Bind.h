#pragma once
#include <functional>
#include <memory>

#include <YY/Base/YY.h>
#include <YY/Base/Exception.h>
#include <YY/Base/ErrorCode.h>
#include <YY/Base/Functional/FunctionTraits.h>
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

            template<typename FunctionType, typename ...InputParameters>
            _NODISCARD auto __YYAPI Bind(FunctionType&& _pfnFunction, InputParameters&&... _InputParameters)
            {
                return std::bind(std::forward<FunctionType>(_pfnFunction), std::forward<InputParameters>(_InputParameters)...);
            }


            template<typename FunctionType, typename InputClass, typename ...InputParameters, typename ThisObject = typename FunctionTraits<FunctionType>::ThisObject, typename = typename std::enable_if< std::is_convertible<InputClass*, ThisObject*>::value >::type>
            _NODISCARD auto __YYAPI Bind(FunctionType _pfnFunction, InputClass* _pThis, InputParameters&&... _InputParameters)
            {
                static_assert(false, "不安全的成员函数绑定，请对_pThis使用YY::UniquePtr、YY::RefPtr、YY::ObserverPtr、YY::WeakPtr、std::unique_ptr、std::shared_ptr、std::weak_ptr之一。如果自行可以保证绝对内存安全，则使用YY::UnsafeBindPtr。");
                return std::bind(_pfnFunction, _pThis, _InputParameters...);
            }

            /// <summary>
            /// 安全版本捕获，使用 YY::UniquePtr 来捕获 this 指针。确保异步期间对象不会被销毁。
            /// </summary>
            /// <param name="_pfnFunction">需要绑定的函数</param>
            /// <param name="_pThis">YY::UniquePtr对象，保存了this指针</param>
            /// <param name="..._InputParameters">绑定的参数。</param>
            /// <returns>std::bind对象</returns>
            template<typename FunctionType, typename InputClass, typename ...InputParameters, typename ThisObject = typename FunctionTraits<FunctionType>::ThisObject, typename = typename std::enable_if< std::is_convertible<InputClass*, ThisObject*>::value >::type>
            _NODISCARD auto __YYAPI Bind(FunctionType _pfnFunction, YY::Base::Memory::UniquePtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                using ReturnType = typename FunctionTraits<FunctionType>::ReturnType;
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](auto&&... _Parameters) mutable -> ReturnType
                    {
                        return (_pThis->*_pfnFunction)(std::forward<decltype(_Parameters)>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 std::unique_ptr 来捕获 this 指针。确保异步期间对象不会被销毁。
            /// </summary>
            /// <param name="_pfnFunction">需要绑定的函数</param>
            /// <param name="_pThis">std::unique_ptr对象，保存了this指针</param>
            /// <param name="..._InputParameters">绑定的参数。</param>
            /// <returns>std::bind对象</returns>
            template<typename FunctionType, typename InputClass, typename ...InputParameters, typename ThisObject = typename FunctionTraits<FunctionType>::ThisObject, typename = typename std::enable_if< std::is_convertible<InputClass*, ThisObject*>::value >::type>
            _NODISCARD auto __YYAPI Bind(FunctionType _pfnFunction, std::unique_ptr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                using ReturnType = typename FunctionTraits<FunctionType>::ReturnType;
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](auto&&... _Parameters) mutable -> ReturnType
                    {
                        return (_pThis.get()->*_pfnFunction)(std::forward<decltype(_Parameters)>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 YY::ObserverPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            /// <param name="_pfnFunction">需要绑定的函数</param>
            /// <param name="_pThisObserver">YY::ObserverPtr对象，保存了this指针</param>
            /// <param name="..._InputParameters">绑定的参数。</param>
            /// <returns>std::bind对象</returns>
            template<typename FunctionType, typename InputClass, typename ...InputParameters, typename ThisObject = typename FunctionTraits<FunctionType>::ThisObject, typename = typename std::enable_if< std::is_convertible<InputClass*, ThisObject*>::value >::type>
            _NODISCARD auto __YYAPI Bind(FunctionType _pfnFunction, YY::Base::Memory::ObserverPtr<InputClass> _pThisObserver, InputParameters&&... _InputParameters)
            {
                using ReturnType = typename FunctionTraits<FunctionType>::ReturnType;
                return std::bind(
                    [_pfnFunction, _pThisObserver](auto&&... _Parameters) mutable -> ReturnType
                    {
                        auto _pThis = _pThisObserver.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: ObserverPtr is expired."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<decltype(_Parameters)>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 YY::RefPtr 来捕获 this 指针。确保异步期间对象不会被销毁。
            /// </summary>
            /// <param name="_pfnFunction">需要绑定的函数</param>
            /// <param name="_pThis">YY::RefPtr对象，保存了this指针</param>
            /// <param name="..._InputParameters">绑定的参数。</param>
            /// <returns>std::bind对象</returns>
            template<typename FunctionType, typename InputClass, typename ...InputParameters, typename ThisObject = typename FunctionTraits<FunctionType>::ThisObject, typename = typename std::enable_if< std::is_convertible<InputClass*, ThisObject*>::value >::type>
            _NODISCARD auto __YYAPI Bind(FunctionType _pfnFunction, YY::Base::Memory::RefPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                using ReturnType = typename FunctionTraits<FunctionType>::ReturnType;
                return std::bind(
                    [_pfnFunction, _pThis](auto&&... _Parameters) mutable -> ReturnType
                    {
                        return (_pThis->*_pfnFunction)(std::forward<decltype(_Parameters)>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 YY::WeakPtr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            /// <param name="_pfnFunction">需要绑定的函数</param>
            /// <param name="_pThisWeak">YY::WeakPtr对象，保存了this指针</param>
            /// <param name="..._InputParameters">绑定的参数。</param>
            /// <returns>std::bind对象</returns>
            template<typename FunctionType, typename InputClass, typename ...InputParameters, typename ThisObject = typename FunctionTraits<FunctionType>::ThisObject, typename = typename std::enable_if< std::is_convertible<InputClass*, ThisObject*>::value >::type>
            _NODISCARD auto __YYAPI Bind(FunctionType _pfnFunction, YY::Base::Memory::WeakPtr<InputClass> _pThisWeak, InputParameters&&... _InputParameters)
            {
                using ReturnType = typename FunctionTraits<FunctionType>::ReturnType;
                return std::bind(
                    [_pfnFunction, _pThisWeak](auto&&... _Parameters) mutable -> ReturnType
                    {
                        auto _pThis = _pThisWeak.Get();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: WeakPtr is expired."));
                        }

                        return (_pThis->*_pfnFunction)(std::forward<decltype(_Parameters)>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 std::shared_ptr 来捕获 this 指针。确保异步期间对象不会被销毁。
            /// </summary>
            /// <param name="_pfnFunction">需要绑定的函数</param>
            /// <param name="_pThis">std::shared_ptr对象，保存了this指针</param>
            /// <param name="..._InputParameters">绑定的参数。</param>
            /// <returns>std::bind对象</returns>
            template<typename FunctionType, typename InputClass, typename ...InputParameters, typename ThisObject = typename FunctionTraits<FunctionType>::ThisObject, typename = typename std::enable_if< std::is_convertible<InputClass*, ThisObject*>::value >::type>
            _NODISCARD auto __YYAPI Bind(FunctionType _pfnFunction, std::shared_ptr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                using ReturnType = typename FunctionTraits<FunctionType>::ReturnType;
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](auto&&... _Parameters) mutable -> ReturnType
                    {
                        return (_pThis.get()->*_pfnFunction)(std::forward<decltype(_Parameters)>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 安全版本捕获，使用 std::weak_ptr 来捕获 this 指针。注意，如果对象已经被销毁，此时进行调用将抛出 YY::Base::OperationCanceledException 异常。
            /// </summary>
            /// <param name="_pfnFunction">需要绑定的函数</param>
            /// <param name="_pThisWeak">std::weak_ptr对象，保存了this指针</param>
            /// <param name="..._InputParameters">绑定的参数。</param>
            /// <returns>std::bind对象</returns>
            template<typename FunctionType, typename InputClass, typename ...InputParameters, typename ThisObject = typename FunctionTraits<FunctionType>::ThisObject, typename = typename std::enable_if< std::is_convertible<InputClass*, ThisObject*>::value >::type>
            _NODISCARD auto __YYAPI Bind(FunctionType _pfnFunction, std::weak_ptr<InputClass> _pThisWeak, InputParameters&&... _InputParameters)
            {
                using ReturnType = typename FunctionTraits<FunctionType>::ReturnType;
                return std::bind(
                    [_pfnFunction, _pThisWeak = std::move(_pThisWeak)](auto&&... _Parameters) mutable -> ReturnType
                    {
                        auto _pThis = _pThisWeak.lock();
                        if (!_pThis)
                        {
                            throw YY::Base::OperationCanceledException(_S("Call Failed: weak_ptr is expired."));
                        }

                        return (_pThis.get()->*_pfnFunction)(std::forward<decltype(_Parameters)>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }

            /// <summary>
            /// 非安全版本捕获，使用裸指针来捕获 this 指针。请确保在调用时对象绝对内存安全，否则可能会导致未定义行为！！
            /// </summary>
            /// <param name="_pfnFunction">需要绑定的函数</param>
            /// <param name="_pThis">YY::UnsafeBindPtr对象，保存了this指针</param>
            /// <param name="..._InputParameters">绑定的参数。</param>
            /// <returns>std::bind对象</returns>
            template<typename FunctionType, typename InputClass, typename ...InputParameters, typename ThisObject = typename FunctionTraits<FunctionType>::ThisObject, typename = typename std::enable_if< std::is_convertible<InputClass*, ThisObject*>::value >::type>
            _NODISCARD auto __YYAPI Bind(FunctionType _pfnFunction, YY::Base::Functional::UnsafeBindPtr<InputClass> _pThis, InputParameters&&... _InputParameters)
            {
                using ReturnType = typename FunctionTraits<FunctionType>::ReturnType;
                return std::bind(
                    [_pfnFunction, _pThis = std::move(_pThis)](auto&&... _Parameters) mutable -> ReturnType
                    {
                        return (_pThis->*_pfnFunction)(std::forward<decltype(_Parameters)>(_Parameters)...);
                    },
                    std::forward<InputParameters>(_InputParameters)...);
            }
        } // namespace Functional
    } // namespace Base
} // namespace YY

namespace YY
{
    using namespace YY::Base::Functional;
} // namespace YY

#pragma pack(pop)
