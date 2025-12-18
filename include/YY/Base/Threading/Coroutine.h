#pragma once
#include <YY/Base/YY.h>

#if defined(_HAS_CXX20) && _HAS_CXX20
#include <coroutine>

#include <YY/Base/Containers/Optional.h>
#include <YY/Base/Sync/Interlocked.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            template<typename ReturnType_, typename TaskType>
            struct Promise
            {
                using ReturnType = ReturnType_;
                YY::Optional<ReturnType> oValue;

                TaskType get_return_object() noexcept
                {
                    return {};
                }
                std::suspend_never initial_suspend() noexcept
                {
                    return {};
                }

                std::suspend_never final_suspend() noexcept
                {
                    return {};
                }

                void return_value(ReturnType&& _oValue) noexcept
                {
                    oValue.Emplace(std::move(_oValue));
                }

                void unhandled_exception()
                {
                }
            };

            template<typename TaskType>
            struct Promise<void, TaskType>
            {
                using ReturnType = void;

                TaskType get_return_object() noexcept
                {
                    return {};
                }
                std::suspend_never initial_suspend() noexcept
                {
                    return {};
                }

                std::suspend_never final_suspend() noexcept
                {
                    return {};
                }

                void return_void() noexcept
                {
                }
                void unhandled_exception()
                {
                }
            };


            template<typename _ReturnType>
            struct Coroutine
            {
                using promise_type = Promise<_ReturnType, Coroutine>;
            };

            template<typename ResumeType_>
            class TaskAwaiter;

            template<>
            class TaskAwaiter<void>
            {
            public:
                using ResumeType = void;

                class RefData
                {
                public:
                    intptr_t iReserved = 0;

                    struct CoroutineInfo
                    {
                        // 0 hCoroutineHandle
                        // 1 pOwner
                        intptr_t eType = 0;

                        union
                        {
                            // 协程句柄
                            volatile intptr_t hCoroutineHandle = 0;

                            // 协程句柄Owner
                            RefData* pOwner;
                        };

                        bool __YYAPI IsReady() const noexcept
                        {
                            return hCoroutineHandle == (intptr_t)-1;
                        }

                        CoroutineInfo __YYAPI Flush()
                        {
                            CoroutineInfo _oNewInfo = { 0, intptr_t(-1) };
                            CoroutineInfo _oOldInfo = *this;

                            for (;;)
                            {
                                if (YY::Sync::CompareExchangeTwoPoints(
                                    (volatile intptr_t*)this,
                                    (intptr_t*)&_oNewInfo,
                                    (intptr_t*)&_oOldInfo))
                                {
                                    break;
                                }

                                if (_oOldInfo.hCoroutineHandle == intptr_t(-1))
                                {
                                    break;
                                }

                                YieldProcessor();
                            }

                            return _oOldInfo;
                        }

                        void __YYAPI Destroy()
                        {
                            CoroutineInfo _oCoroutineInfo = Flush();
                            if (_oCoroutineInfo.hCoroutineHandle == 0 || _oCoroutineInfo.hCoroutineHandle == (intptr_t)-1)
                            {
                                return;
                            }

                            if (_oCoroutineInfo.eType == 0)
                            {
                                // 协程句柄
                                std::coroutine_handle<>::from_address((void*)_oCoroutineInfo.hCoroutineHandle).destroy();

                            }
                            else if (_oCoroutineInfo.eType == 1)
                            {
                                // Owner
                                _oCoroutineInfo.pOwner->Release();
                            }
                        }

                        HRESULT __YYAPI Resume()
                        {
                            CoroutineInfo _oCoroutineInfo = Flush();

                            if (_oCoroutineInfo.hCoroutineHandle == 0 || _oCoroutineInfo.hCoroutineHandle == (intptr_t)-1)
                            {
                                return S_OK;
                            }

                            if (_oCoroutineInfo.eType == 0)
                            {
                                // 协程句柄
                                try
                                {
                                    std::coroutine_handle<>::from_address((void*)_oCoroutineInfo.hCoroutineHandle).resume();
                                }
                                catch (const YY::Base::OperationCanceledException& _Exception)
                                {
                                    return YY::Base::HRESULT_From_LSTATUS(ERROR_CANCELLED);
                                }
                            }
                            else if (_oCoroutineInfo.eType == 1)
                            {
                                // Owner
                                auto _pOwner = YY::RefPtr<RefData>::FromPtr(_oCoroutineInfo.pOwner);
                                _pOwner->Resume();
                            }

                            return S_OK;
                        }

                        bool __YYAPI Suspend(const CoroutineInfo& _oNewCoroutineInfo)
                        {
                            CoroutineInfo _oCoroutineInfo = { 0, intptr_t(0) };

                            for (;;)
                            {
                                if (YY::Sync::CompareExchangeTwoPoints(
                                    (volatile intptr_t*)this,
                                    (intptr_t*)&_oNewCoroutineInfo,
                                    (intptr_t*)&_oCoroutineInfo))
                                {
                                    break;
                                }

                                if (_oCoroutineInfo.hCoroutineHandle)
                                {
                                    return false;
                                }

                                YieldProcessor();
                            }

                            return true;
                        }
                    };

                    static_assert(sizeof(CoroutineInfo) == sizeof(intptr_t) * 2, "");

                    // 必须对齐到8字节，所以添加iReserved占坑
                    CoroutineInfo oCoroutineInfo;

                    ~RefData()
                    {
                        oCoroutineInfo.Destroy();
                    }

                    virtual uint32_t __YYAPI AddRef() noexcept = 0;

                    virtual uint32_t __YYAPI Release() noexcept = 0;

                    virtual HRESULT __YYAPI Resume()
                    {
                        return oCoroutineInfo.Resume();
                    }

                    virtual bool __YYAPI IsReady() noexcept
                    {
                        return oCoroutineInfo.IsReady();
                    }

                    virtual bool __YYAPI Suspend(std::coroutine_handle<> _hHandle) noexcept
                    {
                        if (!_hHandle)
                            return false;

                        CoroutineInfo _oNewCoroutineInfo = { 0, intptr_t(_hHandle.address()) };
                        return oCoroutineInfo.Suspend(_oNewCoroutineInfo);
                    }

                    virtual bool __YYAPI Suspend(RefData* _pCoroutineHandleOwner) noexcept
                    {
                        if (!_pCoroutineHandleOwner)
                            return false;

                        _pCoroutineHandleOwner->AddRef();
                        CoroutineInfo _oNewCoroutineInfo = { 1, intptr_t(_pCoroutineHandleOwner) };
                        if (oCoroutineInfo.Suspend(_oNewCoroutineInfo))
                            return true;

                        _pCoroutineHandleOwner->Release();
                        return false;
                    }

                    virtual bool __YYAPI UndoSuspend(RefData* _pCoroutineHandleOwner) noexcept
                    {
                        if (!_pCoroutineHandleOwner)
                            return false;

                        CoroutineInfo _oNewCoroutineInfo = { 0, 0 };
                        CoroutineInfo _oPreCoroutineInfo = { 1, intptr_t(_pCoroutineHandleOwner) };

                        if (!YY::Sync::CompareExchangeTwoPoints(
                            (volatile intptr_t*)&oCoroutineInfo,
                            (intptr_t*)&_oNewCoroutineInfo,
                            (intptr_t*)&_oPreCoroutineInfo))
                        {
                            return false;
                        }

                        _pCoroutineHandleOwner->Release();
                        return true;
                    }
                };

            protected:
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

                RefPtr<RefData> GetAwaiterData() const noexcept
                {
                    return pAwaiterData;
                }

                bool await_ready() noexcept
                {
                    return pAwaiterData->IsReady();
                }

                bool await_suspend(std::coroutine_handle<> _hHandle) noexcept
                {
                    return pAwaiterData->Suspend(_hHandle);
                }

                bool await_suspend(RefData* _pCoroutineHandleOwner) noexcept
                {
                    return pAwaiterData->Suspend(_pCoroutineHandleOwner);
                }

                bool await_undo_suspend(RefData* _pCoroutineHandleOwner) noexcept
                {
                    return pAwaiterData->UndoSuspend(_pCoroutineHandleOwner);
                }

                void await_resume() noexcept
                {
                }
            };

            template<typename ResumeType_>
            class TaskAwaiter
            {
            public:
                using ResumeType = ResumeType_;

                class RefData : public TaskAwaiter<void>::RefData
                {
                public:
                    virtual ResumeType_ __YYAPI GetResult() noexcept = 0;
                };

            protected:
                RefPtr<RefData> pAwaiterData;

            public:
                TaskAwaiter(_In_ RefPtr<RefData> _pAwaiterData)
                    : pAwaiterData(std::move(_pAwaiterData))
                {
                }

                TaskAwaiter(TaskAwaiter&&) = default;

                TaskAwaiter(const TaskAwaiter&) = delete;
                TaskAwaiter& operator=(const TaskAwaiter&) = delete;

                RefPtr<TaskAwaiter<void>::RefData> GetAwaiterData() const noexcept
                {
                    return pAwaiterData;
                }

                bool await_ready() noexcept
                {
                    return pAwaiterData->IsReady();
                }

                bool await_suspend(std::coroutine_handle<> _hHandle) noexcept
                {
                    return pAwaiterData->Suspend(_hHandle);
                }

                bool await_suspend(TaskAwaiter<void>::RefData* _pCoroutineHandleOwner) noexcept
                {
                    return pAwaiterData->Suspend(_pCoroutineHandleOwner);
                }

                bool await_undo_suspend(TaskAwaiter<void>::RefData* _pCoroutineHandleOwner) noexcept
                {
                    return pAwaiterData->UndoSuspend(_pCoroutineHandleOwner);
                }

                ResumeType await_resume() noexcept
                {
                    return pAwaiterData->GetResult();
                }
            };

            template<typename ResultType>
            class AsyncResult
                : public YY::RefValue
                , public TaskAwaiter<ResultType>::RefData
            {
            private:
                YY::Optional<ResultType> oResultOptional;

            public:
                uint32_t __YYAPI AddRef() noexcept override
                {
                    return RefValue::AddRef();
                }

                uint32_t __YYAPI Release() noexcept override
                {
                    return RefValue::Release();
                }

                HRESULT __YYAPI Resolve(ResultType&& _Result) noexcept
                {
                    auto _oCoroutineInfo = this->oCoroutineInfo.Flush();

                    if (_oCoroutineInfo.hCoroutineHandle == (intptr_t)-1)
                    {
                        return S_FALSE;
                    }

                    oResultOptional.Emplace(std::move(_Result));
                    return _oCoroutineInfo.Resume();
                }

            protected:
                ResultType __YYAPI GetResult() noexcept override
                {
                    return oResultOptional.GetValue();
                }
            };

            template<>
            class AsyncResult<void>
                : public YY::RefValue
                , public TaskAwaiter<void>::RefData
            {
            public:
                uint32_t __YYAPI AddRef() noexcept override
                {
                    return RefValue::AddRef();
                }

                uint32_t __YYAPI Release() noexcept override
                {
                    return RefValue::Release();
                }

                HRESULT __YYAPI Resolve() noexcept
                {
                    return TaskAwaiter<void>::RefData::Resume();
                }
            };

            template<typename... TaskAwaiters>
            static TaskAwaiter<int32_t> __YYAPI WhenAny(const TaskAwaiters&... _Awaiters) noexcept
            {
                constexpr size_t N = sizeof...(TaskAwaiters);
                static_assert(N >= 1, "WaitAny requires at least one awaiter.");

                struct WaitAnyRefData
                    : public YY::RefValue
                    , public TaskAwaiter<int32_t>::RefData
                {
                    YY::RefPtr<TaskAwaiter<void>::RefData> pWaitData[N];

                    int32_t iIndex = -1;

                    ~WaitAnyRefData()
                    {
                        for (int32_t i = 0; i != std::size(pWaitData); ++i)
                        {
                            if (pWaitData[i])
                            {
                                pWaitData[i]->UndoSuspend(this);
                            }
                        }
                    }

                    uint32_t __YYAPI AddRef() noexcept override
                    {
                        return RefValue::AddRef();
                    }

                    uint32_t __YYAPI Release() noexcept override
                    {
                        return RefValue::Release();
                    }

                    int32_t __YYAPI GetResult() noexcept override
                    {
                        if (iIndex == -1)
                        {
                            for (int32_t i = 0; i != _countof(pWaitData); ++i)
                            {
                                if (pWaitData[i]->IsReady())
                                {
                                    iIndex = i;
                                    break;
                                }
                            }
                        }

                        return iIndex;
                    }

                    bool __YYAPI IsReady() noexcept override
                    {
                        return GetResult() != -1;
                    }

                    bool __YYAPI Suspend(std::coroutine_handle<> _hHandle) noexcept override
                    {
                        if (iIndex != -1)
                        {
                            return false;
                        }

                        int32_t i = 0;
                        for (; i != std::size(pWaitData); ++i)
                        {
                            if (!pWaitData[i]->Suspend(this))
                            {
                                iIndex = i;
                                break;
                            }
                        }

                        if (i == std::size(pWaitData) && TaskAwaiter<int32_t>::RefData::Suspend(_hHandle))
                        {
                            return true;
                        }

                        while (i)
                        {
                            --i;
                            pWaitData[i]->UndoSuspend(this);
                        }

                        return false;
                    }

                    HRESULT __YYAPI Resume() override
                    {
                        CoroutineInfo _oCoroutineInfo = oCoroutineInfo.Flush();
                        if (_oCoroutineInfo.hCoroutineHandle == 0 || _oCoroutineInfo.hCoroutineHandle == (intptr_t)-1)
                        {
                            return S_OK;
                        }

                        GetResult();
                        for (int32_t i = 0; i != std::size(pWaitData); ++i)
                        {
                            pWaitData[i]->UndoSuspend(this);
                        }

                        return _oCoroutineInfo.Resume();
                    }
                };

                auto _pAny = RefPtr<WaitAnyRefData>::Create();

                size_t idx = 0;
                ((void)(_pAny->pWaitData[idx++] = _Awaiters.GetAwaiterData()), ...);

                return TaskAwaiter<int32_t>(std::move(_pAny));
            }

            template<typename... TaskAwaiters>
            static TaskAwaiter<void> __YYAPI WhenAll(const TaskAwaiters&... _Awaiters) noexcept
            {
                constexpr size_t N = sizeof...(TaskAwaiters);
                static_assert(N >= 1, "WaitAny requires at least one awaiter.");

                struct WhenAllRefData
                    : public YY::RefValue
                    , public TaskAwaiter<void>::RefData
                {
                    YY::RefPtr<TaskAwaiter<void>::RefData> pWaitData[N];

                    size_t uCompletedCount = 0;

                    ~WaitAnyRefData()
                    {
                        for (int32_t i = 0; i != std::size(pWaitData); ++i)
                        {
                            if (pWaitData[i])
                            {
                                pWaitData[i]->UndoSuspend(this);
                            }
                        }
                    }

                    uint32_t __YYAPI AddRef() noexcept override
                    {
                        return RefValue::AddRef();
                    }

                    uint32_t __YYAPI Release() noexcept override
                    {
                        return RefValue::Release();
                    }

                    bool __YYAPI IsReady() noexcept override
                    {
                        if (uCompletedCount == std::size(pWaitData))
                        {
                            return true;
                        }

                        for (int32_t i = 0; i != _countof(pWaitData); ++i)
                        {
                            if (!pWaitData[i]->IsReady())
                            {
                                return false;
                            }
                        }

                        YY::Sync::Exchange(&uCompletedCount, std::size(pWaitData));
                        return true;
                    }

                    bool __YYAPI Suspend(std::coroutine_handle<> _hHandle) noexcept override
                    {
                        if (uCompletedCount == std::size(pWaitData))
                        {
                            return false;
                        }

                        size_t _uCompletedCount = 0;
                        for (int32_t i = 0; i != std::size(pWaitData); ++i)
                        {

                            if (!pWaitData[i]->Suspend(this))
                            {
                                ++_uCompletedCount;
                            }
                        }

                        YY::Sync::Exchange(&uCompletedCount, _uCompletedCount);

                        if (_uCompletedCount == std::size(pWaitData))
                        {
                            return false;
                        }

                        if (TaskAwaiter<int32_t>::RefData::Suspend(_hHandle))
                        {
                            return true;
                        }

                        for (int32_t i = 0; i != std::size(pWaitData); ++i)
                        {
                            pWaitData[i]->UndoSuspend(this);
                        }

                        return false;
                    }

                    HRESULT __YYAPI Resume() override
                    {
                        if (YY::Increment(&uCompletedCount) < std::size(pWaitData))
                        {
                            return S_OK;
                        }

                        CoroutineInfo _oCoroutineInfo = oCoroutineInfo.Flush();
                        if (_oCoroutineInfo.hCoroutineHandle == 0 || _oCoroutineInfo.hCoroutineHandle == (intptr_t)-1)
                        {
                            return S_OK;
                        }

                        for (int32_t i = 0; i != std::size(pWaitData); ++i)
                        {
                            pWaitData[i]->UndoSuspend(this);
                        }

                        return _oCoroutineInfo.Resume();
                    }
                };

                auto _pWhenAll = RefPtr<WhenAllRefData>::Create();

                size_t idx = 0;
                ((void)(_pWhenAll->pWaitData[idx++] = _Awaiters.GetAwaiterData()), ...);

                return TaskAwaiter<int32_t>(std::move(_pWhenAll));
            }
        }
    }

    using namespace YY::Base::Threading;
}

template<typename LeftType, typename RigthType>
static YY::Base::Threading::TaskAwaiter<int32_t> __YYAPI operator||(const YY::Base::Threading::TaskAwaiter<LeftType>& _Left, const YY::Base::Threading::TaskAwaiter<RigthType>& _Right) noexcept
{
    return YY::Base::Threading::WhenAny(_Left, _Right);
}

template<typename LeftType, typename RigthType>
static YY::Base::Threading::TaskAwaiter<void> __YYAPI operator&&(const YY::Base::Threading::TaskAwaiter<LeftType>& _Left, const YY::Base::Threading::TaskAwaiter<RigthType>& _Right) noexcept
{
    return YY::Base::Threading::WhenAll(_Left, _Right);
}

#pragma pack(pop)

#endif
