#pragma once
#include <YY/Base/YY.h>

#if defined(_HAS_CXX20) && _HAS_CXX20
#include <coroutine>

#include <YY/Base/Containers/Optional.h>
#include <YY/Base/Sync/Interlocked.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            template<typename ReturnType_, typename TaskType>
            struct Promise
            {
                using ReturnType = ReturnType_;
                YY::Optional<ReturnType> oValue;

                TaskType get_return_object() noexcept
                {
                    return {};
                }
                std::suspend_never initial_suspend() noexcept
                {
                    return {};
                }

                std::suspend_never final_suspend() noexcept
                {
                    return {};
                }

                void return_value(ReturnType&& _oValue) noexcept
                {
                    oValue.Emplace(std::move(_oValue));
                }

                void unhandled_exception()
                {
                }
            };

            template<typename TaskType>
            struct Promise<void, TaskType>
            {
                using ReturnType = void;

                TaskType get_return_object() noexcept
                {
                    return {};
                }
                std::suspend_never initial_suspend() noexcept
                {
                    return {};
                }

                std::suspend_never final_suspend() noexcept
                {
                    return {};
                }

                void return_void() noexcept
                {
                }
                void unhandled_exception()
                {
                }
            };


            template<typename _ReturnType>
            struct Coroutine
            {
                using promise_type = Promise<_ReturnType, Coroutine>;
            };

            template<typename ResumeType_>
            class TaskAwaiter
            {
            public:
                using ResumeType = ResumeType_;

                class RefData
                {
                public:
                    // 协程句柄
                    intptr_t hCoroutineHandle = 0;

                    ~RefData()
                    {
                        if (hCoroutineHandle && hCoroutineHandle != (intptr_t)-1)
                        {
                            std::coroutine_handle<>::from_address((void*)hCoroutineHandle).destroy();
                        }
                    }

                    virtual uint32_t __YYAPI AddRef() noexcept = 0;

                    virtual uint32_t __YYAPI Release() noexcept = 0;

                    virtual ResumeType_ __YYAPI Resume() noexcept = 0;
                };

            private:
                RefPtr<RefData> pAwaiterData;

            public:
                TaskAwaiter(_In_ RefPtr<RefData> _pAwaiterData)
                    : pAwaiterData(std::move(_pAwaiterData))
                {
                }

                TaskAwaiter(TaskAwaiter&&) = default;

                TaskAwaiter(const TaskAwaiter&) = delete;
                TaskAwaiter& operator=(const TaskAwaiter&) = delete;

                bool await_ready() noexcept
                {
                    return pAwaiterData->hCoroutineHandle == /*hReadyHandle*/ (intptr_t)-1;
                }

                bool await_suspend(std::coroutine_handle<> _hHandle) noexcept
                {
                    return YY::Base::Sync::CompareExchange(&pAwaiterData->hCoroutineHandle, (intptr_t)_hHandle.address(), 0) == 0;
                }

                ResumeType await_resume() noexcept
                {
                    return pAwaiterData->Resume();
                }
            };

            template<>
            class TaskAwaiter<void>
            {
            public:
                using ResumeType = void;

                class RefData
                {
                public:
                    // 协程句柄
                    intptr_t hCoroutineHandle = 0;

                    ~RefData()
                    {
                        if (hCoroutineHandle && hCoroutineHandle != (intptr_t)-1)
                        {
                            std::coroutine_handle<>::from_address((void*)hCoroutineHandle).destroy();
                        }
                    }

                    virtual uint32_t __YYAPI AddRef() noexcept = 0;

                    virtual uint32_t __YYAPI Release() noexcept = 0;
                };

            private:
                RefPtr<RefData> pAwaiterData;

            public:
                TaskAwaiter(_In_ RefPtr<RefData> _pAwaiterData)
                    : pAwaiterData(std::move(_pAwaiterData))
                {
                }

                TaskAwaiter(TaskAwaiter&&) = default;

                TaskAwaiter(const TaskAwaiter&) = delete;
                TaskAwaiter& operator=(const TaskAwaiter&) = delete;

                TaskAwaiter& operator=(TaskAwaiter&&) = default;

                bool await_ready() noexcept
                {
                    return pAwaiterData->hCoroutineHandle == /*hReadyHandle*/ (intptr_t)-1;
                }

                bool await_suspend(std::coroutine_handle<> _hHandle) noexcept
                {
                    return YY::Base::Sync::CompareExchange(&pAwaiterData->hCoroutineHandle, (intptr_t)_hHandle.address(), 0) == 0;
                }

                void await_resume() noexcept
                {
                    // 因为没有返回值，所以空函数占坑
                }
            };
        }
    }

    using namespace YY::Base::Threading;
}
#pragma pack(pop)

#endif
