#include <YY/Base/Threading/TaskRunner.h>

#include <YY/Base/Exception.h>
#include <YY/Base/Sync/Sync.h>

#include "ThreadTaskRunnerImpl.h"
#include "SequencedTaskRunnerImpl.h"
#include "ParallelTaskRunnerImpl.hpp"
#include "TaskRunnerDispatchImpl.h"
#include "ThreadTaskRunnerProxyImpl.h"

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            static constexpr TickCount __YYAPI GetTimeout(TimeSpan _iWaitTimeOut) noexcept
            {
                if (_iWaitTimeOut.GetTicks() <= 0)
                {
                    return TickCount::FromTicks(0);
                }
                else if (_iWaitTimeOut >= TimeSpan::FromMilliseconds(UINT32_MAX))
                {
                    return TickCount::GetMax();
                }
                else
                {
                    return TickCount::GetNow() + _iWaitTimeOut;
                }
            }
            
            TaskEntry::~TaskEntry()
            {
                Cancel();
            }

            void __YYAPI TaskEntry::Wakeup(HRESULT _hrCode)
            {
                if (YY::CompareExchange(&hr, _hrCode, E_PENDING) == E_PENDING)
                {
                    WakeByAddressAll(&hr);
                }
            }

            bool __YYAPI TaskEntry::WaitTask(YY::TimeSpan _oTimeout)
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

                HRESULT _hrTarget = E_PENDING;
                return WaitOnAddress(&hr, &_hrTarget, sizeof(_hrTarget), _uMilliseconds);
            }

            bool __YYAPI TaskEntry::Cancel()
            {
                if (!YY::Sync::BitSet((int32_t*)&fStyle, 2))
                {
                    Wakeup(YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED));
                    return true;
                }

                return false;
            }

            HRESULT __YYAPI TaskEntry::RunTask()
            {
                if (IsCanceled())
                    return YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED);

                HRESULT _hr = S_OK;
                try
                {
                    pfnTaskCallback();
                }
                catch (const YY::Base::OperationCanceledException& _Exception)
                {
                    UNREFERENCED_PARAMETER(_Exception);
                    _hr = YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED);
                }

                Wakeup(_hr);
                return _hr;
            }

            HRESULT __YYAPI Timer::RunTask()
            {
                if (IsCanceled())
                    return YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED);

                if (uInterval.GetTotalMilliseconds() <= 0)
                {
                    return TaskEntry::RunTask();
                }
                else
                {
                    HRESULT _hr = S_OK;
                    bool _bRet = false;
                    auto _uExpire = TickCount::GetNow() + uInterval;
                    try
                    {
                        _bRet = pfnTimerCallback();
                    }
                    catch (const YY::Base::OperationCanceledException& _Exception)
                    {
                        UNREFERENCED_PARAMETER(_Exception);
                        _hr = YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED);
                    }

                    Wakeup(_hr);

                    if (_bRet)
                    {
                        auto _pOwnerTaskRunner = pOwnerTaskRunnerWeak.Get();
                        // 任务被取消？
                        if (!_pOwnerTaskRunner)
                            return S_OK;

                        uExpire = _uExpire;
                        return _pOwnerTaskRunner->SetTimerInternal(this);
                    }

                    return _hr;
                }
            }

            bool __YYAPI Timer::Cancel()
            {
                const auto _bRet = TaskEntry::Cancel();

                HANDLE _hThreadPoolTimer = YY::ExchangePoint(&hThreadPoolTimer, nullptr);
                if (_hThreadPoolTimer == INVALID_HANDLE_VALUE)
                {
                    // 标记任务取消后，线程后续会自动释放资源。
                }
                else if(_hThreadPoolTimer)
                {
                    // 如果失败，往往意味着回调函数正在进行，此时交给回调函数 执行Release。
                    if (DeleteTimerQueueTimer(NULL, _hThreadPoolTimer, NULL))
                    {
                        Release();
                    }
                }

                return _bRet;
            }

            Threading::TaskRunner::TaskRunner()
                : uTaskRunnerId(GenerateNewTaskRunnerId())
            {
            }

            RefPtr<TaskRunner> __YYAPI Threading::TaskRunner::GetCurrent()
            {
                return g_pTaskRunnerWeak.Get();
            }

            bool __YYAPI TaskRunner::BindIO(HANDLE _hHandle) noexcept
            {
                return TaskRunnerDispatch::Get()->BindIO(_hHandle);
            }

            void __YYAPI TaskRunner::StartIo() noexcept
            {
                return TaskRunnerDispatch::Get()->StartIo();
            }

            Task<HRESULT> __YYAPI TaskRunner::SleepAsync(TimeSpan _uAfter)
            {
                class SleepAsyncOperation : public AsyncOperationImpl<HRESULT>
                {
                public:

                };

                auto _pSleepAsyncOperation = YY::RefPtr<SleepAsyncOperation>::Create();

                auto _pTaskRunner = TaskRunner::GetCurrent();
                if (!_pTaskRunner)
                {
                    _pSleepAsyncOperation->Resolve(E_UNEXPECTED);
                    return Task<HRESULT>(std::move(_pSleepAsyncOperation));
                }

                _pTaskRunner->PostDelayTask(_uAfter, [_pSleepAsyncOperation]() mutable
                    {
                        _pSleepAsyncOperation->Resolve(S_OK);
                    });

                return Task<HRESULT>(std::move(_pSleepAsyncOperation));
            }

