#include <YY/Base/YY.h>
#include <YY/Base/Memory/RefPtr.h>
#include <YY/Base/Time/TimeSpan.h>
#include <YY/Base/Containers/DoublyLinkedList.h>
#include <YY/Base/Functional/Delegate.h>
#include <YY/Base/Sync/SRWLock.h>
#include <YY/Base/Sync/AutoLock.h>
#include <YY/Base/Containers/Optional.h>
#include <YY/Base/Threading/CancellationToken.h>

#include <exception>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            enum class AsyncStatus : uint32_t
            {
                // 操作已启动。
                Started,
                // 操作已完成。
                Completed,
                // 该操作已取消。
                Canceled,
                // 操作遇到错误。通过GetErrorCode可以获取详细的错误信息。
                Error,
            };

            class AsyncInfo : public YY::RefValue
            {
            protected:
                volatile HRESULT hr = E_PENDING;
                volatile AsyncStatus eStatus = AsyncStatus::Started;
                std::exception_ptr eptr;
                YY::RefPtr<CancellationToken> pCancellationToken;

            public:
                AsyncInfo(YY::RefPtr<CancellationToken> _pCancellationToken = nullptr)
                    : pCancellationToken(std::move(_pCancellationToken))
                {
                }

                AsyncInfo(const AsyncInfo&) = delete;
                AsyncInfo& operator=(const AsyncInfo&) = delete;

                /// <summary>
                /// 获取对象存储的错误代码。
                /// </summary>
                /// <returns>如果异步操作失败，则返回内部存储的错误代码（hr）；如果成功，则返回 S_OK。</returns>
                HRESULT __YYAPI GetErrorCode() const noexcept
                {
                    return hr;
                }

                std::exception_ptr __YYAPI GetExceptionPtr() const noexcept
                {
                    return eptr;
                }

                static std::exception_ptr __YYAPI CreateExceptionPtr(_In_opt_ AsyncInfo* _pAsyncInfo, AsyncStatus _eStatus)
                {
                    if (_eStatus == AsyncStatus::Completed)
                    {
                        return nullptr;
                    }

                    try
                    {
                        if (_pAsyncInfo)
                        {
                            auto _eptr = _pAsyncInfo->GetExceptionPtr();
                            if (_eptr)
                            {
                                return _eptr;
                            }

                            _pAsyncInfo->ThrowIfWaitTaskFailed();
                        }
                        else
                        {
                            if (_eStatus == AsyncStatus::Canceled)
                            {
                                throw YY::OperationCanceledException(_S("异步任务已经被取消。"));
                            }
                        }

                        throw YY::Exception(E_FAIL);
                    }
                    catch (...)
                    {
                        return std::current_exception();
                    }
                }

                AsyncStatus __YYAPI GetStatus() const noexcept
                {
                    return eStatus;
                }

                /// <summary>
                /// 取消异步操作。
                /// </summary>
                /// <returns></returns>
                virtual bool __YYAPI Cancel()
                {
                    if (!BeginNotifyCompletedHandlers(AsyncStatus::Canceled))
                    {
                        return false;
                    }

                    NotifyCompletedHandlers(__HRESULT_FROM_WIN32(ERROR_CANCELLED));
                    return true;
                }

                bool __YYAPI IsCanceled() const
                {
                    if (GetStatus() == AsyncStatus::Completed)
                    {
                        return false;
                    }

                    return (GetStatus() == AsyncStatus::Canceled) || (pCancellationToken && pCancellationToken->IsCancellationRequested());
                }

                YY::RefPtr<CancellationToken> __YYAPI GetCancellationToken() const
                {
                    return pCancellationToken;
                }

                /// <summary>
                /// 阻塞等待任务完成或直到达到指定超时时间（默认无限等待）。
                /// </summary>
                /// <param name="_oTimeout">最大等待时间，类型为 YY::TimeSpan。默认值为 YY::TimeSpan::GetMax()，表示无限期等待。</param>
                /// <returns>如果在指定超时时间内等待成功（任务完成等）则返回 true；如果超时则返回 false。</returns>
                bool __YYAPI WaitTask(YY::TimeSpan _oTimeout = YY::TimeSpan::GetMax())
                {
                    if (IsCanceled())
                        return false;

                    DWORD _uMilliseconds;
                    auto _iTimeoutMilliseconds = _oTimeout.GetTotalMilliseconds();
                    if (_iTimeoutMilliseconds <= 0)
                    {
                        _uMilliseconds = 0;
                    }
                    else if (_iTimeoutMilliseconds > UINT32_MAX)
                    {
                        _uMilliseconds = UINT32_MAX;
                    }
                    else
                    {
                        _uMilliseconds = (DWORD)_iTimeoutMilliseconds;
                    }

                    HRESULT _hrPending = E_PENDING;
                    return WaitOnAddress(&hr, &_hrPending, sizeof(_hrPending), _uMilliseconds);
                }

                void __YYAPI ThrowIfWaitTaskFailed()
                {
                    if (IsCanceled())
                    {
                        throw YY::OperationCanceledException(_S("异步任务已经被取消。"));
                    }

                    if (!WaitTask())
                    {
                        throw YY::Exception(_S("等待异步任务WaitTask失败。"), E_FAIL);
                    }

                    switch (this->GetStatus())
                    {
                    case AsyncStatus::Completed:
                        break;
                    case AsyncStatus::Canceled:
                        throw YY::OperationCanceledException(_S("异步任务已经被取消。"));
                        break;
                    case AsyncStatus::Error:
                    {
                        auto _eptr = this->GetExceptionPtr();
                        if (_eptr)
                        {
                            std::rethrow_exception(_eptr);
                        }
                        throw YY::Exception(this->GetErrorCode());
                        break;
                    }
                    default:
                        throw YY::Exception(_S("异步任务状态不符合预期。"), E_FAIL);
                        break;
                    }
                }

            protected:
                bool __YYAPI BeginNotifyCompletedHandlers(AsyncStatus _eStatus) noexcept
                {
                    if ((AsyncStatus)YY::Sync::CompareExchange(reinterpret_cast<volatile uint32_t*>(&eStatus), uint32_t(_eStatus), uint32_t(AsyncStatus::Started)) != AsyncStatus::Started)
                    {
                        return false;
                    }

                    return true;
                }

                bool __YYAPI SetErrorCode(HRESULT _hr)
                {
                    if ((AsyncStatus)YY::Sync::CompareExchange(reinterpret_cast<volatile uint32_t*>(&eStatus), uint32_t(AsyncStatus::Error), uint32_t(AsyncStatus::Started)) != AsyncStatus::Started)
                    {
                        return false;
                    }

                    if (_hr == E_PENDING || _hr == S_OK)
                    {
                        _hr = E_UNEXPECTED;
                    }

                    NotifyCompletedHandlers(_hr);
                    return true;
                }

                bool __YYAPI SetExceptionPtr(std::exception_ptr _eptr, HRESULT _hr)
                {
                    if ((AsyncStatus)YY::Sync::CompareExchange(reinterpret_cast<volatile uint32_t*>(&eStatus), uint32_t(AsyncStatus::Error), uint32_t(AsyncStatus::Started)) != AsyncStatus::Started)
                    {
                        return false;
                    }

                    eptr = std::move(_eptr);

                    if (_hr == E_PENDING || _hr == S_OK)
                    {
                        _hr = E_UNEXPECTED;
                    }

                    NotifyCompletedHandlers(_hr);
                    return true;
                }

                virtual void __YYAPI NotifyCompletedHandlers(HRESULT _hr) = 0;
            };

            template<typename ResultType_>
            class AsyncOperation;

            template<typename ResultType_>
            class AsyncOperationCompletedHandler : public DoublyLinkedListEntryImpl<AsyncOperationCompletedHandler<ResultType_>>
            {
            public:
                using ResultType = ResultType_;

                virtual void __YYAPI OnCompleted(AsyncOperation<ResultType>* _pAsyncInfo, AsyncStatus _eStatus) = 0;
            };

            template<typename ResultType_>
            class AsyncOperation : public AsyncInfo
            {
            public:
                using ResultType = ResultType_;
                using AsyncOperationCompletedHandler = AsyncOperationCompletedHandler<ResultType>;
                using Delegate = Delegate<AsyncOperationCompletedHandler>;

            protected:
                YY::SRWLock oSRWLock;
                Delegate oCompletedDelegate;

            public:
                AsyncOperation(YY::RefPtr<CancellationToken> _pCancellationToken = nullptr)
                    : AsyncInfo(std::move(_pCancellationToken))
                {
                }

                /// <summary>
                /// 如果操作成功，则获取异步操作的结果。
                /// </summary>
                /// <returns>指向 ResultType 的引用，表示获取到的结果。</returns>
                virtual ResultType& __YYAPI GetResult() = 0;

                bool __YYAPI AddCompletedHandler(_In_ AsyncOperationCompletedHandler* _pHandler)
                {
                    if (!_pHandler)
                    {
                        return false;
                    }
                    
                    YY::AutoLock<YY::SRWLock> _oAutoPushBackLock(oSRWLock);

                    if (GetStatus() != AsyncStatus::Started)
                    {
                        return false;
                    }

                    return oCompletedDelegate.AddHandler(_pHandler);
                }

                bool __YYAPI RemoveCompletedHandler(_In_ AsyncOperationCompletedHandler* _pHandler)
                {
                    if (!_pHandler)
                        return false;

                    YY::AutoLock<YY::SRWLock> _oAutoRemoveLock(oSRWLock);
                    return oCompletedDelegate.RemoveHandler(_pHandler);
                }

            protected:
                void __YYAPI NotifyCompletedHandlers(HRESULT _hr) override
                {
                    hr = _hr;
                    WakeByAddressAll((PVOID)&hr);
                    Delegate _oCompletedDelegate;
                    {
                        YY::AutoLock<YY::SRWLock> _oAutoSwapLock(oSRWLock);
                        _oCompletedDelegate += std::move(oCompletedDelegate);
                    }

                    _oCompletedDelegate.Invoke(
                        [this](AsyncOperationCompletedHandler* _pHandler)
                        {
                            _pHandler->OnCompleted(this, GetStatus());
                        });
                }
            };

            template<>
            class AsyncOperation<void> : public AsyncInfo
            {
            public:
                using ResultType = void;
                using AsyncOperationCompletedHandler = AsyncOperationCompletedHandler<ResultType>;
                using Delegate = Delegate<AsyncOperationCompletedHandler>;

            protected:
                YY::SRWLock oSRWLock;
                Delegate oCompletedDelegate;

            public:
                AsyncOperation(YY::RefPtr<CancellationToken> _pCancellationToken = nullptr)
                    : AsyncInfo(std::move(_pCancellationToken))
                {
                }

                /// <summary>
                /// 如果操作成功，则获取异步操作的结果。
                /// </summary>
                /// <returns>指向 ResultType 的引用，表示获取到的结果。</returns>
                virtual void __YYAPI GetResult() = 0;

                bool __YYAPI AddCompletedHandler(_In_ AsyncOperationCompletedHandler* _pHandler)
                {
                    if (!_pHandler)
                    {
                        return false;
                    }

                    YY::AutoLock<YY::SRWLock> _oAutoPushBackLock(oSRWLock);

                    if (GetStatus() != AsyncStatus::Started)
                    {
                        return false;
                    }

                    return oCompletedDelegate.AddHandler(_pHandler);
                }

                bool __YYAPI RemoveCompletedHandler(_In_ AsyncOperationCompletedHandler* _pHandler)
                {
                    if (!_pHandler)
                        return false;

                    YY::AutoLock<YY::SRWLock> _oAutoRemoveLock(oSRWLock);
                    return oCompletedDelegate.RemoveHandler(_pHandler);
                }

            protected:
                void __YYAPI NotifyCompletedHandlers(HRESULT _hr) override
                {
                    hr = _hr;
                    WakeByAddressAll((PVOID)&hr);
                    Delegate _oCompletedDelegate;
                    {
                        YY::AutoLock<YY::SRWLock> _oAutoSwapLock(oSRWLock);
                        _oCompletedDelegate += std::move(oCompletedDelegate);
                    }

                    _oCompletedDelegate.Invoke(
                        [this](AsyncOperationCompletedHandler* _pHandler)
                        {
                            _pHandler->OnCompleted(this, GetStatus());
                        });
                }
            };

            template<typename ResultType_>
            class AsyncOperationImpl : public AsyncOperation<ResultType_>
            {
            public:
                using ResultType = ResultType_;
                using AsyncOperationCompletedHandler = typename AsyncOperation<ResultType>::AsyncOperationCompletedHandler;
                using Delegate = typename AsyncOperation<ResultType>::Delegate;

            protected:
                union
                {
                    char oResultBuffer[sizeof(ResultType)] = {};
                    ResultType oResult;
                };

            public:
                AsyncOperationImpl(YY::RefPtr<CancellationToken> _pCancellationToken = nullptr)
                    : AsyncOperation<ResultType_>(std::move(_pCancellationToken))
                {
                }

                ~AsyncOperationImpl()
                {
                    if (AsyncOperation<ResultType>::GetStatus() == AsyncStatus::Completed)
                    {
                        oResult.~ResultType();
                    }
                }

                ResultType& __YYAPI GetResult() override
                {
                    AsyncOperation<ResultType>::ThrowIfWaitTaskFailed();
                    return oResult;
                }

                bool __YYAPI Resolve(_In_ AsyncOperation<ResultType_>* _pAsyncOperation)
                {
                    return Resolve(_pAsyncOperation->GetResult());
                }

                bool __YYAPI Resolve(const ResultType& _oResult)
                {
                    if (AsyncOperation<ResultType>::IsCanceled())
                    {
                        AsyncOperation<ResultType>::Cancel();
                        return false;
                    }

                    if (!AsyncOperation<ResultType>::BeginNotifyCompletedHandlers(AsyncStatus::Completed))
                    {
                        return false;
                    }

                    new (oResultBuffer) ResultType(_oResult);

                    AsyncOperation<ResultType>::NotifyCompletedHandlers(S_OK);
                    return true;
                }

                bool __YYAPI Resolve(ResultType&& _oResult)
                {
                    if (AsyncOperation<ResultType>::IsCanceled())
                    {
                        AsyncOperation<ResultType>::Cancel();
                        return false;
                    }

                    if (!AsyncOperation<ResultType>::BeginNotifyCompletedHandlers(AsyncStatus::Completed))
                    {
                        return false;
                    }

                    new (oResultBuffer) ResultType(std::move(_oResult));
                    AsyncOperation<ResultType>::NotifyCompletedHandlers(S_OK);
                    return true;
                }

                bool __YYAPI SetErrorCode(HRESULT _hr)
                {
                    return AsyncOperation<ResultType>::SetErrorCode(_hr);
                }

                bool __YYAPI SetExceptionPtr(std::exception_ptr _eptr, HRESULT _hr)
                {
                    return AsyncOperation<ResultType>::SetExceptionPtr(std::move(_eptr), _hr);
                }
            };

            template<>
            class AsyncOperationImpl<void> : public AsyncOperation<void>
            {
            public:
                using ResultType = void;
                using AsyncOperationCompletedHandler = typename AsyncOperation<ResultType>::AsyncOperationCompletedHandler;
                using Delegate = typename AsyncOperation<ResultType>::Delegate;

            private:

            public:
                AsyncOperationImpl(YY::RefPtr<CancellationToken> _pCancellationToken = nullptr)
                    : AsyncOperation(std::move(_pCancellationToken))
                {
                }

                void __YYAPI GetResult() override
                {
                    AsyncOperation<ResultType>::ThrowIfWaitTaskFailed();
                    return;
                }

                bool __YYAPI Resolve(_In_ AsyncOperation<void>* _pAsyncOperation)
                {
                    if (AsyncOperation<ResultType>::IsCanceled() || _pAsyncOperation->IsCanceled())
                    {
                        AsyncOperation<ResultType>::Cancel();
                        return false;
                    }

                    _pAsyncOperation->GetResult();
                    return Resolve();
                }

                bool __YYAPI Resolve(void)
                {
                    if (AsyncOperation<ResultType>::IsCanceled())
                    {
                        AsyncOperation<ResultType>::Cancel();
                        return false;
                    }

                    if (!AsyncOperation<ResultType>::BeginNotifyCompletedHandlers(AsyncStatus::Completed))
                    {
                        return false;
                    }

                    AsyncOperation<ResultType>::NotifyCompletedHandlers(S_OK);
                    return true;
                }

                bool __YYAPI SetErrorCode(HRESULT _hr)
                {
                    return AsyncOperation<ResultType>::SetErrorCode(_hr);
                }

                bool __YYAPI SetExceptionPtr(std::exception_ptr _eptr, HRESULT _hr)
                {
                    return AsyncOperation<ResultType>::SetExceptionPtr(std::move(_eptr), _hr);
                }
            };

            template<typename ResultType_>
            class IoAsyncOperation
                : public AsyncOperation<ResultType_>
                , public OVERLAPPED
            {
            public:
                using ResultType = ResultType_;

                LSTATUS lStatus = ERROR_IO_PENDING;

                IoAsyncOperation(YY::RefPtr<CancellationToken> _pCancellationToken = nullptr)
                    : AsyncOperation<ResultType_>(std::move(_pCancellationToken))
                    , OVERLAPPED{}
                {
                }

                bool __YYAPI Resolve(LSTATUS _lStatus)
                {
                    if (AsyncOperation<ResultType>::IsCanceled())
                    {
                        AsyncOperation<ResultType>::Cancel();
                        return false;
                    }

                    if (!AsyncOperation<ResultType>::BeginNotifyCompletedHandlers(AsyncStatus::Completed))
                    {
                        return false;
                    }

                    lStatus = _lStatus;
                    AsyncOperation<ResultType>::NotifyCompletedHandlers(S_OK);
                    return true;
                }

                bool __YYAPI SetErrorCode(HRESULT _hr)
                {
                    return AsyncOperation<ResultType>::SetErrorCode(_hr);
                }
            };
        }
    }

    using namespace YY::Base::Threading;
}

#pragma pack(pop)
