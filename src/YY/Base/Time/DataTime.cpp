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
                if (!FileTimeToSystemTime(&oFileTime, &_oSystemTime))
                {
                    throw YY::Exception(__HRESULT_FROM_WIN32(GetLastError()));
                }

                return _oSystemTime;
            }

            LocalDataTime __YYAPI UtcDataTime::ToLocalDataTime(YY::RefPtr<const TimeZone> _pTimeZone) const
            {
                try
                {
                    auto _oSystemTime = ToSystemTime();
                    SYSTEMTIME _oLocalSystemTime;
                    if (!SystemTimeToTzSpecificLocalTimeEx(_pTimeZone, &_oSystemTime, &_oLocalSystemTime))
                    {
                        throw YY::Exception(__HRESULT_FROM_WIN32(GetLastError()));
                    }

                    LocalDataTime _oLocalDataTime(_oLocalSystemTime, _pTimeZone);
                    // 修正转换过程中的精度损失
                    _oLocalDataTime.uTicks += uTicks % (kTicksPerMicrosecond * kMicrosecondsPerMillisecond);
                    return _oLocalDataTime;
                }
                catch (const YY::Exception&)
                {
                    return LocalDataTime(std::move(_pTimeZone));
                }
            }

            SYSTEMTIME __YYAPI LocalDataTime::ToSystemTime() const
            {
                SYSTEMTIME _oSystemTime;
                if (!FileTimeToSystemTime(&oFileTime, &_oSystemTime))
                {
                    throw YY::Exception(__HRESULT_FROM_WIN32(GetLastError()));
                }

                return _oSystemTime;
            }

            UtcDataTime __YYAPI LocalDataTime::ToUtcDataTime() const
            {
                try
                {
                    auto _oSystemTime = ToSystemTime();
                    SYSTEMTIME _oUtcSystemTime;
                    if (!TzSpecificLocalTimeToSystemTimeEx(pTimeZone, &_oSystemTime, &_oUtcSystemTime))
                    {
                        throw YY::Exception(__HRESULT_FROM_WIN32(GetLastError()));
                    }

                    UtcDataTime _oUtcDataTime(_oUtcSystemTime);
                    // 修正转换过程中的精度损失
                    _oUtcDataTime.uTicks += uTicks % (kTicksPerMicrosecond * kMicrosecondsPerMillisecond);
                    return _oUtcDataTime;
                }
                catch (const YY::Exception&)
                {
                    return UtcDataTime();
                }
            }
        }
    }
}
