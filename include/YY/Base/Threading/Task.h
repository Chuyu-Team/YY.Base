#pragma once
#include <type_traits>
#include <utility>
#include <YY/Base/Threading/Async.h>
#include <YY/Base/Memory/WeakPtr.h>
#include <YY/Base/Memory/RefPtr.h>
#include <YY/Base/Functional/FunctionTraits.h>

#if defined(_HAS_CXX20) && _HAS_CXX20
#include <coroutine>
#endif

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            class TaskRunner;

            template<typename SourceResultType_, typename ResultType_, typename CallbackType_>
            class TaskContinueAsyncOperation;

            template<typename ResultType_, typename CallbackType_>
            class TaskAsyncOperation;

#if defined(_HAS_CXX20) && _HAS_CXX20
            template<typename ResultType_>
            class TaskAwaiter;

            template<typename ReturnType_>
            struct Promise;
#endif

            template<typename ResultType_>
            class Task
            {
            public:
                using ResultType = ResultType_;
                using AsyncOperationType = AsyncOperation<ResultType>;
#if defined(_HAS_CXX20) && _HAS_CXX20
                using promise_type = Promise<ResultType_>;
#endif

            private:
                YY::RefPtr<AsyncOperationType> pAsyncOperation;

            public:
                Task(YY::RefPtr<AsyncOperationType> _pAsyncOperation) noexcept
                    : pAsyncOperation(std::move(_pAsyncOperation))
                {
                }

                Task(Task&& _oTher) noexcept
                    : pAsyncOperation(std::move(_oTher.pAsyncOperation))
                {
                }

                Task(const Task& _oTher)
                    : pAsyncOperation(_oTher.pAsyncOperation)
                {
                }

                Task& operator=(const Task& _oTher) = default;

                Task& operator=(Task&& _oTher) noexcept = default;

                YY::RefPtr<AsyncOperationType> GetAsyncOperation() const noexcept
                {
                    return pAsyncOperation;
                }

                HRESULT __YYAPI GetErrorCode() const noexcept
                {
                    return pAsyncOperation->GetErrorCode();
                }

                AsyncStatus __YYAPI GetStatus() const noexcept
                {
                    return pAsyncOperation->GetStatus();
                }

                auto __YYAPI GetResult() const
                {
                    return pAsyncOperation->GetResult();
                }

#if defined(_HAS_CXX20) && _HAS_CXX20
                auto __YYAPI operator co_await() const
                {
                    return TaskAwaiter<ResultType>(pAsyncOperation, TaskRunner::GetCurrent());
                }
#endif

                template<typename ContinueCallback, typename ContinueResultType_ = typename FunctionTraits<ContinueCallback>::ReturnType>
                Task<ContinueResultType_> __YYAPI Then(_In_ TaskRunner* _pResumeTaskRunner, _In_ ContinueCallback&& pfnTaskCallback)
                {
                    using ContinueCallbackType = typename std::decay<ContinueCallback>::type;
                    using TaskContinueAsyncOperationType = TaskContinueAsyncOperation<ResultType, ContinueResultType_, ContinueCallbackType>;

                    auto _pTaskContinueAsyncOperation = YY::RefPtr<TaskContinueAsyncOperationType>::Create(std::forward<ContinueCallback>(pfnTaskCallback));
                    _pTaskContinueAsyncOperation->pResumeTaskRunnerWeak = _pResumeTaskRunner;
                    _pTaskContinueAsyncOperation.Get()->AddRef();
                    if (!pAsyncOperation->AddCompletedHandler(_pTaskContinueAsyncOperation))
                    {
                        _pTaskContinueAsyncOperation->OnCompleted(pAsyncOperation, pAsyncOperation->GetStatus());
                    }
                    return Task<ContinueResultType_>(_pTaskContinueAsyncOperation);
                }

                template<typename ContinueCallback, typename ContinueResultType_ = typename FunctionTraits<ContinueCallback>::ReturnType>
                Task<ContinueResultType_> __YYAPI Then(_In_ ContinueCallback&& pfnTaskCallback)
                {
                    return Then(TaskRunner::GetCurrent(), std::forward<ContinueCallback>(pfnTaskCallback));
                }
            };

            /// <summary>
            /// 创建一个任务，该任务将在所有提供的任务完成时完成。
            /// </summary>
            /// <param name="..._Tasks">需要等待的任务</param>
            /// <returns>表示所有任务的完成结果。如果所有任务都成功完成，则结果为 S_OK；如果至少有一个任务失败，则结果为第一个失败任务的错误代码。</returns>
            template<typename... Tasks>
            static Task<HRESULT> __YYAPI WhenAll(const Tasks&... _Tasks)
            {
                constexpr static size_t N = sizeof...(Tasks);
                static_assert(N >= 1, "WaitAny requires at least one Task.");

                class WhenAllTaskAsyncOperation
                    : public AsyncOperationImpl<HRESULT>
                    , public AsyncOperationCompletedHandler<void>
                {
                public:
                    uint32_t cchPendingTasks = N;
                    HRESULT hrFirstError = S_OK;
                    YY::RefPtr<AsyncOperation<void>> pAsyncOperations[N];

                    void __YYAPI InstallCompletedHandler()
                    {
                        if (cchPendingTasks == 0)
                        {
                            return;
                        }

                        // OnCompleted时会减少引用计数，因此在此处需要增加引用计数以确保对象在回调期间不会被销毁。
                        AddRef();
                        auto _pAsyncOperation = pAsyncOperations[N - cchPendingTasks];
                        if (!_pAsyncOperation)
                        {
                            OnCompleted(nullptr, AsyncStatus::Completed);
                            return;
                        }

                        if (!_pAsyncOperation->AddCompletedHandler(this))
                        {
                            OnCompleted(_pAsyncOperation, _pAsyncOperation->GetStatus());
                            return;
                        }
                    }

                    void __YYAPI OnCompleted(AsyncOperation<void>* _pAsyncInfo, AsyncStatus _eStatus) override
                    {
                        // 对应 InstallCompletedHandler 中的 AddRef。
                        auto _pThis = YY::RefPtr<WhenAllTaskAsyncOperation>::FromPtr(this);
                        if (_eStatus != AsyncStatus::Completed)
                        {
                            auto _hr = _eStatus == AsyncStatus::Canceled ? __HRESULT_FROM_WIN32(ERROR_CANCELLED) : _pAsyncInfo->GetErrorCode();
                            YY::Sync::CompareExchange(&hrFirstError, _hr, S_OK);
                        }

                        --cchPendingTasks;
                        if (cchPendingTasks == 0)
                        {
                            this->Resolve((HRESULT)hrFirstError);
                        }
                        else
                        {
                            InstallCompletedHandler();
                        }
                    }
                };

                auto _pWhenAllTaskAsyncOperation = YY::RefPtr<WhenAllTaskAsyncOperation>::Create();

                size_t idx = 0;
                int _unusedWhenAll[] = { 0, ((void)(_pWhenAllTaskAsyncOperation->pAsyncOperations[idx++] = (AsyncOperation<void>*)_Tasks.GetAsyncOperation().Get()), 0)... };
                (void)_unusedWhenAll;

                _pWhenAllTaskAsyncOperation->InstallCompletedHandler();
                return Task<HRESULT>(std::move(_pWhenAllTaskAsyncOperation));
            }

            /// <summary>
            /// 创建一个任务，该任务将在任何提供的任务完成时完成。
            /// </summary>
            /// <typeparam name="...Tasks"></typeparam>
            /// <param name="..._Tasks"></param>
            /// <returns>返回一个 Task&lt;int32_t&gt;，表示第一个完成的任务的索引。如果所有任务都失败，则结果为第一个失败任务的索引。</returns>
            template<typename... Tasks>
            static Task<int32_t> __YYAPI WhenAny(const Tasks&... _Tasks)
            {
                constexpr size_t N = sizeof...(Tasks);
                static_assert(N >= 1, "WaitAny requires at least one Task.");

                class WhenAnyTaskAsyncOperationCompletedHandler : public YY::AsyncOperationCompletedHandler<void>
                {
                public:
                    YY::RefPtr<AsyncOperationImpl<int32_t>> pWhenAnyTaskAsyncOperation;
                    int32_t iIndex = -1;

                    void __YYAPI OnCompleted(AsyncOperation<void>* _pAsyncInfo, AsyncStatus _eStatus) override
                    {
                        YY::RefPtr<AsyncOperationImpl<int32_t>> _pWhenAnyTaskAsyncOperation(std::move(pWhenAnyTaskAsyncOperation));      
                        _pWhenAnyTaskAsyncOperation->Resolve(std::move(iIndex));
                    }
                };

                class WhenAnyTaskAsyncOperation
                    : public AsyncOperationImpl<int32_t>
                {
                public:
                    YY::RefPtr<AsyncOperation<void>> pAsyncOperations[N];
                    WhenAnyTaskAsyncOperationCompletedHandler oAsyncOperationCompletedHandlers[N];

                    void __YYAPI InstallCompletedHandler()
                    {
                        size_t _iIndex = 0;
                        for (; _iIndex != N; ++_iIndex)
                        {
                            auto _pAsyncOperation = pAsyncOperations[_iIndex];
                            if (!_pAsyncOperation)
                            {
                                Resolve((int32_t)_iIndex);
                                break;
                            }

                            auto _pHandler = &oAsyncOperationCompletedHandlers[_iIndex];
                            _pHandler->pWhenAnyTaskAsyncOperation = this;
                            _pHandler->iIndex = (int32_t)_iIndex;
                            if (!_pAsyncOperation->AddCompletedHandler(_pHandler))
                            {
                                Resolve((int32_t)_iIndex);
                                break;
                            }
                        }

                        if (_iIndex == N)
                        {
                            return;
                        }

                        // 已经存在成功的任务，开始回滚 CompletedHandler
                        for (; _iIndex;--_iIndex)
                        {
                            auto _pAsyncOperation = pAsyncOperations[_iIndex];
                            if (!_pAsyncOperation)
                            {
                                continue;
                            }

                            auto _pHandler = &oAsyncOperationCompletedHandlers[_iIndex];
                            if (_pAsyncOperation->RemoveCompletedHandler(_pHandler))
                            {
                                _pHandler->pWhenAnyTaskAsyncOperation.Reset();
                            }
                        }
                    }
                };

                auto _pWhenAnyTaskAsyncOperation = YY::RefPtr<WhenAnyTaskAsyncOperation>::Create();
                
                size_t idx = 0;
                int _unusedWhenAny[] = { 0, ((void)(_pWhenAnyTaskAsyncOperation->pAsyncOperations[idx++] = (AsyncOperation<void>*)_Tasks.GetAsyncOperation().Get()), 0)... };
                (void)_unusedWhenAny;

                _pWhenAnyTaskAsyncOperation->InstallCompletedHandler();
                return Task<int32_t>(std::move(_pWhenAnyTaskAsyncOperation));
            }

            template<typename CallbackType_, typename ResultType_ = typename FunctionTraits<CallbackType_>::ReturnType>
            class TaskAsyncOperation : public AsyncOperationImpl<ResultType_>
            {
            public:
                using ResultType = ResultType_;
                using CallbackType = CallbackType_;

                WeakPtr<TaskRunner> pResumeTaskRunnerWeak;
                CallbackType pfnTaskCallback;

                TaskAsyncOperation(CallbackType&& _pfnTaskCallback)
                    : pfnTaskCallback(std::move(_pfnTaskCallback))
                {
                }

                template<typename... _Parameters>
                bool __YYAPI Resume(_Parameters... _oParameters)
                {
                    try
                    {
                        if (!this->Resolve(pfnTaskCallback(std::forward<_Parameters>(_oParameters)...)))
                        {
                            this->SetErrorCode(E_FAIL);
                            return false;
                        }
                    }
                    catch (const YY::Exception& _oEx)
                    {
                        this->SetErrorCode(_oEx.GetErrorCode());
                        return false;
                    }
                    return true;
                }
            };

            template<typename CallbackType_>
            class TaskAsyncOperation<CallbackType_, void > : public AsyncOperationImpl<void>
            {
            public:
                using ResultType = void;
                using CallbackType = CallbackType_;

                WeakPtr<TaskRunner> pResumeTaskRunnerWeak;
                CallbackType pfnTaskCallback;

                TaskAsyncOperation(CallbackType&& _pfnTaskCallback)
                    : pfnTaskCallback(std::move(_pfnTaskCallback))
                {
                }

                template<typename... _Parameters>
                bool __YYAPI Resume(_Parameters... _oParameters)
                {
                    try
                    {
                        pfnTaskCallback(std::forward<_Parameters>(_oParameters)...);
                        if (!this->Resolve())
                        {
                            this->SetErrorCode(E_FAIL);
                            return false;
                        }
                    }
                    catch (const YY::Exception& _oEx)
                    {
                        this->SetErrorCode(_oEx.GetErrorCode());
                        return false;
                    }
                    return true;
                }
            };

            template<typename SourceResultType_, typename ResultType_, typename CallbackType_>
            class TaskContinueAsyncOperation
                : public TaskAsyncOperation<CallbackType_, ResultType_>
                , public AsyncOperationCompletedHandler<SourceResultType_>
            {
            public:
                using SourceResultType = SourceResultType_;
                using ResultType = ResultType_;
                using CallbackType = CallbackType_;

                TaskContinueAsyncOperation(CallbackType&& _pfnTaskCallback)
                    : TaskAsyncOperation<CallbackType_, ResultType_>(std::move(_pfnTaskCallback))
                {
                }

                void __YYAPI OnCompleted(AsyncOperation<SourceResultType>* _pAsyncInfo, AsyncStatus _eStatus) override
                {
                    auto _pThis = YY::RefPtr<TaskContinueAsyncOperation>::FromPtr(this);
                    if (_eStatus != AsyncStatus::Completed)
                    {
                        this->SetErrorCode(_pAsyncInfo->GetErrorCode());
                        return;
                    }

                    auto _pResumeTaskRunner = this->pResumeTaskRunnerWeak.Get();
                    if (!_pResumeTaskRunner)
                    {
                        this->SetErrorCode(__HRESULT_FROM_WIN32(ERROR_CANCELLED));
                        return;
                    }

                    _pResumeTaskRunner->PostTask(
                        [_pThis, _pAsyncInfo = YY::RefPtr<AsyncOperation<SourceResultType>>(_pAsyncInfo)]()
                        {
                            _pThis->Resume(_pAsyncInfo->GetResult());
                        });
                }
            };

            template<typename ResultType_, typename CallbackType_>
            class TaskContinueAsyncOperation<void, ResultType_, CallbackType_>
                : public TaskAsyncOperation<CallbackType_, ResultType_>
                , public AsyncOperationCompletedHandler<void>
            {
            public:
                using SourceResultType = void;
                using ResultType = ResultType_;
                using CallbackType = CallbackType_;

                TaskContinueAsyncOperation(CallbackType&& _pfnTaskCallback)
                    : TaskAsyncOperation<CallbackType_, ResultType_>(std::move(_pfnTaskCallback))
                {
                }

                void __YYAPI OnCompleted(AsyncOperation<SourceResultType>* _pAsyncInfo, AsyncStatus _eStatus) override
                {
                    auto _pThis = YY::RefPtr<TaskContinueAsyncOperation>::FromPtr(this);
                    if (_eStatus != AsyncStatus::Completed)
                    {
                        this->SetErrorCode(_pAsyncInfo->GetErrorCode());
                        return;
                    }

                    auto _pResumeTaskRunner = this->pResumeTaskRunnerWeak.Get();
                    if (!_pResumeTaskRunner)
                    {
                        this->SetErrorCode(__HRESULT_FROM_WIN32(ERROR_CANCELLED));
                        return;
                    }

                    _pResumeTaskRunner->PostTask(
                        [_pThis]()
                        {
                            _pThis->Resume();
                        });
                }
            };

