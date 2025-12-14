#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Memory/RefPtr.h>
#include <YY/Base/Memory/WeakPtr.h>
#include <YY/Base/Memory/ObserverPtr.h>
#include <YY/Base/Functional/Delegate.h>
#include <YY/Base/Sync/CriticalSection.h>
#include <YY/Base/Sync/AutoLock.h>
#include <YY/Base/Sync/Interlocked.h>
#include <YY/Base/Exception.h>
#include <memory>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            class CancellationToken;

            class CancellationTokenCancelHandle : public DoublyLinkedListEntryImpl<CancellationTokenCancelHandle>
            {
            public:
                virtual void __YYAPI OnCanceled() = 0;
            };

            class CancellationToken : public RefValue
            {
            public:
                /// <summary>
                /// 指示是否已请求取消操作。
                /// </summary>
                /// <returns>如果已请求取消则返回 true，否则返回 false。</returns>
                virtual bool __YYAPI IsCancellationRequested() const = 0;

                virtual bool __YYAPI Register(_In_ CancellationTokenCancelHandle* _pCallback)
                {
                    UNREFERENCED_PARAMETER(_pCallback);
                    return false;
                }

                virtual bool __YYAPI Unregister(_In_ CancellationTokenCancelHandle* _pCallback)
                {
                    UNREFERENCED_PARAMETER(_pCallback);
                    return false;
                }

                void __YYAPI ThrowIfCancellationRequested() const
                {
                    if (IsCancellationRequested())
                    {
                        throw OperationCanceledException();
                    }
                }

                template<typename T>
                static RefPtr<CancellationToken> __YYAPI FromPtr(ObserverPtr<T> _pObserverPtr)
                {
                    class CancellationTokenImpl : public CancellationToken
                    {
                    private:
                        ObserverPtr<T> pObserverPtr;

                    public:
                        CancellationTokenImpl(ObserverPtr<T> _pObserverPtr) noexcept
                            : pObserverPtr(std::move(_pObserverPtr))
                        {
                        }

                        bool __YYAPI IsCancellationRequested() const override
                        {
                            return pObserverPtr.IsExpired();
                        }
                    };

                    return RefPtr<CancellationToken>(new CancellationTokenImpl(std::move(_pObserverPtr)));
                }

                template<typename T>
                static RefPtr<CancellationToken> __YYAPI FromPtr(WeakPtr<T> _pWeakPtr)
                {
                    class CancellationTokenImpl : public CancellationToken
                    {
                    private:
                        WeakPtr<T> pWeakPtr;

                    public:
                        CancellationTokenImpl(WeakPtr<T> _pWeakPtr) noexcept
                            : pWeakPtr(std::move(_pWeakPtr))
                        {
                        }

                        bool __YYAPI IsCancellationRequested() const override
                        {
                            return pWeakPtr.IsExpired();
                        }
                    };

                    return RefPtr<CancellationToken>(new CancellationTokenImpl(std::move(_pWeakPtr)));
                }

                template<typename T>
                static RefPtr<CancellationToken> __YYAPI FromPtr(std::weak_ptr<T> _pWeakPtr)
                {
                    class CancellationTokenImpl : public CancellationToken
                    {
                    private:
                        std::weak_ptr<T> pWeakPtr;

                    public:
                        CancellationTokenImpl(std::weak_ptr<T> _pWeakPtr) noexcept
                            : pWeakPtr(std::move(_pWeakPtr))
                        {
                        }

                        bool __YYAPI IsCancellationRequested() const override
                        {
                            return pWeakPtr.expired();
                        }
                    };

                    return RefPtr<CancellationToken>(new CancellationTokenImpl(std::move(_pWeakPtr)));
                }
            };

            class CancellationTokenSource : public CancellationToken
            {
            public:
                virtual bool __YYAPI Cancel() noexcept = 0;

                static RefPtr<CancellationTokenSource> __YYAPI Create()
                {
                    class CancellationTokenSourceImpl : public CancellationTokenSource
                    {
                    private:
                        volatile uint32_t bCancel = 0;
                        YY::CriticalSection oCallbacksLock;
                        Delegate<CancellationTokenCancelHandle> oCallbacks;

                    public:
                        bool __YYAPI IsCancellationRequested() const noexcept override
                        {
                            return bCancel != 0;
                        }

                        bool __YYAPI Register(_In_ CancellationTokenCancelHandle* _pCallback) override
                        {
                            if (!_pCallback)
                            {
                                return false;
                            }

                            YY::AutoLock<YY::CriticalSection> _oAutoLock(oCallbacksLock);
                            if (IsCancellationRequested())
                            {
                                return false;
                            }

                            return oCallbacks.AddHandler(_pCallback);
                        }

                        bool __YYAPI Unregister(_In_ CancellationTokenCancelHandle* _pCallback) override
                        {
                            if (!_pCallback)
                            {
                                return false;
                            }

                            YY::AutoLock<YY::CriticalSection> _oAutoLock(oCallbacksLock);
                            return oCallbacks.RemoveHandler(_pCallback);
                        }

                        bool __YYAPI Cancel() noexcept override
                        {
                            if (YY::Sync::Exchange(&bCancel, 1u) != 0u)
                            {
                                return false;
                            }

                            {
                                Delegate<CancellationTokenCancelHandle> _oPendingCallbacks;

                                YY::AutoLock<YY::CriticalSection> _oAutoLock(oCallbacksLock);
                                _oPendingCallbacks += std::move(oCallbacks);

                                _oPendingCallbacks.Invoke(
                                    [this](CancellationTokenCancelHandle* _pCallback)
                                    {
                                        _pCallback->OnCanceled();
                                    });
                            }
                            return true;
                        }
                    };

                    return RefPtr<CancellationTokenSource>::FromPtr(new CancellationTokenSourceImpl());
                }
            };
        }
    }

    using namespace Base::Threading;
}

#pragma pack(pop)
