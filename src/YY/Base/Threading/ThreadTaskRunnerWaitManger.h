#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Threading/TaskRunner.h>
#include <YY/Base/Time/TickCount.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            /// <summary>
            /// 单线程的Wait Manger实现，最多等待63个句柄。
            /// </summary>
            class ThreadTaskRunnerWaitManger
            {
            protected:
                static constexpr auto kMaxWaitHandleCount = MAXIMUM_WAIT_OBJECTS - 1;
                HANDLE hWaitHandles[MAXIMUM_WAIT_OBJECTS] = {};
                WeakPtr<Wait> pWaitTaskWeakPtrs[MAXIMUM_WAIT_OBJECTS];
                volatile uint32_t cWaitHandle = 0;

                ~ThreadTaskRunnerWaitManger()
                {
                    for (auto& _oWaitTaskWeakPtr : pWaitTaskWeakPtrs)
                    {
                        if(auto _pTask = _oWaitTaskWeakPtr.Get())
                        {
                            _pTask->Cancel();
                        }
                    }
                }

                bool __YYAPI RemoveWaitHandleByIndex(size_t _uRemoveIndex) noexcept
                {
                    if (cWaitHandle >= _uRemoveIndex)
                        return false;

                    if (auto _pWaitTask = pWaitTaskWeakPtrs[_uRemoveIndex].Get())
                    {
                        _pWaitTask->hThreadPoolWait = NULL;
                    }

                    --cWaitHandle;
                    if (_uRemoveIndex == cWaitHandle)
                    {
                        hWaitHandles[_uRemoveIndex] = nullptr;
                        pWaitTaskWeakPtrs[_uRemoveIndex] = nullptr;
                    }
                    else
                    {
                        hWaitHandles[_uRemoveIndex] = hWaitHandles[cWaitHandle];
                        hWaitHandles[cWaitHandle] = nullptr;
                        pWaitTaskWeakPtrs[_uRemoveIndex] = pWaitTaskWeakPtrs[cWaitHandle];
                        pWaitTaskWeakPtrs[cWaitHandle] = nullptr;
                    }

                    return true;
                }

                TickCount __YYAPI GetMinimumWakeupTickCount() noexcept
                {
                    auto _uMinimumWakeupTickCount = TickCount::GetMax();
                    for (size_t i = cWaitHandle; i; )
                    {
                        --i;
                        auto _pWait = pWaitTaskWeakPtrs[i].Get();
                        if (_pWait == nullptr || _pWait->IsCanceled())
                        {
                            RemoveWaitHandleByIndex(i);
                            continue;
                        }

                        if (_uMinimumWakeupTickCount > _pWait->uTimeOut)
                        {
                            _uMinimumWakeupTickCount = _pWait->uTimeOut;
                        }
                    }

                    return _uMinimumWakeupTickCount;
                }

                size_t __YYAPI FindWaitHandle(HANDLE _hHandle) const noexcept
                {
                    if (!_hHandle)
                        return kMaxWaitHandleCount;

                    for (size_t i = 0; i != cWaitHandle; ++i)
                    {
                        if (hWaitHandles[i] == _hHandle)
                            return i;
                    }

                    return kMaxWaitHandleCount;
                }

                HRESULT __YYAPI SetWaitInternal(_In_ RefPtr<Wait> _pWaitTask) noexcept
                {
                    if (!_pWaitTask)
                        return E_INVALIDARG;

                    if (!_pWaitTask->hHandle)
                        return E_INVALIDARG;
                    
                    const auto _uIndex = FindWaitHandle(_pWaitTask->hHandle);
                    if (_uIndex != kMaxWaitHandleCount)
                    {
                        _pWaitTask->hThreadPoolWait = INVALID_HANDLE_VALUE;
                        if (auto _pOldWaitTask = pWaitTaskWeakPtrs[_uIndex].Get())
                        {
                            if (_pOldWaitTask == _pWaitTask)
                            {
                                return S_OK;
                            }

                            _pOldWaitTask->Cancel();
                        }

                        pWaitTaskWeakPtrs[_uIndex] = _pWaitTask;
                        return S_OK;
                    }

                    if (cWaitHandle >= kMaxWaitHandleCount)
                    {
                        // 尚未实现，需要派发到线程池句柄等待队列中
                        return E_NOTIMPL;
                    }

                    _pWaitTask->hThreadPoolWait = INVALID_HANDLE_VALUE;
                    hWaitHandles[cWaitHandle] = _pWaitTask->hHandle;
                    pWaitTaskWeakPtrs[cWaitHandle] = _pWaitTask;
                    ++cWaitHandle;
                    return S_OK;
                }

                size_t __YYAPI ProcessingWaitTasks(DWORD uWaitResult, DWORD _cWaitHandle, DWORD _uTaskRunnerServerHandleIndex) noexcept
                {
                    size_t _cTaskProcessed = 0;
                    if (WAIT_OBJECT_0 <= uWaitResult && uWaitResult < WAIT_OBJECT_0 + _cWaitHandle)
                    {
                        uWaitResult -= WAIT_OBJECT_0;
                        if (uWaitResult != _uTaskRunnerServerHandleIndex)
                        {
                            ProcessingWaitTask(uWaitResult, WAIT_OBJECT_0);
                            ++_cTaskProcessed;
                        }
                    }
                    else if (WAIT_ABANDONED_0 <= uWaitResult && uWaitResult < WAIT_ABANDONED_0 + _cWaitHandle)
                    {
                        uWaitResult -= WAIT_ABANDONED_0;
                        if (uWaitResult != _uTaskRunnerServerHandleIndex)
                        {
                            ProcessingWaitTask(uWaitResult, WAIT_ABANDONED_0);
                            ++_cTaskProcessed;
                        }
                    }
                    else if (WAIT_IO_COMPLETION == uWaitResult)
                    {
                        // APC被执行，不是存在信号
                    }
                    else if (WAIT_TIMEOUT == uWaitResult)
                    {
                        const auto _uCurrentTickCount = TickCount::GetNow();

                        for (size_t i = cWaitHandle; i;)
                        {
                            --i;
                            auto _pWaitTask = pWaitTaskWeakPtrs[i].Get();
                            if (!_pWaitTask)
                            {
                                RemoveWaitHandleByIndex(i);
                                ++_cTaskProcessed;
                                continue;
                            }

                            if ((_pWaitTask->uTimeOut - _uCurrentTickCount).GetTotalMilliseconds() > 0ll)
                                continue;

                            ProcessingWaitTask(i, WAIT_TIMEOUT);
                            ++_cTaskProcessed;
                        }
                    }
                    else/* if (WAIT_FAILED == uWaitResult)*/
                    {
                        // 有句柄可能无效，检测一下无效句柄……然后报告
                        // 这里需要从后向前移动，避免 DispatchWaitList 后 cWaitHandle不停的减小
                        for(; _cWaitHandle;)
                        {
                            --_cWaitHandle;
                            if (_cWaitHandle != _uTaskRunnerServerHandleIndex)
                            {
                                const auto _uTaskWaitResult = WaitForSingleObject(hWaitHandles[_cWaitHandle], 0);
                                if (_uTaskWaitResult != WAIT_TIMEOUT)
                                {
                                    ProcessingWaitTask(_cWaitHandle, _uTaskWaitResult);
                                    ++_cTaskProcessed;
                                }
                            }
                        }
                    }

                    return _cTaskProcessed;
                }

            private:
                virtual void __YYAPI DispatchWaitTask(RefPtr<Wait> _pWaitTask) = 0;

                void __YYAPI ProcessingWaitTask(size_t _uDispatchIndex, DWORD _uWaitResult) noexcept
                {
                    if (_uDispatchIndex >= cWaitHandle)
                        return;

                    auto _pWaitTask = pWaitTaskWeakPtrs[_uDispatchIndex].Get();
                    RemoveWaitHandleByIndex(_uDispatchIndex);
                    if (_pWaitTask)
                    {
                        _pWaitTask->uWaitResult = _uWaitResult;
                        DispatchWaitTask(std::move(_pWaitTask));
                    }
                }
            };
        }
    }
}
#pragma pack(pop)
