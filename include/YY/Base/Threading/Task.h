#pragma once
#include <functional>
#include <type_traits>
#include <utility>
#include <YY/Base/Threading/Async.h>
#include <YY/Base/Memory/WeakPtr.h>
#include <YY/Base/Functional/FunctionTraits.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            class TaskRunner;

            namespace Detail
            {
                template<typename SourceResultType_, typename ContinueResultType_, typename ContinueCallbackType_, bool bSourceIsVoid = std::is_void<SourceResultType_>::value, bool bContinueIsVoid = std::is_void<ContinueResultType_>::value>
                class ContinueHandler;

                template<typename SourceResultType_, typename ContinueResultType_, typename ContinueCallbackType_>
                class ContinueHandler<SourceResultType_, ContinueResultType_, ContinueCallbackType_, false, false>
                    : public AsyncOperationImpl<ContinueResultType_>
                    , public AsyncOperationCompletedHandler<SourceResultType_>
                {
                public:
                    using SourceResultType = SourceResultType_;
                    using ContinueResultType = ContinueResultType_;
                    using ContinueCallbackType = ContinueCallbackType_;

                    WeakPtr<TaskRunner> pResumeTaskRunnerWeak;
                    ContinueCallbackType pfnTaskCallback;

                    ContinueHandler(ContinueCallbackType&& _pfnTaskCallback)
                        : pfnTaskCallback(std::move(_pfnTaskCallback))
                    {
                    }

                    void __YYAPI OnCompleted(AsyncOperation<SourceResultType>* _pAsyncInfo, AsyncStatus _eStatus) override
                    {
                        if (_eStatus != AsyncStatus::Completed)
                        {
                            this->SetErrorCode(_pAsyncInfo->GetErrorCode());
                            return;
                        }

                        try
                        {
                            auto& _oResult = _pAsyncInfo->GetResult();
                            if (!this->Resolve(pfnTaskCallback(_oResult)))
                            {
                                this->SetErrorCode(E_FAIL);
                            }
                        }
                        catch (const YY::Exception& _oEx)
                        {
                            this->SetErrorCode(_oEx.GetErrorCode());
                        }
                    }
                };

                template<typename SourceResultType_, typename ContinueResultType_, typename ContinueCallbackType_>
                class ContinueHandler<SourceResultType_, ContinueResultType_, ContinueCallbackType_, false, true>
                    : public AsyncOperationImpl<void>
                    , public AsyncOperationCompletedHandler<SourceResultType_>
                {
                public:
                    using SourceResultType = SourceResultType_;
                    using ContinueResultType = ContinueResultType_;
                    using ContinueCallbackType = ContinueCallbackType_;

                    WeakPtr<TaskRunner> pResumeTaskRunnerWeak;
                    ContinueCallbackType pfnTaskCallback;

                    ContinueHandler(ContinueCallbackType&& _pfnTaskCallback)
                        : pfnTaskCallback(std::move(_pfnTaskCallback))
                    {
                    }

                    void __YYAPI OnCompleted(AsyncOperation<SourceResultType>* _pAsyncInfo, AsyncStatus _eStatus) override
                    {
                        if (_eStatus != AsyncStatus::Completed)
                        {
                            this->SetErrorCode(_pAsyncInfo->GetErrorCode());
                            return;
                        }

                        try
                        {
                            auto& _oResult = _pAsyncInfo->GetResult();
                            pfnTaskCallback(_oResult);
                            if (!this->Resolve())
                            {
                                this->SetErrorCode(E_FAIL);
                            }
                        }
                        catch (const YY::Exception& _oEx)
                        {
                            this->SetErrorCode(_oEx.GetErrorCode());
                        }
                    }
                };

                template<typename SourceResultType_, typename ContinueResultType_, typename ContinueCallbackType_>
                class ContinueHandler<SourceResultType_, ContinueResultType_, ContinueCallbackType_, true, false>
                    : public AsyncOperationImpl<ContinueResultType_>
                    , public AsyncOperationCompletedHandler<void>
                {
                public:
                    using SourceResultType = SourceResultType_;
                    using ContinueResultType = ContinueResultType_;
                    using ContinueCallbackType = ContinueCallbackType_;

                    WeakPtr<TaskRunner> pResumeTaskRunnerWeak;
                    ContinueCallbackType pfnTaskCallback;

                    ContinueHandler(ContinueCallbackType&& _pfnTaskCallback)
                        : pfnTaskCallback(std::move(_pfnTaskCallback))
                    {
                    }

                    void __YYAPI OnCompleted(AsyncOperation<void>* _pAsyncInfo, AsyncStatus _eStatus) override
                    {
                        if (_eStatus != AsyncStatus::Completed)
                        {
                            this->SetErrorCode(_pAsyncInfo->GetErrorCode());
                            return;
                        }

                        try
                        {
                            _pAsyncInfo->GetResult();
                            if (!this->Resolve(pfnTaskCallback()))
                            {
                                this->SetErrorCode(E_FAIL);
                            }
                        }
                        catch (const YY::Exception& _oEx)
                        {
                            this->SetErrorCode(_oEx.GetErrorCode());
                        }
                    }
                };

                template<typename SourceResultType_, typename ContinueResultType_, typename ContinueCallbackType_>
                class ContinueHandler<SourceResultType_, ContinueResultType_, ContinueCallbackType_, true, true>
                    : public AsyncOperationImpl<void>
                    , public AsyncOperationCompletedHandler<void>
                {
                public:
                    using SourceResultType = SourceResultType_;
                    using ContinueResultType = ContinueResultType_;
                    using ContinueCallbackType = ContinueCallbackType_;

                    WeakPtr<TaskRunner> pResumeTaskRunnerWeak;
                    ContinueCallbackType pfnTaskCallback;

                    ContinueHandler(ContinueCallbackType&& _pfnTaskCallback)
                        : pfnTaskCallback(std::move(_pfnTaskCallback))
                    {
                    }

                    void __YYAPI OnCompleted(AsyncOperation<void>* _pAsyncInfo, AsyncStatus _eStatus) override
                    {
                        if (_eStatus != AsyncStatus::Completed)
                        {
                            this->SetErrorCode(_pAsyncInfo->GetErrorCode());
                            return;
                        }

                        try
                        {
                            _pAsyncInfo->GetResult();
                            pfnTaskCallback();
                            if (!this->Resolve())
                            {
                                this->SetErrorCode(E_FAIL);
                            }
                        }
                        catch (const YY::Exception& _oEx)
                        {
                            this->SetErrorCode(_oEx.GetErrorCode());
                        }
                    }
                };
            } // namespace Detail

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

                template<typename ContinueCallback, typename ContinueResultType_ = typename FunctionTraits<ContinueCallback>::ReturnType>
                Task<ContinueResultType_> __YYAPI Then(_In_ TaskRunner* _pResumeTaskRunner, _In_ ContinueCallback&& pfnTaskCallback)
                {
                    using ContinueCallbackType = typename std::decay<ContinueCallback>::type;
                    using ContinueHandlerType = Detail::ContinueHandler<ResultType, ContinueResultType_, ContinueCallbackType>;

                    auto _pContinueHandler = YY::RefPtr<ContinueHandlerType>::Create(std::forward<ContinueCallback>(pfnTaskCallback));
                    _pContinueHandler->pResumeTaskRunnerWeak = _pResumeTaskRunner;
                    pAsyncOperation->AddCompletedHandler(_pContinueHandler);
                    return Task<ContinueResultType_>(_pContinueHandler);
                }
            };

            template<>
            class Task<void>
            {
            public:
                using ResultType = void;

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

                template<typename ContinueCallback, typename ContinueResultType_ = typename FunctionTraits<ContinueCallback>::ReturnType>
                Task<ContinueResultType_> __YYAPI Then(_In_ TaskRunner* _pResumeTaskRunner, _In_ ContinueCallback&& pfnTaskCallback)
                {
                    using ContinueCallbackType = typename std::decay<ContinueCallback>::type;
                    using ContinueHandlerType = Detail::ContinueHandler<void, ContinueResultType_, ContinueCallbackType>;

                    auto _pContinueHandler = YY::RefPtr<ContinueHandlerType>::Create(std::forward<ContinueCallback>(pfnTaskCallback));
                    _pContinueHandler->pResumeTaskRunnerWeak = _pResumeTaskRunner;
                    pAsyncOperation->AddCompletedHandler(_pContinueHandler);
                    return Task<ContinueResultType_>(_pContinueHandler);
                }
            };

        } // namespace YY::Base::Threading
    } // namespace YY::Base

    using namespace YY::Base::Threading;
} // namespace YY
