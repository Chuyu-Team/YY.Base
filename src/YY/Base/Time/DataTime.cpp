#include <YY/Base/Time/DataTime.h>

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

namespace YY
{
    namespace Base
    {
        namespace Time
        {
            SYSTEMTIME __YYAPI UtcDataTime::ToSystemTime() const
            {
                SYSTEMTIME _oSystemTime;
                FileTimeToSystemTime(&oFileTime, &_oSystemTime);
                return _oSystemTime;
            }

            LocalDataTime __YYAPI UtcDataTime::ToLocalDataTime(YY::RefPtr<const TimeZone> _pTimeZone) const
            {
                auto _oSystemTime = ToSystemTime();
                SYSTEMTIME _oLocalSystemTime;
                SystemTimeToTzSpecificLocalTimeEx(_pTimeZone, &_oSystemTime, &_oLocalSystemTime);

                LocalDataTime _oLocalDataTime(_oLocalSystemTime);
                // 修正转换过程中的精度损失
                _oLocalDataTime.uTimeInternalValue += uTimeInternalValue % (10 * MillisecondsPerMicrosecond);
                return _oLocalDataTime;
            }

            SYSTEMTIME __YYAPI LocalDataTime::ToSystemTime() const
            {
                SYSTEMTIME _oSystemTime;
                FileTimeToSystemTime(&oFileTime, &_oSystemTime);
                return _oSystemTime;
            }

            UtcDataTime __YYAPI LocalDataTime::ToUtcDataTime() const
            {
                auto _oSystemTime = ToSystemTime();
                SYSTEMTIME _oUtcSystemTime;
                TzSpecificLocalTimeToSystemTimeEx(pTimeZone, &_oSystemTime, &_oUtcSystemTime);

                UtcDataTime _oUtcDataTime(_oUtcSystemTime);
                // 修正转换过程中的精度损失
                _oUtcDataTime.uTimeInternalValue += uTimeInternalValue % (10 * MillisecondsPerMicrosecond);
                return _oUtcDataTime;
            }
        }
    }
}
