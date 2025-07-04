﻿#pragma once
#include <type_traits>
#include <functional>

#include <YY/Base/YY.h>
#include <YY/Base/Memory/RefPtr.h>
#include <YY/Base/Sync/Interlocked.h>
#include <YY/Base/Exception.h>
#include <YY/Base/ErrorCode.h>
#include <YY/Base/Memory/WeakPtr.h>
#include <YY/Base/Time/TickCount.h>
#include <YY/Base/Time/TimeSpan.h>
#include <YY/Base/Threading/Coroutine.h>
#include <YY/Base/Strings/String.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            enum class TaskRunnerStyle
            {
                None = 0,
                // 保证提交的任务串行
                Sequenced = 0x00000001,
                // 拥有固定线程，没有此标准表示实际指向物理线程可能随时变化。
                FixedThread = 0x00000002,
            };

            YY_APPLY_ENUM_CALSS_BIT_OPERATOR(TaskRunnerStyle);

            enum class TaskEntryStyle
            {
                None = 0,
                // 任务同步进行。
                Sync = 0x00000001,
                // 任务尝试进行取消
                Canceled = 0x00000004,
            };

            YY_APPLY_ENUM_CALSS_BIT_OPERATOR(TaskEntryStyle);

            class TaskRunner;

            struct TaskEntry : public RefValue
            {
                TaskEntryStyle fStyle = TaskEntryStyle::None;
                // 操作结果，任务可能被取消。
                HRESULT hr = E_PENDING;

                std::function<void(void)> pfnTaskCallback;

                WeakPtr<TaskRunner> pOwnerTaskRunnerWeak;

                // 这个任务完成后重新回到的 TaskRunner
                WeakPtr<TaskRunner> pResumeTaskRunnerWeak;

                TaskEntry() = default;

                TaskEntry(const TaskEntry&) = delete;
                TaskEntry& operator=(const TaskEntry&) = delete;

                void __YYAPI operator()()
                {
                    Wakeup(RunTask());
                }

                /// <summary>
                /// 设置错误代码，并唤醒相关等待者。
                /// </summary>
                /// <param name="_hrCode">任务退出代码。</param>
                /// <returns></returns>
                void __YYAPI Wakeup(_In_ HRESULT _hrCode);

                /// <summary>
                /// 等待此任务完成。
                /// </summary>
                /// <param name="_uMilliseconds">需要等待的毫秒数。</param>
                /// <returns></returns>
                bool __YYAPI Wait(_In_ uint32_t _uMilliseconds = UINT32_MAX);

                bool __YYAPI IsCanceled() const noexcept
                {
                    return HasFlags(fStyle, TaskEntryStyle::Canceled);
                }

                void __YYAPI Cancel()
                {
                    YY::Sync::BitSet((int32_t*)&fStyle, 2);
                }

                virtual HRESULT __YYAPI RunTask();
            };

            struct Timer : public TaskEntry
            {
                std::function<bool(void)> pfnTimerCallback;

                // 任务到期时间
                TickCount<TimePrecise::Microsecond> uExpire;

                // 任务重复间隔，如果为0，那么任务不会重复
                TimeSpan<TimePrecise::Millisecond> uInterval;

                Timer* pNext = nullptr;

                HRESULT __YYAPI RunTask() override;
            };

            struct Wait : public TaskEntry
            {
                HANDLE hHandle = nullptr;
                TickCount<TimePrecise::Microsecond> uTimeOut;
                DWORD uWaitResult = WAIT_FAILED;

                std::function<bool(DWORD _uWaitResult)> pfnWaitTaskCallback;
                Wait* pPrior = nullptr;
                Wait* pNext = nullptr;

                HRESULT __YYAPI RunTask() override;
            };

#if defined(_WIN32)
            struct IoTaskEntry
                : public TaskEntry
                , public OVERLAPPED

            {
                LSTATUS lStatus = ERROR_IO_PENDING;

                IoTaskEntry()
                    : TaskEntry()
                    , OVERLAPPED{}
                {
                }

                bool __YYAPI OnComplete(LSTATUS _lStatus)
                {
                    return Sync::CompareExchange(&lStatus, _lStatus, ERROR_IO_PENDING) == ERROR_IO_PENDING;
                }
            };
#endif

