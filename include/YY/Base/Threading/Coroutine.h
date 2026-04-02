#pragma once
#include <YY/Base/YY.h>

#if defined(_HAS_CXX20) && _HAS_CXX20
#include <YY/Base/Threading/Task.h>

#pragma pack(push, __YY_PACKING)

namespace YY {
namespace Base {
namespace Threading {

template<typename _ReturnType>
using Coroutine = Task<_ReturnType>;

}
}

using namespace YY::Base::Threading;
}

#pragma pack(pop)

#endif
