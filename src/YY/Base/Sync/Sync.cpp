#include <YY/Base/Sync/Sync.h>
#include <YY/Base/Time/TickCount.h>

#pragma comment(lib, "Synchronization.lib")

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

namespace YY
{
    namespace Base
    {
        namespace Sync
        {
            template<typename ValueType>
            bool __YYAPI WaitEqualOnAddress(
                volatile ValueType* _pAddress,
                ValueType _CompareValue,
                TimeSpan _uMilliseconds) noexcept
            {
                if (_uMilliseconds.GetTotalMilliseconds() <= 0)
                {
                    return *_pAddress == _CompareValue;
                }
                else if (_uMilliseconds == TimeSpan::GetMax())
                {
                    for (;;)
                    {
                        auto _Temp = *_pAddress;
                        if (_Temp == _CompareValue)
                            break;

                        WaitOnAddress(_pAddress, &_Temp, sizeof(_Temp), UINT32_MAX);
                    }

                    return true;
                }
                else
                {
                    const auto _uExpireTick = TickCount::GetNow() + _uMilliseconds;
                    for (;;)
                    {
                        auto _Temp = *_pAddress;
                        if (_Temp == _CompareValue)
                            break;

                        const auto _uCurrent = TickCount::GetNow();
                        if (_uCurrent > _uExpireTick)
                            return false;

                        if (!WaitOnAddress(_pAddress, &_Temp, sizeof(_Temp), (_uExpireTick - _uCurrent).GetTotalMilliseconds()))
                            return false;
                    }

                    return true;
                }
            }

            bool __YYAPI WaitEqualOnAddress(
                volatile void* Address,
                void* CompareAddress,
                size_t AddressSize,
                TimeSpan _uMilliseconds) noexcept
            {
                switch (AddressSize)
                {
                case 1:
                {
                    using Type = uint8_t;
                    return WaitEqualOnAddress((volatile Type*)(Address), *(Type*)(CompareAddress), _uMilliseconds);
                }
                case 2:
                {
                    using Type = uint16_t;
                    return WaitEqualOnAddress((volatile Type*)(Address), *(Type*)(CompareAddress), _uMilliseconds);
                }
                case 4:
                {
                    using Type = uint32_t;
                    return WaitEqualOnAddress((volatile Type*)(Address), *(Type*)(CompareAddress), _uMilliseconds);
                }
                case 8:
                {
                    using Type = uint64_t;
                    return WaitEqualOnAddress((volatile Type*)(Address), *(Type*)(CompareAddress), _uMilliseconds);
                }
                default:
                    return false;
                    break;
                }
            }
        }
    }
}