#if defined(_HAS_CXX20) && _HAS_CXX20
            template<typename ResumeType_>
            class TaskAwaiter
            {
            public:
                using ResumeType = ResumeType_;

                class RefData
                {
                public:
                    // 协程句柄
                    intptr_t hCoroutineHandle = 0;
                    
                    ~RefData()
                    {
                        if (hCoroutineHandle && hCoroutineHandle != (intptr_t)-1)
                        {
                            std::coroutine_handle<>::from_address((void*)hCoroutineHandle).destroy();
                        }
                    }

                    virtual uint32_t __YYAPI AddRef() noexcept = 0;

                    virtual uint32_t __YYAPI Release() noexcept = 0;

                    virtual ResumeType_ __YYAPI Resume() noexcept = 0;
                };

            private:
                RefPtr<RefData> pAwaiterData;

            public:
                TaskAwaiter(_In_ RefPtr<RefData> _pAwaiterData)
                    : pAwaiterData(std::move(_pAwaiterData))
                {
                }

                TaskAwaiter(TaskAwaiter&&) = default;

                TaskAwaiter(const TaskAwaiter&) = delete;
                TaskAwaiter& operator=(const TaskAwaiter&) = delete;

                bool await_ready() noexcept
                {
                    return pAwaiterData->hCoroutineHandle == /*hReadyHandle*/ (intptr_t)-1;
                }

                bool await_suspend(std::coroutine_handle<> _hHandle) noexcept
                {
                    return YY::Base::Sync::CompareExchange(&pAwaiterData->hCoroutineHandle, (intptr_t)_hHandle.address(), 0) == 0;
                }

                ResumeType await_resume() noexcept
                {
                    return pAwaiterData->Resume();
                }
            };
            
            template<>
            class TaskAwaiter<void>
            {
            public:
                using ResumeType = void;

                class RefData
                {
                public:
                    // 协程句柄
                    intptr_t hCoroutineHandle = 0;

                    ~RefData()
                    {
                        if (hCoroutineHandle && hCoroutineHandle != (intptr_t)-1)
                        {
                            std::coroutine_handle<>::from_address((void*)hCoroutineHandle).destroy();
                        }
                    }

                    virtual uint32_t __YYAPI AddRef() noexcept = 0;

                    virtual uint32_t __YYAPI Release() noexcept = 0;
                };

            private:
                RefPtr<RefData> pAwaiterData;

            public:
                TaskAwaiter(_In_ RefPtr<RefData> _pAwaiterData)
                    : pAwaiterData(std::move(_pAwaiterData))
                {
                }

                TaskAwaiter(TaskAwaiter&&) = default;

                TaskAwaiter(const TaskAwaiter&) = delete;
                TaskAwaiter& operator=(const TaskAwaiter&) = delete;
                
                TaskAwaiter& operator=(TaskAwaiter&&) = default;

                bool await_ready() noexcept
                {
                    return pAwaiterData->hCoroutineHandle == /*hReadyHandle*/ (intptr_t)-1;
                }

                bool await_suspend(std::coroutine_handle<> _hHandle) noexcept
                {
                    return YY::Base::Sync::CompareExchange(&pAwaiterData->hCoroutineHandle, (intptr_t)_hHandle.address(), 0) == 0;
                }

                void await_resume() noexcept
                {
                    // 因为没有返回值，所以空函数占坑
                }
            };
