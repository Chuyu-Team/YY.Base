﻿#pragma once

#include <YY/Base/Sync/InterlockedQueue.h>
#include <YY/Base/Memory/UniquePtr.h>
#include <YY/Base/Sync/Sync.h>

#include "TaskRunnerImpl.h"

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            inline uint32_t __YYAPI BitsCount32(uint32_t _fBitMask)
            {
                _fBitMask = (_fBitMask & 0x55555555) + ((_fBitMask >> 1) & 0x55555555);
                _fBitMask = (_fBitMask & 0x33333333) + ((_fBitMask >> 2) & 0x33333333);
                _fBitMask = (_fBitMask & 0x0f0f0f0f) + ((_fBitMask >> 4) & 0x0f0f0f0f);
                _fBitMask = (_fBitMask & 0x00ff00ff) + ((_fBitMask >> 8) & 0x00ff00ff);
                _fBitMask = (_fBitMask & 0x0000ffff) + ((_fBitMask >> 16) & 0x0000ffff);
                return _fBitMask;
            }

            inline uint32_t __YYAPI BitsCount64(uint64_t _fBitMask)
            {
                _fBitMask = (_fBitMask & 0x55555555'55555555) + ((_fBitMask >> 1) & 0x55555555'55555555);
                _fBitMask = (_fBitMask & 0x33333333'33333333) + ((_fBitMask >> 2) & 0x33333333'33333333);
                _fBitMask = (_fBitMask & 0x0f0f0f0f'0f0f0f0f) + ((_fBitMask >> 4) & 0x0f0f0f0f'0f0f0f0f);
                _fBitMask = (_fBitMask & 0x00ff00ff'00ff00ff) + ((_fBitMask >> 8) & 0x00ff00ff'00ff00ff);
                _fBitMask = (_fBitMask & 0x0000ffff'0000ffff) + ((_fBitMask >> 16) & 0x0000ffff'0000ffff);
                _fBitMask = (_fBitMask & 0x00000000'ffffffff) + ((_fBitMask >> 32) & 0x00000000'ffffffff);
                return static_cast<uint32_t>(_fBitMask);
            }

#ifdef _WIN32
#ifdef _X86_
#define BitsCount32(_fBitMask) static_cast<uint32_t>(__popcnt(_fBitMask))
#elif defined(_AMD64_)
#define BitsCount64(_fBitMask) static_cast<uint32_t>(__popcnt64(_fBitMask))
#endif
#endif

            inline uint32_t __YYAPI BitsCount(uint32_t _fBitMask)
            {
                return BitsCount32(_fBitMask);
            }

            inline uint32_t __YYAPI BitsCount(uint64_t _fBitMask)
            {
                return BitsCount64(_fBitMask);
            }

#ifdef _WIN32
            static uint32_t __YYAPI GetLogicalProcessorCount() noexcept
            {
                static uint32_t s_LogicalProcessorCount = 0;
                if (s_LogicalProcessorCount != 0)
                {
                    return s_LogicalProcessorCount;
                }
                uint32_t _uLogicalProcessorCount = 0;

                do
                {
                    DWORD _cbData = 0;
                    if (!GetLogicalProcessorInformationEx(LOGICAL_PROCESSOR_RELATIONSHIP::RelationProcessorPackage, nullptr, &_cbData))
                    {
                        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                        {
                            break;
                        }
                    }

                    UniquePtr<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX> _pBuffer((SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)Alloc(_cbData));
                    if (!_pBuffer)
                        break;

                    if (!GetLogicalProcessorInformationEx(LOGICAL_PROCESSOR_RELATIONSHIP::RelationProcessorPackage, _pBuffer, &_cbData))
                        break;

                    for (size_t i = 0; i != _pBuffer->Processor.GroupCount; ++i)
                    {
                        _uLogicalProcessorCount += BitsCount((uintptr_t)_pBuffer->Processor.GroupMask[i].Mask);
                    }

                } while (false);


                if (_uLogicalProcessorCount == 0)
                {
                    SYSTEM_INFO _oSystemInfo;
                    GetNativeSystemInfo(&_oSystemInfo);

                    _uLogicalProcessorCount = _oSystemInfo.dwNumberOfProcessors;
                }

                if (_uLogicalProcessorCount == 0)
                    _uLogicalProcessorCount = 1;

                // 防止意外的逻辑处理器数量发送变化，造成潜在的风险，特意使用原子操作
                auto _uLast = Sync::CompareExchange(&s_LogicalProcessorCount, _uLogicalProcessorCount, 0);
                return _uLast ? _uLast : _uLogicalProcessorCount;
            }
