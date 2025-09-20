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
            /// 表示一段时间间，精度100纳秒，并提供多种单位的转换和操作方法。
            /// </summary>
            class TimeSpan
            {
            private:
                constexpr TimeSpan(int64_t _iElapsedTicks) noexcept
                    : iElapsedTicks(_iElapsedTicks)
                {
                }

            public:
                int64_t iElapsedTicks = 0;

                static constexpr int64_t kTicksPerMicrosecond = 10;

                constexpr TimeSpan() = default;

                constexpr TimeSpan(const TimeSpan&) noexcept = default;

                constexpr static TimeSpan __YYAPI GetMax() noexcept
                {
                    return TimeSpan(INT64_MAX);
                }

                constexpr static TimeSpan __YYAPI GetMin() noexcept
                {
                    return TimeSpan(INT64_MIN);
                }

                constexpr static int64_t __YYAPI GetTicksPerSecond() noexcept
                {
                    return kMillisecondsPerSecond * kMicrosecondsPerMillisecond * kTicksPerMicrosecond;
                }

                constexpr static TimeSpan __YYAPI FromTicks(int64_t _iElapsedTicks) noexcept
                {
                    return TimeSpan(_iElapsedTicks);
                }

                constexpr static TimeSpan __YYAPI FromMicroseconds(int64_t _iElapsedMicroseconds) noexcept
                {
                    return TimeSpan(_iElapsedMicroseconds * kTicksPerMicrosecond);
                }

                constexpr static TimeSpan __YYAPI FromMilliseconds(int64_t _uElapsedMilliseconds) noexcept
                {
                    return TimeSpan(_uElapsedMilliseconds * kMicrosecondsPerMillisecond * kTicksPerMicrosecond);
                }

                constexpr static TimeSpan __YYAPI FromSeconds(int64_t _uElapsedSeconds) noexcept
                {
                    return TimeSpan(_uElapsedSeconds * kMillisecondsPerSecond * kMicrosecondsPerMillisecond * kTicksPerMicrosecond);
                }

                constexpr static TimeSpan __YYAPI FromMinutes(int64_t _uElapsedMinutes) noexcept
                {
                    return TimeSpan(_uElapsedMinutes  * kSecondsPerMinute * kMillisecondsPerSecond * kMicrosecondsPerMillisecond * kTicksPerMicrosecond);
                }

                constexpr static TimeSpan __YYAPI FromHours(int64_t _uElapsedHours) noexcept
                {
                    return TimeSpan(_uElapsedHours * kMinutesPerHour * kSecondsPerMinute * kMillisecondsPerSecond * kMicrosecondsPerMillisecond * kTicksPerMicrosecond);
                }

                constexpr static TimeSpan __YYAPI FromDays(int64_t _uElapsedDays) noexcept
                {
                    return TimeSpan(_uElapsedDays * kHoursPerDay * kMinutesPerHour * kSecondsPerMinute * kMillisecondsPerSecond * kMicrosecondsPerMillisecond * kTicksPerMicrosecond);
                }
        
                constexpr int64_t __YYAPI GetTicks() const noexcept
                {
                    return iElapsedTicks;
                }

                constexpr int64_t __YYAPI GetTotalMicroseconds() const noexcept
                {
                    return iElapsedTicks / kTicksPerMicrosecond;
                }

                constexpr int64_t __YYAPI GetTotalMilliseconds() const noexcept
                {
                    return iElapsedTicks / (kMicrosecondsPerMillisecond * kTicksPerMicrosecond);
                }

                constexpr int64_t __YYAPI GetTotalSeconds() const noexcept
                {
                    return iElapsedTicks / (kMillisecondsPerSecond * kMicrosecondsPerMillisecond * kTicksPerMicrosecond);
                }

                constexpr int64_t __YYAPI GetTotalMinutes() const noexcept
                {
                    return iElapsedTicks / (kSecondsPerMinute * kMillisecondsPerSecond * kMicrosecondsPerMillisecond * kTicksPerMicrosecond);
                }

                constexpr int64_t __YYAPI GetTotalHours() const noexcept
                {
                    return iElapsedTicks / (kMinutesPerHour * kSecondsPerMinute * kMillisecondsPerSecond * kMicrosecondsPerMillisecond * kTicksPerMicrosecond);
                }

                constexpr int64_t __YYAPI GetTotalDays() const noexcept
                {
                    return iElapsedTicks / (kHoursPerDay * kMinutesPerHour * kSecondsPerMinute * kMillisecondsPerSecond * kMicrosecondsPerMillisecond * kTicksPerMicrosecond);
                }

                TimeSpan& __YYAPI operator=(const TimeSpan&) noexcept = default;

                constexpr bool __YYAPI operator==(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedTicks == _oOther.iElapsedTicks;
                }

#if defined(_HAS_CXX20) && _HAS_CXX20
                constexpr auto __YYAPI operator<=>(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedTicks <=> _oOther.iElapsedTicks;
                }
#else
                constexpr bool __YYAPI operator<(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedTicks < _oOther.iElapsedTicks;
                }

                constexpr bool __YYAPI operator<=(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedTicks <= _oOther.iElapsedTicks;
                }

                constexpr bool __YYAPI operator>=(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedTicks >= _oOther.iElapsedTicks;
                }

                constexpr bool __YYAPI operator>(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedTicks >= _oOther.iElapsedTicks;
                }

                constexpr bool __YYAPI operator!=(const TimeSpan& _oOther) const noexcept
                {
                    return iElapsedTicks != _oOther.iElapsedTicks;
                }
#endif
                constexpr TimeSpan& __YYAPI operator-=(const TimeSpan& _oOther) noexcept
                {
                    iElapsedTicks -= _oOther.iElapsedTicks;
                    return *this;
                }

                constexpr TimeSpan& __YYAPI operator+=(const TimeSpan& _oOther) noexcept
                {
                    iElapsedTicks += _oOther.iElapsedTicks;
                    return *this;
                }
            };
        }
    }
}
#pragma pack(pop)
