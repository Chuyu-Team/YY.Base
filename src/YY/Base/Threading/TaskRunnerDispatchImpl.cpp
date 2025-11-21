#include "TaskRunnerDispatchImpl.h"

#include <utility>

#ifndef _WIN32
#include <signal.h>
#endif

#include <YY/Base/Time/TimeSpan.h>
#include <YY/Base/Sync/Interlocked.h>
#include <YY/Base/Utils/SystemInfo.h>

#include "TaskRunnerImpl.h"

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            template<typename DerivedClass>
            class TaskRunnerDispatchBaseImpl
                : public TaskRunnerDispatch
                , public ThreadPoolTimerManger
                , public ThreadPoolWaitMangerForMultiThreading
            {
            protected:
                // 该 Dispatch持有的任务引用计数，如果引用计数 <=0，那么可以归将线程归还给全局线程池。
                volatile int32_t nDispatchTaskRef = 0ul;

                enum : uint32_t
                {
                    PendingTaskQueuePushLockBitIndex = 0,
                    WakeupRefBitIndex,

                    WakeupRefOnceRaw = 1 << WakeupRefBitIndex,
                    UnlockQueuePushLockBitAndWakeupRefOnceRaw = WakeupRefOnceRaw - (1u << PendingTaskQueuePushLockBitIndex),
                };

                union
                {
                    volatile uint32_t fFlags = 0ul;
                    struct
                    {
                        uint32_t bPendingTaskQueuePushLock : 1;
                        // 唤醒的引用计数，这里偶尔因为时序，从 0 溢出也没事。
                        // 这里只是做一个记录。
                        uint32_t uWakeupRef : 31;
                    };
                };

                InterlockedQueue<Timer> oPendingTimerTaskQueue;
                InterlockedQueue<Wait> oPendingWaitTaskQueue;

            public:
#ifdef _WIN32
                TaskRunnerDispatchBaseImpl(HANDLE _hTaskRunnerServerHandle)
                    : ThreadPoolWaitMangerForMultiThreading(_hTaskRunnerServerHandle)
                {
                }
#endif

                ~TaskRunnerDispatchBaseImpl()
                {
                    while (auto _pTask = oPendingTimerTaskQueue.Pop())
                    {
                        _pTask->Cancel();
                        _pTask->Wakeup(YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED));
                        _pTask->Release();
                    }

                    while (auto _pTask = oPendingWaitTaskQueue.Pop())
                    {
                        _pTask->Cancel();
                        _pTask->Wakeup(YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED));
                        _pTask->Release();
                    }
                }

                void __YYAPI SetTimerInternal(_In_ RefPtr<Timer> _pTimer) noexcept override
                {
                    if (!_pTimer)
                        return;

                    JoinPendingTaskQueue(oPendingTimerTaskQueue, std::move(_pTimer));
                }

                HRESULT __YYAPI SetWaitInternal(_In_ RefPtr<Wait> _pWait) noexcept override
                {
                    if (_pWait == nullptr || _pWait->hHandle == NULL)
                        return E_INVALIDARG;

                    JoinPendingTaskQueue(oPendingWaitTaskQueue, std::move(_pWait));
                    return S_OK;
                }

            protected:
                size_t ProcessingPendingTaskQueue() noexcept
                {
                    size_t _cTaskProcessed = 0;
                    for (;;)
                    {
                        auto _pTimerTask = oPendingTimerTaskQueue.Pop();
                        if (!_pTimerTask)
                            break;

                        YY::Sync::Subtract(&fFlags, WakeupRefOnceRaw);
                        auto _hr = ThreadPoolTimerManger::SetTimerInternal(RefPtr<Timer>::FromPtr(_pTimerTask));
                        if (FAILED(_hr))
                        {
                            ++_cTaskProcessed;
                        }
                    }

                    for (;;)
                    {
                        auto _pWait = oPendingWaitTaskQueue.Pop();
                        if (!_pWait)
                            break;

                        YY::Sync::Subtract(&fFlags, WakeupRefOnceRaw);
                        auto _hr = ThreadPoolWaitMangerForMultiThreading::SetWaitInternal(RefPtr<Wait>::FromPtr(_pWait));
                        if (_hr == S_FALSE ||  FAILED(_hr))
                        {
                            ++_cTaskProcessed;
                        }
                    }
                    return _cTaskProcessed;
                }

                void __YYAPI DispatchTimerTask(RefPtr<Timer> _pTimerTask) override
                {
                    if (_pTimerTask)
                    {
                        DispatchTask(std::move(_pTimerTask));
                    }
                }

                void __YYAPI DispatchWaitTask(RefPtr<Wait> _pWaitTask) override
                {
                    if (_pWaitTask)
                    {
                        DispatchTask(std::move(_pWaitTask));
                    }
                }

                template<typename TaskType>
                void __YYAPI JoinPendingTaskQueue(InterlockedQueue<TaskType>& _oPendingTaskQueue, _In_ RefPtr<TaskType> _pTask) noexcept
                {
                    // 先增加任务计数，防止这段时间，服务线程不必要的意外归还线程池
                    const auto _nNewDispatchTaskRef = YY::Sync::Increment(&nDispatchTaskRef);
                    for (;;)
                    {
                        if (!Sync::BitSet(&fFlags, PendingTaskQueuePushLockBitIndex))
                        {
                            _oPendingTaskQueue.Push(_pTask.Detach());
                            const auto _uNewFlags = Sync::Add(&fFlags, UnlockQueuePushLockBitAndWakeupRefOnceRaw);
                            static_cast<DerivedClass*>(this)->Weakup(_nNewDispatchTaskRef, _uNewFlags);
                            break;
                        }
                    }

                    return;
                }
            };