#else
            static uint32_t __YYAPI GetLogicalProcessorCount() noexcept
            {
                static uint32_t s_LogicalProcessorCount = 0;
                if (s_LogicalProcessorCount != 0)
                {
                    return s_LogicalProcessorCount;
                }

                const auto _uCount = get_nprocs();
                if (_uCount)
                {
                    s_LogicalProcessorCount = _uCount;
                }
                else
                {
                    s_LogicalProcessorCount = 1;
                }
                return s_LogicalProcessorCount;
            }
#endif

            class ParallelTaskRunnerImpl : public ParallelTaskRunner
            {
            public:
                InterlockedQueue<TaskEntry> oTaskQueue;

                // |uWeakCount| bPushLock | bStopWakeup | bPushLock |
                // | 31  ~  3 |    2      |     1       |    0      |
                union TaskRunnerFlagsType
                {
                    volatile uint64_t fFlags64;
                    volatile uint32_t uWakeupCountAndPushLock;

                    struct
                    {
                        volatile uint32_t bPushLock : 1;
                        volatile uint32_t bStopWakeup : 1;
                        volatile uint32_t bInterrupt : 1;
                        volatile uint32_t bPopLock : 1;
                        int32_t uWakeupCount : 28;
                        // 当前已经启动的线程数
                        uint32_t uParallelCurrent;
                    };
                };

                TaskRunnerFlagsType TaskRunnerFlags;

                static_assert(sizeof(TaskRunnerFlags) == sizeof(uint64_t), "");

                enum : uint32_t
                {
                    LockedQueuePushBitIndex = 0,
                    StopWakeupBitIndex,
                    InterruptBitIndex,
                    LockedQueuePopBitIndex,
                    WakeupCountStartBitIndex,
                    StopWakeupRaw = 1 << StopWakeupBitIndex,
                    InterruptRaw = 1 << InterruptBitIndex,
                    WakeupOnceRaw = 1 << WakeupCountStartBitIndex,
                    UnlockQueuePushLockBitAndWakeupOnceRaw = WakeupOnceRaw - (1u << LockedQueuePushBitIndex),
                    TerminateTaskRunnerRaw = StopWakeupRaw | InterruptRaw,
                };

                uString szThreadDescription;

                ParallelTaskRunnerImpl(uint32_t _uParallelMaximum, uString _szThreadDescription)
                    : ParallelTaskRunner(_uParallelMaximum)
                    , TaskRunnerFlags{ 0u }
                    , szThreadDescription(std::move(_szThreadDescription))
                {
                }

                ~ParallelTaskRunnerImpl() override
                {
                    CleanupTaskQueue();
                }

                /////////////////////////////////////////////////////
                // TaskRunner
                TaskRunnerStyle __YYAPI GetStyle() const noexcept override
                {
                    return TaskRunnerStyle::None;
                }

                HRESULT __YYAPI Join(TimeSpan<TimePrecise::Millisecond> _nWaitTimeOut) noexcept override
                {
                    if (GetCurrent() == this)
                    {
                        // 自己怎么Join自己？
                        return E_UNEXPECTED;
                    }

                    const auto _uWakeupCountAndPushLock = Sync::BitOr(&TaskRunnerFlags.uWakeupCountAndPushLock, StopWakeupRaw);
                    if (_uWakeupCountAndPushLock < WakeupOnceRaw)
                    {
                        return S_OK;
                    }

                    uint32_t _uTargetValye = TerminateTaskRunnerRaw;
                    static_assert(sizeof(TaskRunnerFlags.uWakeupCountAndPushLock) == sizeof(_uTargetValye), "");
                    if (!WaitEqualOnAddress(&TaskRunnerFlags.uWakeupCountAndPushLock, &_uTargetValye, sizeof(_uTargetValye), _nWaitTimeOut))
                    {
                        return __HRESULT_FROM_WIN32(ERROR_TIMEOUT);
                    }

                    return S_OK;
                }

                HRESULT __YYAPI Interrupt() noexcept override
                {
                    Sync::BitOr(&TaskRunnerFlags.uWakeupCountAndPushLock, StopWakeupRaw | InterruptRaw);
                    return S_OK;
                }

                HRESULT __YYAPI PostTaskInternal(_In_ RefPtr<TaskEntry> _pTask) override
                {
                    _pTask->hr = E_PENDING;

                    if (TaskRunnerFlags.bStopWakeup)
                    {
                        return YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED);
                    }

                    const auto _uParallelMaximum = uParallelMaximum ? uParallelMaximum : GetLogicalProcessorCount();

                    // 解除锁定，并且 WeakupCount + 1，也尝试提升 uParallelCurrent
                    TaskRunnerFlagsType _uOldFlags = TaskRunnerFlags;
                    TaskRunnerFlagsType _uNewFlags;
                    for (;;)
                    {
                        if (_uOldFlags.uWakeupCountAndPushLock & (1 << LockedQueuePushBitIndex))
                        {
                            YieldProcessor();
                            _uOldFlags.fFlags64 = TaskRunnerFlags.fFlags64;
                            continue;
                        }

                        _uNewFlags = _uOldFlags;
                        _uNewFlags.uWakeupCountAndPushLock += WakeupOnceRaw | (1 << LockedQueuePushBitIndex);

                        if (_uNewFlags.uParallelCurrent < _uParallelMaximum && _uNewFlags.uWakeupCount >= (int32_t)_uNewFlags.uParallelCurrent)
                        {
                            _uNewFlags.uParallelCurrent += 1;
                        }

                        auto _uLast = Sync::CompareExchange(&TaskRunnerFlags.fFlags64, _uNewFlags.fFlags64, _uOldFlags.fFlags64);
                        if (_uLast == _uOldFlags.fFlags64)
                        {
                            oTaskQueue.Push(_pTask.Detach());
                            Sync::BitReset(&TaskRunnerFlags.uWakeupCountAndPushLock, LockedQueuePushBitIndex);
                            break;
                        }

                        _uOldFlags.fFlags64 = _uLast;
                    }

                    // 并发送数量没有提升，所以无需从线程池拉新的线程
                    if (_uOldFlags.uParallelCurrent == _uNewFlags.uParallelCurrent)
                        return S_OK;

                    // 如果是第一次那么再额外 AddRef，避免ThreadPool::PostTaskInternalWithoutAddRef回调函数调用时 TaskRunner 被释放了
                    if (_uOldFlags.uParallelCurrent == 0u)
                    {
                        AddRef();
                    }
                    auto _hr = ThreadPool::PostTaskInternalWithoutAddRef(this);
                    if (FAILED(_hr))
                    {
                        // 阻止后续再唤醒线程
                        Sync::BitSet(&TaskRunnerFlags.uWakeupCountAndPushLock, StopWakeupBitIndex);
                        if (Sync::Decrement(&TaskRunnerFlags.uParallelCurrent) == 0u)
                        {
                            // 对应上面 if (_uOldFlags.uParallelCurrent == 0u) AddRef();
                            Release();
                        }
                        return _hr;
                    }

                    return S_OK;
                }
                //
                /////////////////////////////////////////////////////


                RefPtr<TaskEntry> PopTask() noexcept
                {
                    RefPtr<TaskEntry> _pTmp;
                    for (;;)
                    {
                        if (!Sync::BitSet(&TaskRunnerFlags.uWakeupCountAndPushLock, LockedQueuePopBitIndex))
                        {
                            _pTmp.Attach(oTaskQueue.Pop());
                            Sync::BitReset(&TaskRunnerFlags.uWakeupCountAndPushLock, LockedQueuePopBitIndex);
                            break;
                        }
                    }
                    return _pTmp;
                }

                void __YYAPI operator()()
                {
#if defined(_WIN32)
                    if(szThreadDescription.GetSize())
                        SetThreadDescription(GetCurrentThread(), szThreadDescription);
#endif

                    ExecuteTaskRunner();

                    if (IsShared() == false
                        || TaskRunnerFlags.bInterrupt
                        || (TaskRunnerFlags.bStopWakeup && TaskRunnerFlags.uWakeupCount == 0))
                    {
                        CleanupTaskQueue();
                    }
#if defined(_WIN32)
                    if (szThreadDescription.GetSize())
                        SetThreadDescription(GetCurrentThread(), _S(""));
#endif
                }

                void __YYAPI ExecuteTaskRunner()
                {
                    g_pTaskRunnerWeak = this;
                    bool _bReleaseParallelCurrent = true;
                    for (;;)
                    {
                        // 理论上 ExecuteTaskRunner 执行时引用计数 = 2，因为执行器拥有一次引用计数
                        // 如果为 1 (IsShared() == false)，那么说明用户已经释放了这个 TaskRunner
                        // 这时我们需要及时的退出，随后会将队列里的任务全部取消释放内存。
                        if (!IsShared())
                            break;

                        if (TaskRunnerFlags.bInterrupt)
                            break;

                        auto _pTask = PopTask();
                        if (!_pTask)
                            break;

                        _pTask->operator()();
                        _pTask.Reset();
                        if (!SubtractWakeupOnce())
                        {
                            _bReleaseParallelCurrent = false;
                            break;
                        }
                    }

                    if (_bReleaseParallelCurrent)
                        Sync::Decrement(&TaskRunnerFlags.uParallelCurrent);

                    g_pTaskRunnerWeak = nullptr;
                    return;
                }

                bool SubtractWakeupOnce()
                {
                    bool _bEixt = false;
                    TaskRunnerFlagsType _uOldFlags = TaskRunnerFlags;
                    TaskRunnerFlagsType _uNewFlags;
                    for (;;)
                    {
                        _uNewFlags = _uOldFlags;
                        _uNewFlags.uWakeupCountAndPushLock -= WakeupOnceRaw;
                        _bEixt = _uNewFlags.uWakeupCount < (int32_t)_uNewFlags.uParallelCurrent;
                        if (_bEixt)
                            _uNewFlags.uParallelCurrent -= 1;

                        auto _uLast = Sync::CompareExchange(&TaskRunnerFlags.fFlags64, _uNewFlags.fFlags64, _uOldFlags.fFlags64);
                        if (_uLast == _uOldFlags.fFlags64)
                            break;

                        _uOldFlags.fFlags64 = _uLast;
                    }

                    if (_uNewFlags.uParallelCurrent == 0u)
                    {
                        // 对应上面 ThreadPool::PostTaskInternalWithoutAddRef 的AddRef();
                        Release();
                    }

                    return !_bEixt;
                }

                void __YYAPI CleanupTaskQueue() noexcept
                {
                    if (!Sync::BitSet(&TaskRunnerFlags.uWakeupCountAndPushLock, LockedQueuePopBitIndex))
                    {
                        for (;;)
                        {
                            auto _pTask = RefPtr<TaskEntry>::FromPtr(oTaskQueue.Pop());
                            if (!_pTask)
                                break;

                            _pTask->Wakeup(YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED));
                        }

                        Sync::Exchange(&TaskRunnerFlags.uWakeupCountAndPushLock, TerminateTaskRunnerRaw);
                        WakeByAddressAll((PVOID)&TaskRunnerFlags.uWakeupCountAndPushLock);
                    }
                    return;
                }
            };
        }
    }
}

#pragma pack(pop)
