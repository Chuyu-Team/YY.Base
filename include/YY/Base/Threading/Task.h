#pragma once
#include <type_traits>
#include <utility>
#include <YY/Base/Threading/Async.h>
#include <YY/Base/Memory/WeakPtr.h>
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
            class TaskCoAwaiter
                : public YY::RefValue
                , public AsyncOperationCompletedHandler<ResultType_>
            {
            private:
                YY::RefPtr<AsyncOperation<ResultType_>> pAsyncOperation;
                std::coroutine_handle<> hCoroutine;

            public:
                TaskCoAwaiter(YY::RefPtr<AsyncOperation<ResultType_>> _pAsyncOperation) noexcept
                    : pAsyncOperation(std::move(_pAsyncOperation))
                {
                }

                bool await_ready() const noexcept
                {
                    return pAsyncOperation->GetStatus() != AsyncStatus::Started;
                }

                bool await_suspend(std::coroutine_handle<> _hCoroutine) noexcept
                {
                    hCoroutine = _hCoroutine;
                    AddRef();
                    if (!pAsyncOperation->AddCompletedHandler(this))
                    {
                        Release();
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
                    UNREFERENCED_PARAMETER(_eStatus);

                    auto _pThis = YY::RefPtr<TaskCoAwaiter<ResultType_>>::FromPtr(this);
                    if (hCoroutine)
                    {
                        hCoroutine.resume();
                    }
                }
            };

            template<>
            class TaskCoAwaiter<void>
                : public YY::RefValue
                , public AsyncOperationCompletedHandler<void>
            {
            private:
                YY::RefPtr<AsyncOperation<void>> pAsyncOperation;
                std::coroutine_handle<> hCoroutine;

            public:
                TaskCoAwaiter(YY::RefPtr<AsyncOperation<void>> _pAsyncOperation) noexcept
                    : pAsyncOperation(std::move(_pAsyncOperation))
                {
                }

                bool await_ready() const noexcept
                {
                    return pAsyncOperation->GetStatus() != AsyncStatus::Started;
                }

                bool await_suspend(std::coroutine_handle<> _hCoroutine) noexcept
                {
                    hCoroutine = _hCoroutine;
                    AddRef();
                    if (!pAsyncOperation->AddCompletedHandler(this))
                    {
                        Release();
                        return false;
                    }
                    return true;
                }

                void await_resume()
                {
                    pAsyncOperation->GetResult();
                }

                void __YYAPI OnCompleted(AsyncOperation<void>* _pAsyncInfo, AsyncStatus _eStatus) override
                {
                    UNREFERENCED_PARAMETER(_pAsyncInfo);
                    UNREFERENCED_PARAMETER(_eStatus);

                    auto _pThis = YY::RefPtr<TaskCoAwaiter<void>>::FromPtr(this);
                    if (hCoroutine)
                    {
                        hCoroutine.resume();
                    }
                }
            };
#endif

            template<typename ResultType_>
            class Task
            {
            public:
                using ResultType = ResultType_;

            private:
                YY::RefPtr<AsyncOperation<ResultType>> pAsyncOperation;

            public:
                Task(YY::RefPtr<AsyncOperation<ResultType>> _pAsyncOperation) noexcept
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
                TaskCoAwaiter<ResultType> __YYAPI operator co_await() const
                {
                    class TaskCoAwaiter: protected AsyncOperationCompletedHandler<ResultType>
                    {
                    private:
                        YY::RefPtr<AsyncOperation<ResultType>> pAsyncOperation;
                        YY::WeakPtr<TaskRunner> pResumeTaskRunnerWeak;
                        std::coroutine_handle<> hCoroutine;

                    public:
                        TaskCoAwaiter(YY::RefPtr<AsyncOperation<ResultType>> _pAsyncOperation) noexcept
                            : pAsyncOperation(std::move(_pAsyncOperation))
                            , pResumeTaskRunnerWeak(YY::TaskRunner::GetCurrent())
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
                            UNREFERENCED_PARAMETER(_eStatus);
                            if (pResumeTaskRunnerWeak == nullptr)
                            {
                                if (hCoroutine)
                                {
                                    hCoroutine.resume();
                                }
                            }
                        }
                    };

                    return TaskCoAwaiter<ResultType>(pAsyncOperation);
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
                    return Then(YY::TaskRunner::GetCurrent(), std::forward<ContinueCallback>(pfnTaskCallback));
                }
            };

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
        } // namespace YY::Base::Threading
    } // namespace YY::Base

    using namespace YY::Base::Threading;
} // namespace YY

#pragma pack(pop)