#ifdef _WIN32
            /// <summary>
            /// 用于Windows Task调度器。主要服务于SequencedTaskRunner。
            /// </summary>
            class TaskRunnerDispatchForWindows : public TaskRunnerDispatch
            {
            public:
                constexpr TaskRunnerDispatchForWindows() = default;

                bool __YYAPI BindIO(_In_ HANDLE _hHandle) const noexcept override
                {
                    if (_hHandle == INVALID_HANDLE_VALUE)
                    {
                        return false;
                    }

                    // 使用 BindIoCompletionCallback以获得更加友好的线程池调度。
                    // XP的线程池接口直接再Wait函数中执行回调，刚好方便YY.Base将任务调度到对应的TaskRunner。
                    const auto _bRet = BindIoCompletionCallback(
                        _hHandle,
                        [](DWORD _uErrorCode, DWORD _cbNumberOfBytesTransfered, LPOVERLAPPED _pOverlapped)
                        {
                            auto _pDispatchTask = RefPtr<IoTaskEntry>::FromPtr(static_cast<IoTaskEntry*>(_pOverlapped));
                            if (!_pDispatchTask)
                                return;

                            // 错误代码如果已经设置，那么可能调用者线程已经事先处理了。
                            if (_pDispatchTask->OnComplete(_uErrorCode))
                            {
                                DispatchTask(std::move(_pDispatchTask));
                            }
                        },
                        0);

                    return _bRet;
                }

                void __YYAPI StartIo() noexcept override
                {
                }

                void __YYAPI SetTimerInternal(_In_ RefPtr<Timer> _pTimer) noexcept override
                {
                    if (!_pTimer)
                        return;

                    if (HANDLE _hThreadPoolTimer = YY::ExchangePoint(&_pTimer->hThreadPoolTimer, nullptr))
                    {
                        DeleteTimerQueueTimer(NULL, _hThreadPoolTimer, INVALID_HANDLE_VALUE);
                    }

                    const auto _uCurrent = TickCount::GetNow();
                    const auto _iDueTime = (_pTimer->uExpire - _uCurrent).GetTotalMilliseconds();
                    if (_iDueTime <= 0)
                    {
                        _pTimer->uExpire = _uCurrent;
                        DispatchTask(std::move(_pTimer));
                    }
                    else
                    {
                        // 我们使用 CreateTimerQueueTimer这是因为它允许我们在线程池线程中执行回调，没有额外的上下文切换开销。
                        auto _bRet = CreateTimerQueueTimer(
                            &_pTimer->hThreadPoolTimer,
                            NULL,
                            [](PVOID _pParameter, BOOLEAN _bTimerFired)
                            {
                                auto _pTimerTask = RefPtr<Timer>::FromPtr(static_cast<Timer*>(_pParameter));
                                if (!_pTimerTask)
                                    return;

                                DispatchTask(std::move(_pTimerTask));
                            },
                            _pTimer.Get(),
                            _iDueTime,
                            0,
                            WT_EXECUTEINTIMERTHREAD | WT_EXECUTEONLYONCE);

                        if (!_bRet)
                        {
                            return;
                        }

                        _pTimer.Get()->AddRef();
                    }
                }

                HRESULT __YYAPI SetWaitInternal(_In_ RefPtr<Wait> _pWait) noexcept override
                {
                    if (_pWait == nullptr || _pWait->hHandle == NULL)
                        return E_INVALIDARG;

                    if (HANDLE _hThreadPoolWait = YY::ExchangePoint(&_pWait->hThreadPoolWait, nullptr))
                    {
                        UnregisterWaitEx(_hThreadPoolWait, INVALID_HANDLE_VALUE);
                    }

                    // 我们使用 RegisterWaitForSingleObject 这是因为它允许我们在线程池线程中执行回调，没有额外的上下文切换开销。
                    auto _bRet = RegisterWaitForSingleObject(
                        &_pWait->hThreadPoolWait,
                        _pWait->hHandle,
                        [](PVOID _pParameter, BOOLEAN _bTimeout)
                        {
                            auto _pWaitTask = RefPtr<Wait>::FromPtr(static_cast<Wait*>(_pParameter));
                            if (!_pWaitTask)
                                return;

                            _pWaitTask->uWaitResult = _bTimeout ? WAIT_TIMEOUT : WAIT_OBJECT_0;
                            DispatchTask(std::move(_pWaitTask));
                        },
                        _pWait.Get(),
                        (_pWait->uTimeOut - TickCount::GetNow()).GetTotalMilliseconds(),
                        WT_EXECUTEINWAITTHREAD | WT_EXECUTEONLYONCE);

                    if (!_bRet)
                    {
                        return __HRESULT_FROM_WIN32(GetLastError());
                    }

                    _pWait.Get()->AddRef();
                    return S_OK;
                }
            };
#endif

            TaskRunnerDispatch* __YYAPI TaskRunnerDispatch::Get() noexcept
            {
                static TaskRunnerDispatch* s_pCurrentTaskRunnerDispatch = nullptr;
                if (!s_pCurrentTaskRunnerDispatch)
                {
#ifdef _WIN32
                    static TaskRunnerDispatchForWindows s_TaskRunnerDispatch;
                    s_pCurrentTaskRunnerDispatch = &s_TaskRunnerDispatch;
#else
#error "其他系统尚未适配"
#endif
                }
                return s_pCurrentTaskRunnerDispatch;
            }
}
    }
}
