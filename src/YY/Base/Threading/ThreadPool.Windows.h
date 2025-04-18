﻿#pragma once

#include <YY/Base/Memory/RefPtr.h>
#include <YY/Base/ErrorCode.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            class ThreadPool
            {
            public:
                template<typename Task>
                static HRESULT __YYAPI PostTaskInternalWithoutAddRef(_In_ Task* _pTask) noexcept
                {
                    auto _bRet = TrySubmitThreadpoolCallback(
                        [](_Inout_ PTP_CALLBACK_INSTANCE _pInstance,
                            _In_   PVOID _pContext)
                        {
                            auto _pTask = reinterpret_cast<Task*>(_pContext);
                            _pTask->operator()();
                        },
                        _pTask,
                        nullptr);

                    return _bRet ? S_OK : HRESULT_From_LSTATUS(GetLastError());
                }

                template<typename Task>
                static HRESULT __YYAPI PostTaskInternal(_In_ Task* _pTask) noexcept
                {
                    _pTask->AddRef();
                    auto _bRet = TrySubmitThreadpoolCallback(
                        [](_Inout_ PTP_CALLBACK_INSTANCE _pInstance,
                            _In_   PVOID _pContext)
                        {
                            auto _pTask = reinterpret_cast<Task*>(_pContext);
                            _pTask->operator()();
                            _pTask->Release();
                        },
                        _pTask,
                        nullptr);

                    if (!_bRet)
                    {
                        auto _hr = HRESULT_From_LSTATUS(GetLastError());
                        _pTask->Release();
                        return _hr;
                    }

                    return S_OK;
                }
            };
        }
    }
}

namespace YY
{
    using namespace YY::Base::Threading;
}

#pragma pack(pop)
