#pragma once

#if defined(_WIN32)
#include "ThreadPool.Windows.h"
#else
#include "ThreadPool.Linux.h"
#endif
