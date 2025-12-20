#include <YY/Base/YY.h>
#include <YY/Base/Memory/RefPtr.h>
#include <YY/Base/Time/TimeSpan.h>
#include <YY/Base/Containers/DoublyLinkedList.h>
#include <YY/Base/Sync/SRWLock.h>
#include <YY/Base/Sync/AutoLock.h>

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

            template<typename DelegateHandle>
            class Delegate
            {
            protected:
                YY::DoublyLinkedList<DelegateHandle> oDelegateHandleList;

            public:
                constexpr Delegate() = default;

                constexpr Delegate(Delegate&& _oOther) noexcept
                    : oDelegateHandleList(_oOther.oDelegateHandleList.Flush())
                {
                }

                Delegate(const Delegate&) = delete;
                Delegate& operator=(const Delegate&) = delete;

                bool __YYAPI AddHandler(DelegateHandle* _pHandle)
                {
                    if (!_pHandle)
                    {
                        return false;
                    }

                    if(_pHandle->pPrior != nullptr || _pHandle->pNext != nullptr)
                    {
                        // 已经在链表中！！！
                        return false;
                    }

                    oDelegateHandleList.PushBack(_pHandle);
                    return true;
                }

                bool __YYAPI RemoveHandler(DelegateHandle* _pHandle)
                {
                    if (!_pHandle)
                        return false;

                    if (_pHandle->pPrior == nullptr)
                    {
                        // 头节点
                        if (oDelegateHandleList.GetFirst() == _pHandle)
                        {
                            oDelegateHandleList.Remove(_pHandle);
                            return true;
                        }
                    }
                    else if (_pHandle->pNext == nullptr)
                    {
                        // 尾节点
                        if (oDelegateHandleList.GetLast() == _pHandle)
                        {
                            oDelegateHandleList.Remove(_pHandle);
                            return true;
                        }
                    }
                    else
                    {
                        // 中间节点，检查是否在链表中
                        for (auto _pEntry = oDelegateHandleList.GetFirst(); _pEntry != nullptr; _pEntry = _pEntry->pNext)
                        {
                            if (_pEntry == _pHandle)
                            {
                                oDelegateHandleList.Remove(_pHandle);
                                return true;
                            }
                        }
                    }

                    assert(false && "The delegate handle is not in the delegate list.");
                    return false;
                }

                Delegate& __YYAPI operator+=(Delegate&& _oOther)
                {
                    oDelegateHandleList.PushBack(_oOther.oDelegateHandleList.Flush());
                    return *this;
                }

                Delegate& __YYAPI operator+=(DelegateHandle* _pHandle)
                {
                    AddHandler(_pHandle);
                    return *this;
                }

                Delegate& __YYAPI operator-=(DelegateHandle* _pHandle)
                {
                    RemoveHandler(_pHandle);
                    return *this;
                }

                void __YYAPI InvokeAllHandlers(auto&&... _args) const
                {
                    for (auto _pEntry = oDelegateHandleList.GetFirst(); _pEntry != nullptr;)
                    {
                        auto _pNext = _pEntry->pNext;
                        _pNext->OnCompleted(std::forward<decltype(_args)>(_args)...);
                        _pEntry = _pNext;
                    }
                }
            };

            class AsyncInfo : public YY::RefValue
            {
            protected:
                volatile HRESULT hr = E_PENDING;
                volatile AsyncStatus eStatus = AsyncStatus::Started;

            public:
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
                    if ((AsyncStatus)YY::Sync::CompareExchange(reinterpret_cast<volatile uint32_t*>(&eStatus), uint32_t(AsyncStatus::Canceled), uint32_t(AsyncStatus::Started)) != AsyncStatus::Started)
                    {
                        return false;
                    }

                    WakeByAddressAll((PVOID)&eStatus);
                    NotifyCompletedHandlers();
                    return true;
                }

                /// <summary>
                /// 阻塞等待任务完成或直到达到指定超时时间（默认无限等待）。
                /// </summary>
                /// <param name="_oTimeout">最大等待时间，类型为 YY::TimeSpan。默认值为 YY::TimeSpan::GetMax()，表示无限期等待。</param>
                /// <returns>如果在指定超时时间内等待成功（任务完成等）则返回 true；如果超时则返回 false。</returns>
                bool __YYAPI WaitTask(YY::TimeSpan _oTimeout = YY::TimeSpan::GetMax())
                {
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

                    AsyncStatus _eStartedStatus = AsyncStatus::Started;
                    return WaitOnAddress(&eStatus, &_eStartedStatus, sizeof(_eStartedStatus), _uMilliseconds);
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

                    hr = _hr;
                    WakeByAddressAll((PVOID)&eStatus);
                    NotifyCompletedHandlers();
                    return true;
                }
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
                using AsyncOperationCompletedHandler = YY::AsyncOperationCompletedHandler<ResultType>;
                using Delegate = YY::Delegate<AsyncOperationCompletedHandler>;

            protected:
                YY::SRWLock oSRWLock;
                Delegate oCompletedDelegate;

            public:
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
                void __YYAPI NotifyCompletedHandlers() override
                {
                    Delegate _oCompletedDelegate;
                    {
                        YY::AutoLock<YY::SRWLock> _oAutoSwapLock(oSRWLock);
                        _oCompletedDelegate += std::move(oCompletedDelegate);
                    }

                    _oCompletedDelegate.InvokeAllHandlers(this, GetStatus());
                }
            };

            template<>
            class AsyncOperation<void> : public AsyncInfo
            {
            public:
                using ResultType = void;
                using AsyncOperationCompletedHandler = YY::AsyncOperationCompletedHandler<ResultType>;
                using Delegate = YY::Delegate<AsyncOperationCompletedHandler>;

            protected:
                YY::SRWLock oSRWLock;
                Delegate oCompletedDelegate;

            public:
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
                void __YYAPI NotifyCompletedHandlers() override
                {
                    Delegate _oCompletedDelegate;
                    {
                        YY::AutoLock<YY::SRWLock> _oAutoSwapLock(oSRWLock);
                        _oCompletedDelegate += std::move(oCompletedDelegate);
                    }

                    _oCompletedDelegate.InvokeAllHandlers(this, GetStatus());
                }
            };
        }
    }
}

#pragma pack(pop)
