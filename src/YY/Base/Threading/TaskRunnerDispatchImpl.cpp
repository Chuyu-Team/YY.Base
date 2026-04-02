#include "TaskRunnerDispatchImpl.h"

#ifndef _WIN32
#include <signal.h>
#endif

#include <YY/Base/Time/TimeSpan.h>
#include <YY/Base/Sync/Interlocked.h>

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
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
                            auto _pIoAsyncOperation = RefPtr<IoAsyncOperation<void>>::FromPtr(static_cast<IoAsyncOperation<void>*>(_pOverlapped));
                            if (!_pIoAsyncOperation)
                                return;

                            // 错误代码如果已经设置，那么可能调用者线程已经事先处理了。
                            _pIoAsyncOperation->Resolve(_uErrorCode);
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
                        if (_hThreadPoolTimer != INVALID_HANDLE_VALUE)
                        {
                            if (DeleteTimerQueueTimer(NULL, _hThreadPoolTimer, NULL))
                            {
                                _pTimer.Get()->Release();
                            }
                        }
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
                        _pTimer.Get()->AddRef();
                        // 我们使用 CreateTimerQueueTimer这是因为它允许我们在线程池线程中执行回调，没有额外的上下文切换开销。
                        auto _bRet = CreateTimerQueueTimer(
                            &_pTimer->hThreadPoolTimer,
                            NULL,
                            [](PVOID _pParameter, BOOLEAN _bTimerFired)
                            {
                                auto _pTimer = RefPtr<Timer>::FromPtr(static_cast<Timer*>(_pParameter));
                                if (!_pTimer)
                                    return;

                                if (HANDLE _hThreadPoolTimer = YY::ExchangePoint(&_pTimer->hThreadPoolTimer, nullptr))
                                {
                                    if (_hThreadPoolTimer != INVALID_HANDLE_VALUE)
                                    {
                                        DeleteTimerQueueTimer(NULL, _hThreadPoolTimer, NULL);
                                    }
                                }

                                DispatchTask(std::move(_pTimer));
                            },
                            _pTimer.Get(),
                            (DWORD)(std::min)(_iDueTime, static_cast<int64_t>(UINT32_MAX)),
                            0,
                            WT_EXECUTEINTIMERTHREAD | WT_EXECUTEONLYONCE);

                        if (!_bRet)
                        {
                            auto _hr = __HRESULT_FROM_WIN32(GetLastError());
                            _pTimer->Wakeup(_hr);
                            _pTimer.Get()->Release();
                            return;
                        }
                    }
                }

                HRESULT __YYAPI SetWaitInternal(_In_ RefPtr<WaitAsyncOperation> _pWait) noexcept override
                {
                    if (_pWait == nullptr || _pWait->hHandle == NULL)
                        return E_INVALIDARG;

                    if (HANDLE _hThreadPoolWait = YY::ExchangePoint(&_pWait->hThreadPoolWait, NULL))
                    {
                        if (_hThreadPoolWait != INVALID_HANDLE_VALUE)
                        {
                            if (UnregisterWaitEx(_hThreadPoolWait, NULL))
                            {
                                _pWait.Get()->ReleaseWeak();
                            }
                        }
                    }
                    const auto _iTotalMilliseconds = (_pWait->uTimeOut - TickCount::GetNow()).GetTotalMilliseconds();
                    if (_iTotalMilliseconds <= 0)
                    {
                        _pWait->Resolve(WaitForSingleObject(_pWait->hHandle, 0));
                        return S_OK;
                    }

                    _pWait.Get()->AddWeakRef();
                    // 我们使用 RegisterWaitForSingleObject 这是因为它允许我们在线程池线程中执行回调，没有额外的上下文切换开销。
                    auto _bRet = RegisterWaitForSingleObject(
                        &_pWait->hThreadPoolWait,
                        _pWait->hHandle,
                        [](PVOID _pParameter, BOOLEAN _bTimeout)
                        {
                            YY::WeakPtr<WaitAsyncOperation> _pWaitWeak(reinterpret_cast<WeakPtrRef<WaitAsyncOperation>*>(_pParameter));
                            auto _pWait = _pWaitWeak.Get();
                            if (!_pWait)
                                return;

                            if (HANDLE _hThreadPoolWait = YY::ExchangePoint(&_pWait->hThreadPoolWait, NULL))
                            {
                                if (_hThreadPoolWait != INVALID_HANDLE_VALUE)
                                {
                                    UnregisterWaitEx(_hThreadPoolWait, NULL);
                                }
                            }

                            _pWait->Resolve(_bTimeout ? WAIT_TIMEOUT : WAIT_OBJECT_0);
                        },
                        _pWait.Get(),
                        (DWORD)(std::min)(_iTotalMilliseconds, static_cast<int64_t>(UINT32_MAX)),
                        WT_EXECUTEINWAITTHREAD | WT_EXECUTEONLYONCE);

                    if (!_bRet)
                    {
                        auto _hr = __HRESULT_FROM_WIN32(GetLastError());
                        _pWait.Get()->ReleaseWeak();
                        _pWait->SetErrorCode(_hr);
                        return _hr;
                    }

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