#endif // defined(_HAS_CXX20) && _HAS_CXX20

            class TaskRunnerDispatch;

            // 通用任务执行器的抽象层
            class TaskRunner : public RefValue
            {
                friend TaskRunnerDispatch;
                friend Timer;
                friend Wait;

            protected:
                uint32_t uTaskRunnerId;

                TaskRunner();

            public:

                /// <summary>
                /// 获取调用者的 TaskRunner
                /// </summary>
                /// <returns>
                /// 如果返回 nullptr，可能当前调用者是线程池，也可能来自外部线程。
                /// </returns>
                static RefPtr<TaskRunner> __YYAPI GetCurrent();

#if defined(_WIN32)
                /// <summary>
                /// 将文件句柄绑定到调度器内部的完成端口，便于调度器会将IO任务重新派发到指定TaskRunner。
                /// 
                /// 警告：调度器只能调度基于 IoTaskEntry 的任务！
                /// </summary>
                /// <param name="_hHandle"></param>
                /// <returns></returns>
                static bool __YYAPI BindIO(_In_ HANDLE _hHandle) noexcept;
#endif
                /// <summary>
                /// 发起异步请求成功后请调用此函数。内部将对完成端口进行监听。
                /// </summary>
                /// <returns></returns>
                static void __YYAPI StartIo() noexcept;

                /// <summary>
                /// 返回 TaskRunner 的唯一Id，注意，这不是线程Id。
                /// </summary>
                /// <returns></returns>
                uint32_t __YYAPI GetId() const noexcept
                {
                    return uTaskRunnerId;
                }

                virtual TaskRunnerStyle __YYAPI GetStyle() const noexcept = 0;

                /// <summary>
                /// 将任务异步执行。
                /// </summary>
                /// <param name="_pfnTaskCallback">需要异步执行回调。</param>
                /// <returns></returns>
                HRESULT __YYAPI PostDelayTask(
                    _In_ TimeSpan<TimePrecise::Millisecond> _uAfter,
                    _In_ std::function<void(void)>&& _pfnTaskCallback);

                /// <summary>
                /// 将任务异步执行。
                /// </summary>
                /// <param name="_pfnTaskCallback">需要异步执行回调。</param>
                /// <returns></returns>
                HRESULT __YYAPI PostTask(_In_ std::function<void(void)>&& _pfnTaskCallback);

#if defined(_HAS_CXX20) && _HAS_CXX20
                /// <summary>
                /// 创建一个异步可 co_await 任务。
                /// </summary>
                /// <param name="_uAfter">需要延迟执行的时间</param> 
                /// <param name="_pfnLambdaCallback">需要异步执行的 Lambda 表达式</param>
                /// <returns>TaskAwaiter<void></returns>
                TaskAwaiter<void> __YYAPI AsyncDelayTask(
                    _In_ TimeSpan<TimePrecise::Millisecond> _uAfter,
                    _In_ std::function<void(void)>&& _pfnTaskCallback);
#endif

#if defined(_HAS_CXX20) && _HAS_CXX20
                TaskAwaiter<void> __YYAPI AsyncTask(
                    _In_ std::function<void(void)>&& _pfnTaskCallback)
                {
                    return AsyncDelayTask(TimeSpan<TimePrecise::Millisecond>(), std::move(_pfnTaskCallback));
                }
#endif

                /// <summary>
                /// 同步执行Callback。严重警告：这可能阻塞调用者，甚至产生死锁！！！
                /// </summary>
                /// <param name="pfnTaskCallback"></param>
                /// <returns></returns>
                HRESULT __YYAPI SendTask(_In_ std::function<void(void)>&& pfnTaskCallback);

                /// <summary>
                /// 在TaskRunner中创建一个定时器。当时间到达时会在相关TaskRunner中执行 _pfnTaskCallback。
                /// 
                /// * 注意：受Windows系统影响，一般情况下定时器精度为16毫秒左右。
                /// </summary>
                /// <param name="_uInterval">需要定时触发任务的间隔。</param>
                /// <param name="_pfnTaskCallback">时间达到时将调用函数。
                /// 
                /// * 如果需要定时器继续排队，请返回 true。
                /// * 如果后续不在需要执行定时器，请返回 false。
                /// </param>
                /// <returns></returns>
                RefPtr<Timer> __YYAPI CreateTimer(_In_ TimeSpan<TimePrecise::Millisecond> _uInterval, _In_ std::function<bool(void)>&& _pfnTaskCallback);

                /// <summary>
                /// 监听指定句柄状态。如果句柄处于有信号状态则调用 _pfnTaskCallback。如果同一个句柄多次调用CreateWait，对应的_pfnTaskCallback也将多次调用。
                /// 
                /// 
                /// 注意：虽然可以无数量限制的等待句柄，但是数量越多开销可能越高。TaskRunner实现中，每个线程最多等待63个句柄。
                /// 
                /// 如果等待句柄数量小于等于 63，这时几乎没有额外开销，因为当前线程自身即可等待这些句柄。
                /// 如果等待句柄数量大于等于 64，从第64个句柄开始，剩余句柄将安排到单独的线程，每个线程最多等待63个句柄，因此等待的句柄数量越多，额外进行安排的线程也就越多。
                /// </summary>
                /// <param name="_hHandle">需要监听的句柄</param>
                /// <param name="_nWaitTimeOut">最大超时等待的时间。</param>
                /// <param name="_pfnTaskCallback">有信号时、等待失败，或者超时时将触发的回调函数。
                /// _uWaitResult 可以是WAIT_ABANDONED、WAIT_OBJECT_0、WAIT_TIMEOUT、WAIT_FAILED。
                /// 
                /// * 如果需要继续等待句柄信号，请返回 true。
                /// * 如果句柄只等待一次，请返回 false。
                /// </param>
                /// <returns>
                /// 返回 Wait对象，Wait对象可以取消任务。
                /// 如果函数返回 nullptr，那么可能是句柄无效、_pfnTaskCallback无效亦或者内存不足。
                /// </returns>
                RefPtr<Wait> __YYAPI CreateWait(
                    _In_ HANDLE _hHandle,
                    _In_ TimeSpan<TimePrecise::Millisecond> _nWaitTimeOut,
                    _In_ std::function<bool(DWORD _uWaitResult)>&& _pfnTaskCallback);

                RefPtr<Wait> __YYAPI CreateWait(
                    _In_ HANDLE _hHandle,
                    _In_ std::function<bool(DWORD _uWaitResult)>&& _pfnTaskCallback)
                {
                    return CreateWait(_hHandle, TimeSpan<TimePrecise::Millisecond>::GetMax(), std::move(_pfnTaskCallback));
                }

                virtual HRESULT __YYAPI Join(_In_ TimeSpan<TimePrecise::Millisecond> _nWaitTimeOut = TimeSpan<TimePrecise::Millisecond>::GetMax()) noexcept = 0;

                virtual HRESULT __YYAPI Interrupt() noexcept = 0;

            protected:
                virtual HRESULT __YYAPI PostTaskInternal(_In_ RefPtr<TaskEntry> _pTask) = 0;

                virtual HRESULT __YYAPI SetTimerInternal(_In_ RefPtr<Timer> _pTask);

                virtual HRESULT __YYAPI SetWaitInternal(_In_ RefPtr<Wait> _pTask);
            };

            // 按顺序执行的Task（不一定绑定固定物理线程，只保证任务串行）
            class SequencedTaskRunner : public TaskRunner
            {
            public:
                /// <summary>
                /// 获取当前调用所属的 SequencedTaskRunner。
                /// </summary>
                /// <returns>
                /// 如果返回 nullptr，可能当前不是 SequencedTaskRunner，来自线程池，也可能来自外部创建的线程，不属于任何一个 TaskRunner。
                /// </returns>
                static RefPtr<SequencedTaskRunner> __YYAPI GetCurrent();

                /// <summary>
                /// 从线程池创建一个TaskRunner，后续PostTask提交后将保持串行。注意：不保证保证是否同一个线程，仅保证任务串行！
                /// </summary>
                /// <param name="_szThreadDescription">线程描述。对于Windows平台，该信息设置后调试器可直接从线程查看此信息。</param>
                /// <returns>返回TaskRunner指针，函数几乎不会失败，但是如果内存不足，那么将返回 nullptr。</returns>
                static RefPtr<SequencedTaskRunner> __YYAPI Create(uString _szThreadDescription = uString());

            };

            // 任务串行并且拥有固定线程的任务执行器
            class ThreadTaskRunner : public SequencedTaskRunner
            {
            public:
                /// <summary>
                /// 从线程池取一个线程并且与该TaskRunner绑定，始终保证后续任务在同一个线程中执行。
                /// 如果ThreadTaskRunner生命周期解除，则将线程归还线程池。
                /// 温馨提示：优先考虑使用 `SequencedTaskRunner::Create()`，ThreadTaskRunner的创建开销更为高昂。
                /// </summary>
                /// <param name="_bBackgroundLoop"></param>
                /// <param name="_szThreadDescription">线程描述。对于Windows平台，该信息设置后调试器可直接从线程查看此信息。</param>
                /// <returns></returns>
                static RefPtr<ThreadTaskRunner> __YYAPI Create(_In_ bool _bBackgroundLoop = true, uString _szThreadDescription = uString());

                /// <summary>
                /// 获取当前线程绑定的TaskRunner。
                /// </summary>
                /// <returns>
                /// 如果在返回 nullptr，该线程可能属于 SequencedTaskRunner，没有固定物理线程。也可能 RunUIMessageLoop 尚未调用。
                /// </returns>
                static RefPtr<ThreadTaskRunner> __YYAPI GetCurrent();

                /// <summary>
                /// 获取绑定的线程Id。
                /// </summary>
                /// <returns></returns>
                virtual uint32_t __YYAPI GetThreadId() = 0;

                /// <summary>
                /// 将ThreadTaskRunner与当前物理线程绑定。便于其他线程向该线程投递任务。成功后需要调用RunUIMessageLoop进行任务调度。
                ///
                /// 一般来说，此函数是给主线程使用的。如果函数返回成功，那么必须持续持有该TaskRunner！因为一旦释放并且引用计数归0，将解除绑定关系！
                /// </summary>
                /// <returns>如果调用者线程不是主线程，该函数可能返回nullptr。</returns>
                static RefPtr<ThreadTaskRunner> __YYAPI BindCurrentThread();

                /// <summary>
                /// 使用代理模式，将ThreadTaskRunner与当前物理线程绑定。便于其他线程向该线程投递任务，以及保证 ThreadTaskRunner::GetCurrent正常使用。
                ///
                /// 温馨提示：
                ///   * 此函数一般由主线程使用的。如果函数返回成功，那么必须持续持有该TaskRunner！因为一旦释放并且引用计数归0，将解除绑定关系！
                ///   * 非主线程想创建新建的ThreadTaskRunner，首选考虑 ThreadTaskRunner::Create
                ///   * 代理模式需要额外创建一个窗口用于调度任务，这存在额外资源开销，请考虑优先使用BindCurrentThread，已获得更佳性能。
                ///   * 代理模式无法调用 `ThreadTaskRunner::RunUIMessageLoop`，因此当前线程必须自己拥有消息循环！
                /// </summary>
                /// <returns>如果调用者线程已经绑定或者存在对应的TaskRUnner，该函数将返回nullptr。</returns>
                static RefPtr<ThreadTaskRunner> __YYAPI BindCurrentThreadForProxyMode();

                /// <summary>
                /// 运行UI线程专属消息循环。
                /// * 主线程运行消息循环后才能正常使用 ThreadTaskRunner::GetCurrent();
                /// * 如果 RunUIMessageLoop 退出，后续的PostTask等请求将失败。
                /// </summary>
                /// <param name="_pfnCallback">启动循环之前进行的函数调用，Callback发生期间可以使用`ThreadTaskRunner::GetCurrent()`。</param>
                /// <param name="_pUserData">后续传递给 _pfnCallback 的 _pUserData</param>
                /// <returns>消息循环退出代码。</returns>
                static uintptr_t __YYAPI RunUIMessageLoop();
            };

            // 自动将任务并行处理且负载均衡
            class ParallelTaskRunner : public TaskRunner
            {
            protected:
                // 允许并行执行的最大个数
                // 如果为 0，则表示跟随系统物理线程数
                volatile uint32_t uParallelMaximum;

                ParallelTaskRunner(uint32_t _uParallelMaximum)
                    : uParallelMaximum(_uParallelMaximum)
                {
                }

            public:
                static RefPtr<ParallelTaskRunner> __YYAPI GetCurrent() noexcept;

                /// <summary>
                /// 从线程池创建一个TaskRunner，后续PostTask提交发任务可能多个物理线程并行执行。最大的并行数量主要受系统资源以及 _uParallelMaximum 参数控制。
                /// 
                /// 注意：不保证保证任务始终在同一组物理线程执行，ParallelTaskRunner仅保证最大并行数不超过 _uParallelMaximum。
                /// </summary>
                /// <param name="_uParallelMaximum">最大允许的物理线程并发数，如果此参数为0，则按系统CPU逻辑线程数并行。</param>
                /// <param name="_szThreadDescription">线程描述。对于Windows平台，该信息设置后调试器可直接从线程查看此信息。</param>
                /// <returns></returns>
                static RefPtr<ParallelTaskRunner> __YYAPI Create(uint32_t _uParallelMaximum = 0u, uString _szThreadDescription = uString()) noexcept;

                uint32_t __YYAPI GetParallelMaximum() const noexcept
                {
                    return uParallelMaximum;
                }

                void __YYAPI SetParallelMaximum(uint32_t _uParallelMaximum) noexcept
                {
                    uParallelMaximum = _uParallelMaximum;
                }
            };
        }
    }
} // namespace YY::Base::Threading;

namespace YY
{
    using namespace YY::Base::Threading;
}

#pragma pack(pop)