#if defined(_WIN32)
            Task<DWORD>__YYAPI TaskRunner::WaitForObjectAsync(HANDLE _hHandle, TimeSpan _iWaitTimeOut)
            {
                auto _pWaitForObjectAsyncOperation = YY::RefPtr<WaitAsyncOperation>::Create();
                _pWaitForObjectAsyncOperation->uTimeOut = GetTimeout(_iWaitTimeOut);
                _pWaitForObjectAsyncOperation->hHandle = _hHandle;

                if (_hHandle == NULL || _hHandle == INVALID_HANDLE_VALUE)
                {
                    _pWaitForObjectAsyncOperation->Resolve(WAIT_FAILED);
                    return Task<DWORD>(std::move(_pWaitForObjectAsyncOperation));
                }

                auto _pTaskRunner = TaskRunner::GetCurrent();
                if (_pTaskRunner)
                {
                    _pTaskRunner->SetWaitInternal(_pWaitForObjectAsyncOperation);
                }
                else
                {
                    TaskRunnerDispatch::Get()->SetWaitInternal(_pWaitForObjectAsyncOperation);
                }

                return Task<DWORD>(std::move(_pWaitForObjectAsyncOperation));
            }
#endif

            HRESULT __YYAPI TaskRunner::PostDelayTask(TimeSpan _uAfter, std::function<void(void)>&& _pfnTaskCallback)
            {
                auto _uExpire = TickCount::GetNow() + _uAfter;
                auto _pTimer = RefPtr<Timer>::Create();
                if (!_pTimer)
                    return E_OUTOFMEMORY;

                _pTimer->pfnTaskCallback = std::move(_pfnTaskCallback);
                _pTimer->uExpire = _uExpire;
                return SetTimerInternal(std::move(_pTimer));
            }

            HRESULT __YYAPI TaskRunner::PostTask(std::function<void(void)>&& _pfnTaskCallback)
            {
                auto _pTask = RefPtr<TaskEntry>::Create();
                if (!_pTask)
                    return E_OUTOFMEMORY;

                _pTask->pfnTaskCallback = std::move(_pfnTaskCallback);
                return PostTaskInternal(_pTask);
            }

            RefPtr<SequencedTaskRunner> __YYAPI SequencedTaskRunner::GetCurrent()
            {
                auto _pTaskRunner = g_pTaskRunnerWeak.Get();
                if (_pTaskRunner == nullptr || HasFlags(_pTaskRunner->GetStyle(), TaskRunnerStyle::Sequenced) == false)
                    return nullptr;

                return RefPtr<SequencedTaskRunner>(std::move(_pTaskRunner));
            }

            RefPtr<SequencedTaskRunner> __YYAPI SequencedTaskRunner::Create(uString _szThreadDescription)
            {
                return RefPtr<SequencedTaskRunnerImpl>::Create(std::move(_szThreadDescription));
            }

            HRESULT __YYAPI TaskRunner::SendTask(std::function<void(void)>&& pfnTaskCallback)
            {
                // 调用者的跟执行者属于同一个TaskRunner，这时我们直接调用 _pfnCallback，避免各种等待以及任务投递开销。
                if (TaskRunner::GetCurrent() == this)
                {
                    try
                    {
                        pfnTaskCallback();
                        return S_OK;
                    }
                    catch (const YY::Base::OperationCanceledException& _Exception)
                    {
                        UNREFERENCED_PARAMETER(_Exception);
                        return YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED);
                    }
                }

                TaskEntry _oWorkEntry;
                _oWorkEntry.fStyle = TaskEntryStyle::Sync;
                _oWorkEntry.pfnTaskCallback = std::move(pfnTaskCallback);

                auto _hr = PostTaskInternal(&_oWorkEntry);
                if (FAILED(_hr))
                {
                    return _hr;
                }
                _oWorkEntry.WaitTask();
                return _oWorkEntry.hr;
            }

            RefPtr<Timer> __YYAPI TaskRunner::CreateTimer(TimeSpan _uInterval, std::function<bool(void)>&& _pfnTaskCallback)
            {
                if (_uInterval.GetTotalMilliseconds() <= 0)
                    return nullptr;

                auto _uCurrent = TickCount::GetNow();
                auto _pTimer = RefPtr<Timer>::Create();
                if (!_pTimer)
                    return nullptr;

                _pTimer->pfnTimerCallback = std::move(_pfnTaskCallback);
                _pTimer->uInterval = _uInterval;
                _pTimer->uExpire = _uCurrent + _uInterval;
                auto _hr = SetTimerInternal(_pTimer);
                if (FAILED(_hr))
                    return nullptr;

                return _pTimer;
            }

            RefPtr<Wait> __YYAPI TaskRunner::CreateWait(HANDLE _hHandle, TimeSpan _iWaitTimeOut, std::function<bool(DWORD _uWaitResult)>&& _pfnTaskCallback)
            {
                if (_hHandle == nullptr || _hHandle == INVALID_HANDLE_VALUE)
                    return nullptr;

                if (!_pfnTaskCallback)
                    return nullptr;

                auto _pWait = RefPtr<Wait>::Create();
                if (!_pWait)
                    return nullptr;

                auto _pWaitAsyncOperation = YY::RefPtr<WaitAsyncOperation>::Create();
                _pWaitAsyncOperation->uTimeOut = GetTimeout(_iWaitTimeOut);
                _pWaitAsyncOperation->hHandle = _hHandle;
                _pWaitAsyncOperation->pOwnerTaskRunnerWeak = this;

                _pWait->pWaitAsyncOperation = _pWaitAsyncOperation;
                _pWait->pfnWaitTaskCallback = std::move(_pfnTaskCallback);
                _pWait->pOwnerTaskRunnerWeak = this;
                _pWait->iTimeoutSpan = _iWaitTimeOut;

                if (_hHandle == nullptr || _hHandle == INVALID_HANDLE_VALUE)
                {
                    _pWaitAsyncOperation->Resolve(WAIT_FAILED);
                }
                else
                {
                    _pWait->InstallCompleted();
                    SetWaitInternal(_pWaitAsyncOperation);
                }

                return _pWait;
            }

            HRESULT __YYAPI TaskRunner::SetTimerInternal(RefPtr<Timer> _pTask)
            {
                _pTask->pOwnerTaskRunnerWeak = this;
                _pTask->hr = E_PENDING;

                if (_pTask->IsCanceled())
                {
                    _pTask->Wakeup(YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED));
                    return YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED);
                }
                else if (_pTask->uExpire.GetTicks() == 0 || (_pTask->uExpire - YY::TickCount::GetNow()).GetTotalMilliseconds() <= 0)
                {
                    return PostTaskInternal(std::move(_pTask));
                }

                TaskRunnerDispatch::Get()->SetTimerInternal(std::move(_pTask));
                return S_OK;
            }

            HRESULT __YYAPI TaskRunner::SetWaitInternal(RefPtr<WaitAsyncOperation> _pTask)
            {
                if (_pTask->IsCanceled())
                {
                    return YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED);
                }

                TaskRunnerDispatch::Get()->SetWaitInternal(std::move(_pTask));
                return S_OK;
            }

            HRESULT __YYAPI TaskRunner::DeleteWaitInternal(WaitAsyncOperation* _pTask)
            {
                return S_OK;
            }

            RefPtr<ThreadTaskRunner> __YYAPI ThreadTaskRunner::Create(bool _bBackgroundLoop, uString _szThreadDescription)
            {
                auto _pTaskRunner = RefPtr<ThreadTaskRunnerImpl>::Create(0u, _bBackgroundLoop, std::move(_szThreadDescription));
                if (_pTaskRunner)
                {
                    auto _hr = ThreadPool::PostTaskInternal(_pTaskRunner.Get());
                    if (FAILED(_hr))
                    {
                        return nullptr;
                    }
                }
                return _pTaskRunner;
            }

            RefPtr<ThreadTaskRunner> __YYAPI ThreadTaskRunner::GetCurrent()
            {
                auto _pTaskRunner = g_pTaskRunnerWeak.Get();

                // 当前 TaskRunner 必须是物理线程。
                if (_pTaskRunner == nullptr || HasFlags(_pTaskRunner->GetStyle(), TaskRunnerStyle::FixedThread) == false)
                    return nullptr;

                return RefPtr<ThreadTaskRunner>(std::move(_pTaskRunner));
            }

            RefPtr<ThreadTaskRunner> __YYAPI ThreadTaskRunner::BindCurrentThread()
            {
                if (g_pTaskRunnerWeak.Get())
                    return nullptr;

                auto _pThreadTaskRunnerImpl = RefPtr<ThreadTaskRunnerImpl>::Create();
                if (!_pThreadTaskRunnerImpl)
                    return nullptr;

                g_pTaskRunnerWeak = _pThreadTaskRunnerImpl;
                return _pThreadTaskRunnerImpl;
            }

            RefPtr<ThreadTaskRunner> __YYAPI ThreadTaskRunner::BindCurrentThreadForProxyMode()
            {
                if (g_pTaskRunnerWeak.Get())
                    return nullptr;

                auto _pThreadTaskRunnerImpl = RefPtr<ThreadTaskRunnerProxyImpl>::Create();
                if (!_pThreadTaskRunnerImpl)
                    return nullptr;

                if (!_pThreadTaskRunnerImpl->Init())
                    return nullptr;

                g_pTaskRunnerWeak = _pThreadTaskRunnerImpl;
                return _pThreadTaskRunnerImpl;
            }

            uintptr_t __YYAPI ThreadTaskRunner::RunUIMessageLoop()
            {
                RefPtr<ThreadTaskRunnerBaseImpl> _pThreadTaskRunnerImpl;

                if (auto _pTaskRunner = g_pTaskRunnerWeak.Get())
                {
                    if (!HasFlags(_pTaskRunner->GetStyle(), TaskRunnerStyle::FixedThread))
                    {
                        // 非物理线程不能进行UI消息循环！！！
                        // 暂时设计如此，未来再说。
                        throw Exception(E_INVALIDARG);
                    }

                    _pThreadTaskRunnerImpl = std::move(_pTaskRunner);
                }
                else
                {
                    throw Exception(L"尚未调用 BindCurrentThread。", E_INVALIDARG);
                }

                auto _uResult = _pThreadTaskRunnerImpl->RunTaskRunnerLoop();
                return _uResult;
            }

            HRESULT __YYAPI ThreadTaskRunner::PostQuitMessage(uint32_t _uExitCode)
            {
                auto _pTaskRunner = g_pTaskRunnerWeak.Get();
                if (!_pTaskRunner)
                {
                    return E_UNEXPECTED;
                }

                if (!HasFlags(_pTaskRunner->GetStyle(), TaskRunnerStyle::FixedThread))
                {
                    return E_UNEXPECTED;
                }

                ::PostQuitMessage(static_cast<int>(_uExitCode));
                return S_OK;
            }

            RefPtr<ParallelTaskRunner> __YYAPI ParallelTaskRunner::GetCurrent() noexcept
            {
                auto _pTaskRunner = g_pTaskRunnerWeak.Get();
                if (_pTaskRunner == nullptr || _pTaskRunner->GetStyle() != TaskRunnerStyle::None)
                    return nullptr;

                return RefPtr<ParallelTaskRunner>(std::move(_pTaskRunner));
            }

            RefPtr<ParallelTaskRunner> __YYAPI ParallelTaskRunner::Create(uint32_t _uParallelMaximum, uString _szThreadDescription) noexcept
            {
                return RefPtr<ParallelTaskRunnerImpl>::Create(_uParallelMaximum, std::move(_szThreadDescription));
            }

            void __YYAPI Wait::OnCompleted(AsyncOperation<DWORD>* _pAsyncInfo, AsyncStatus _eStatus)
            {
                auto _pThis = YY::RefPtr<Wait>::FromPtr(this);
                if (_eStatus == AsyncStatus::Completed)
                {
                    if (auto _pOwnerTaskRunner = pOwnerTaskRunnerWeak.Get())
                    {
                        _pOwnerTaskRunner->PostTaskInternal(std::move(_pThis));
                    }
                }
            }

            HRESULT __YYAPI Wait::RunTask()
            {
                if (IsCanceled())
                    return YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED);

                HRESULT _hr = S_OK;
                TickCount _uTimeOut = GetTimeout(iTimeoutSpan);;
                bool _bRet = false;
                try
                {
                    _bRet = pfnWaitTaskCallback(pWaitAsyncOperation->GetResult());
                }
                catch (const YY::Base::OperationCanceledException& _Exception)
                {
                    UNREFERENCED_PARAMETER(_Exception);
                    _hr = HRESULT_From_LSTATUS(ERROR_CANCELLED);
                }

                Wakeup(_hr);
                if (_bRet)
                {
                    if (auto _pOwnerTaskRunner = pOwnerTaskRunnerWeak.Get())
                    {
                        pWaitAsyncOperation->Reset();
                        hr = E_PENDING;
                        pWaitAsyncOperation->uTimeOut = _uTimeOut;
                        _pOwnerTaskRunner->SetWaitInternal(pWaitAsyncOperation);
                        InstallCompleted();
                    }
                }

                return _hr;
            }

            bool __YYAPI Wait::Cancel()
            {
                const auto _bRet = TaskEntry::Cancel();
                if (pWaitAsyncOperation)
                {
                    pWaitAsyncOperation->Cancel();
                }

                return _bRet;
            }

            void __YYAPI Wait::InstallCompleted()
            {
                AddRef();
                if (!pWaitAsyncOperation->AddCompletedHandler(this))
                {
                    OnCompleted(pWaitAsyncOperation, pWaitAsyncOperation->GetStatus());
                }
            }
            
            bool __YYAPI WaitAsyncOperation::Cancel()
            {
                if (!BeginNotifyCompletedHandlers(AsyncStatus::Canceled))
                {
                    return false;
                }

                HANDLE _hThreadPoolWait = YY::ExchangePoint(&hThreadPoolWait, NULL);
                if (_hThreadPoolWait == INVALID_HANDLE_VALUE)
                {
                    // Owner线程的线程队列正在等待……
                    if (auto _pOwnerTaskRunner = pOwnerTaskRunnerWeak.Get())
                    {
                        _pOwnerTaskRunner->DeleteWaitInternal(this);
                    }
                }
                else if (_hThreadPoolWait)
                {
                    // 如果失败，往往意味着回调函数正在进行，此时交给回调函数 执行ReleaseWeak即可。
                    if (UnregisterWaitEx(_hThreadPoolWait, NULL))
                    {
                        ReleaseWeak();
                    }
                }

                NotifyCompletedHandlers(__HRESULT_FROM_WIN32(ERROR_CANCELLED));
                return true;
            }
            
            void __YYAPI WaitAsyncOperation::Reset()
            {
                hr = E_PENDING;
                eStatus = AsyncStatus::Started;
            }
} // namespace Threading
    }
} // namespace YY
