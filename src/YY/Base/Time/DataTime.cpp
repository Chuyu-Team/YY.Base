#include <YY/Base/Time/DataTime.h>

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

namespace YY
{
    namespace Base
    {
        namespace Time
        {
            LocalDataTime __YYAPI UtcDataTime::GetLocalDataTime(const DYNAMIC_TIME_ZONE_INFORMATION* _pTimeZoneInformation) const
            {
                auto _oSystemTime = GetSystemTime();
                SYSTEMTIME _oLocalSystemTime;
                SystemTimeToTzSpecificLocalTimeEx(_pTimeZoneInformation, &_oSystemTime, &_oLocalSystemTime);

                LocalDataTime _oLocalDataTime(_oLocalSystemTime);
                // 修正转换过程中的精度损失
                _oLocalDataTime.uTimeInternalValue += uTimeInternalValue % (10 * MillisecondsPerMicrosecond);
                return _oLocalDataTime;
            }

            UtcDataTime __YYAPI LocalDataTime::GetUtcDataTime(const DYNAMIC_TIME_ZONE_INFORMATION* _pTimeZoneInformation) const
            {
                auto _oSystemTime = GetSystemTime();
                SYSTEMTIME _oUtcSystemTime;
                TzSpecificLocalTimeToSystemTimeEx(_pTimeZoneInformation, &_oSystemTime, &_oUtcSystemTime);

                UtcDataTime _oUtcDataTime(_oUtcSystemTime);
                // 修正转换过程中的精度损失
                _oUtcDataTime.uTimeInternalValue += uTimeInternalValue % (10 * MillisecondsPerMicrosecond);
                return _oUtcDataTime;
            }
        }
    }
}
