#pragma once
#include <YY/Base/YY.h>

/*
AutoCleanup 使用RAII思想，在对象生命周期结束时，自动调用指定的清理函数。
但是不建议频繁的使用它，优先建议使用智能指针等更合适的资源管理方式。

```cpp
{
    int _iValue = 0;
    {
        auto _oCleanup = YY::MakeAutoCleanup(
            [&]()
            {
                _iValue++;
            });
        // 在这里 _iValue 仍然是 0
        _iValue == 0;
    }
    // 离开作用域后，清理函数被调用，_iValue 变为 1
    _iValue == 1;
}
```
*/

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Utils
        {
            template<typename Lambda>
            class AutoCleanupImpl
            {
            private:
                Lambda pfnCleanupCallBack;

            public:
                AutoCleanupImpl(Lambda&& _pfnCallBack) noexcept
                    : pfnCleanupCallBack(std::forward<Lambda>(_pfnCallBack))
                {
                }

#if defined(_HAS_CXX17) && _HAS_CXX17
                AutoCleanupImpl(const AutoCleanupImpl&) = delete;
#else
                AutoCleanupImpl(const AutoCleanupImpl&) = default;
#endif

                AutoCleanupImpl& operator=(const AutoCleanupImpl&) = delete;

                ~AutoCleanupImpl()
                {
                    pfnCleanupCallBack();
                }
            };

            template<typename Lambda>
            _Check_return_ AutoCleanupImpl<Lambda> __YYAPI MakeAutoCleanup(Lambda&& _pfnCallBack)
            {
                return AutoCleanupImpl<Lambda>(std::forward<Lambda>(_pfnCallBack));
            }
        }
    }

    using namespace YY::Base::Utils;
}

#pragma pack(pop)
