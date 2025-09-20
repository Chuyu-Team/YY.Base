#pragma once
#include <YY/Base/Threading/TaskRunner.h>
#include <YY/Base/Memory/WeakPtr.h>
#include "ThreadPool.h"

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Threading
        {
            extern thread_local WeakPtr<TaskRunner> g_pTaskRunnerWeak;

            uint32_t __YYAPI GenerateNewTaskRunnerId();
            
            uint32_t __YYAPI GetWaitTimeSpan(_In_ TickCount _uWakeupTickCount) noexcept;
        }
    }
} // namespace YY::Base::Threading

#pragma pack(pop)