#if defined(_HAS_CXX20) && _HAS_CXX20
            template<typename ReturnType_>
            struct Promise
            {
                using ReturnType = ReturnType_;
                YY::RefPtr<AsyncOperationImpl<ReturnType>> pAsyncOperation = YY::RefPtr<AsyncOperationImpl<ReturnType>>::Create();

                Task<ReturnType> get_return_object() noexcept
                {
                    return Task<ReturnType>(pAsyncOperation);
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
                    pAsyncOperation->Resolve(std::move(_oValue));
                }

                void unhandled_exception()
                {
                    try
                    {
                        throw;
                    }
                    catch (const YY::Exception& _oEx)
                    {
                        pAsyncOperation->SetErrorCode(_oEx.GetErrorCode());
                    }
                    catch (...)
                    {
                        pAsyncOperation->SetErrorCode(E_FAIL);
                    }
                }
            };

            template<>
            struct Promise<void>
            {
                using ReturnType = void;
                YY::RefPtr<AsyncOperationImpl<void>> pAsyncOperation = YY::RefPtr<AsyncOperationImpl<void>>::Create();

                Task<ReturnType> get_return_object() noexcept
                {
                    return Task<ReturnType>(pAsyncOperation);
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
                    if (pAsyncOperation)
                    {
                        pAsyncOperation->Resolve();
                    }
                }

                void unhandled_exception()
                {
                    if (!pAsyncOperation)
                        return;

                    try
                    {
                        throw;
                    }
                    catch (const YY::Exception& _oEx)
                    {
                        pAsyncOperation->SetErrorCode(_oEx.GetErrorCode());
                    }
                    catch (...)
                    {
                        pAsyncOperation->SetErrorCode(E_FAIL);
                    }
                }
            };

            template<typename ResultType_>
            class TaskAwaiter : protected AsyncOperationCompletedHandler<ResultType_>
            {
            private:
                YY::RefPtr<AsyncOperation<ResultType_>> pAsyncOperation;
                YY::WeakPtr<TaskRunner> pResumeTaskRunnerWeak;
                std::coroutine_handle<> hCoroutine;

            public:
                TaskAwaiter(YY::RefPtr<AsyncOperation<ResultType_>> _pAsyncOperation, TaskRunner* _pResumeTaskRunner) noexcept
                    : pAsyncOperation(std::move(_pAsyncOperation))
                    , pResumeTaskRunnerWeak(_pResumeTaskRunner)
                {
                }

                bool await_ready() const noexcept
                {
                    return pAsyncOperation->GetStatus() != AsyncStatus::Started;
                }

                bool await_suspend(std::coroutine_handle<> _hCoroutine) noexcept
                {
                    hCoroutine = _hCoroutine;
                    if (!pAsyncOperation->AddCompletedHandler(this))
                    {
                        return false;
                    }
                    return true;
                }

                ResultType_ await_resume()
                {
                    return pAsyncOperation->GetResult();
                }

                void __YYAPI OnCompleted(AsyncOperation<ResultType_>* _pAsyncInfo, AsyncStatus _eStatus) override
                {
                    UNREFERENCED_PARAMETER(_pAsyncInfo);
                    
                    if (_eStatus == AsyncStatus::Canceled)
                    {
                        hCoroutine.destroy();
                        return;
                    }

                    if (pResumeTaskRunnerWeak == nullptr)
                    {
                        if (hCoroutine)
                        {
                            hCoroutine.resume();
                        }
                    }
                    else if (auto _pResumeTaskRunner = pResumeTaskRunnerWeak.Get())
                    {
                        if (_pResumeTaskRunner == YY::TaskRunner::GetCurrent())
                        {
                            if (hCoroutine)
                            {
                                hCoroutine.resume();
                            }

                            return;
                        }

                        _pResumeTaskRunner->PostTask(
                            [hCoroutine = hCoroutine.address()]()
                            {
                                std::coroutine_handle<>::from_address(hCoroutine).resume();
                            });
                    }
                    else
                    {
                        hCoroutine.destroy();
                    }
                }
            };
#endif
        } // namespace YY::Base::Threading
    } // namespace YY::Base

    using namespace YY::Base::Threading;
} // namespace YY

#pragma pack(pop)
