#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Time/Common.h>

#if defined(_HAS_CXX20) && _HAS_CXX20
#include <compare>
#endif

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Time
        {
            /// <summary>
            /// 表示一段时间间，精度微秒，并提供多种单位的转换和操作方法。
            /// </summary>
            class TimeSpan
            {
            private:
                constexpr TimeSpan(int64_t _iElapsedMicroseconds) noexcept
                    : iElapsedMicroseconds(_iElapsedMicroseconds)
                {
                }

            public:
                int64_t iElapsedMicroseconds = 0;

                constexpr TimeSpan() = default;

                constexpr TimeSpan(const TimeSpan&) noexcept = default;

                constexpr static TimeSpan __YYAPI GetMax() noexcept
                {
                    return TimeSpan(INT64_MAX);
                }

                constexpr static int64_t __YYAPI GetSecondsPerInternal() noexcept
                {
                    return SecondsPerMillisecond * MillisecondsPerMicrosecond;
                }

                constexpr static TimeSpan __YYAPI FromInternalValue(int64_t _iElapsedMicroseconds) noexcept
                {
                    return TimeSpan(_iElapsedMicroseconds);
                }

                constexpr static TimeSpan __YYAPI FromMicroseconds(int64_t _iElapsedMicroseconds) noexcept
                {
                    return TimeSpan(_iElapsedMicroseconds);
                }

                constexpr static TimeSpan __YYAPI FromMilliseconds(int64_t _uElapsedMilliseconds) noexcept
                {
                    return TimeSpan(_uElapsedMilliseconds * MillisecondsPerMicrosecond);
                }

                constexpr static TimeSpan __YYAPI FromSeconds(int64_t _uElapsedSeconds) noexcept
                {
                    return TimeSpan(_uElapsedSeconds * SecondsPerMillisecond * MillisecondsPerMicrosecond);
                }

                constexpr static TimeSpan __YYAPI FromMinutes(int64_t _uElapsedMinutes) noexcept
                {
                    return TimeSpan(_uElapsedMinutes  * MinutesPerSecond * SecondsPerMillisecond * MillisecondsPerMicrosecond);
                }

                constexpr static TimeSpan __YYAPI FromHours(int64_t _uElapsedHours) noexcept
                {
                    return TimeSpan(_uElapsedHours * HoursPerMinute * MinutesPerSecond * SecondsPerMillisecond * MillisecondsPerMicrosecond);
                }

                constexpr static TimeSpan __YYAPI FromDays(int64_t _uElapsedDays) noexcept
                {
                    return TimeSpan(_uElapsedDays * DaysPerHour * HoursPerMinute * MinutesPerSecond * SecondsPerMillisecond * MillisecondsPerMicrosecond);
                }
        
                constexpr int64_t __YYAPI GetInternalValue() const noexcept
                {
                    return iElapsedMicroseconds;
                }

                constexpr int64_t __YYAPI GetMicroseconds() const noexcept
                {
                    return iElapsedMicroseconds;
                }

                constexpr int64_t __YYAPI GetMilliseconds() const noexcept
                {
                    return iElapsedMicroseconds / MillisecondsPerMicrosecond;
                }

                constexpr int64_t __YYAPI GetSeconds() const noexcept
                {
                    return iElapsedMicroseconds / (SecondsPerMillisecond * MillisecondsPerMicrosecond);
                }

                constexpr int64_t __YYAPI GetMinutes() const noexcept
                {
                    return iElapsedMicroseconds / (MinutesPerSecond * SecondsPerMillisecond * MillisecondsPerMicrosecond);
                }

                constexpr int64_t __YYAPI GetHours() const noexcept
                {
                    return iElapsedMicroseconds / (HoursPerMinute * MinutesPerSecond * SecondsPerMillisecond * MillisecondsPerMicrosecond);
                }

                constexpr int64_t __YYAPI GetDays() const noexcept
                {
                    return iElapsedMicroseconds / (DaysPerHour * HoursPerMinute * MinutesPerSecond * SecondsPerMillisecond * MillisecondsPerMicrosecond);
                }

                TimeSpan& __YYAPI operator=(const TimeSpan&) noexcept = default;

                constexpr bool __YYAPI operator==(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedMicroseconds == _oOther.iElapsedMicroseconds;
                }

#if defined(_HAS_CXX20) && _HAS_CXX20
                constexpr auto __YYAPI operator<=>(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedMicroseconds <=> _oOther.iElapsedMicroseconds;
                }
#else
                constexpr bool __YYAPI operator<(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedMicroseconds < _oOther.iElapsedMicroseconds;
                }

                constexpr bool __YYAPI operator<=(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedMicroseconds <= _oOther.iElapsedMicroseconds;
                }

                constexpr bool __YYAPI operator>=(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedMicroseconds >= _oOther.iElapsedMicroseconds;
                }

                constexpr bool __YYAPI operator>(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedMicroseconds >= _oOther.iElapsedMicroseconds;
                }

                constexpr bool __YYAPI operator!=(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedMicroseconds != _oOther.iElapsedMicroseconds;
                }
#endif
                constexpr TimeSpan& __YYAPI operator-=(const TimeSpan& _oOther) noexcept
                {
                    iElapsedMicroseconds -= _oOther.iElapsedMicroseconds;
                    return *this;
                }

                constexpr TimeSpan& __YYAPI operator+=(const TimeSpan& _oOther) noexcept
                {
                    iElapsedMicroseconds += _oOther.iElapsedMicroseconds;
                    return *this;
                }
            };
        }
    }
}
#pragma pack(pop)
