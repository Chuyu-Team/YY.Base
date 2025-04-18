﻿#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Time/Common.h>
#include <YY/Base/Utils/MathUtils.h>

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
            template<TimePrecise kPrecise>
            class TimeSpanCommon;
    
            template<>
            class TimeSpanCommon<TimePrecise::Microsecond>
            {
            public:
                constexpr static int64_t __YYAPI GetSecondsPerInternal() noexcept
                {
                    return SecondsPerMillisecond * MillisecondsPerMicrosecond;
                }
            };

            template<>
            class TimeSpanCommon<TimePrecise::Millisecond>
            {
            public:
                constexpr static int64_t __YYAPI GetSecondsPerInternal() noexcept
                {
                    return SecondsPerMillisecond;
                }
            };

            template<TimePrecise ePrecise>
            class TimeSpan
            {
            private:
                constexpr TimeSpan(int64_t _uElapsedInternal) noexcept
                    : uElapsedInternal(_uElapsedInternal)
                {
                }

            public:
                int64_t uElapsedInternal;

                using TimeSpanCommon_t = TimeSpanCommon<ePrecise>;

                constexpr TimeSpan() noexcept
                    : uElapsedInternal(0u)
                {
                }

                constexpr TimeSpan(const TimeSpan&) noexcept = default;

                constexpr int64_t __YYAPI GetInternalValue() const noexcept
                {
                    return uElapsedInternal;
                }

                constexpr static TimeSpan __YYAPI GetMax() noexcept
                {
                    return TimeSpan(INT64_MAX);
                }

                constexpr static int64_t __YYAPI GetSecondsPerInternal() noexcept
                {
                    return TimeSpanCommon_t::GetSecondsPerInternal();
                }

                constexpr static TimeSpan __YYAPI FromInternalValue(int64_t _uElapsedInternalValue) noexcept
                {
                    return TimeSpan(_uElapsedInternalValue);
                }

                constexpr static TimeSpan __YYAPI FromMicroseconds(int64_t _uElapsedMicroseconds) noexcept
                {
                    // return TimeSpan(_uElapsedMicroseconds * GetSecondsPerInternal() / (SecondsPerMillisecond * MillisecondsPerMicrosecond));
                    return TimeSpan(MulDiv64Fast(_uElapsedMicroseconds, GetSecondsPerInternal(), SecondsPerMillisecond * MillisecondsPerMicrosecond));
                }

                constexpr static TimeSpan __YYAPI FromMilliseconds(int64_t _uElapsedMilliseconds) noexcept
                {
                    // return TimeSpan(_uElapsedMilliseconds * GetSecondsPerInternal() / SecondsPerMillisecond);
                    return TimeSpan(MulDiv64Fast(_uElapsedMilliseconds, GetSecondsPerInternal(), SecondsPerMillisecond));
                }

                constexpr static TimeSpan __YYAPI FromSeconds(int64_t _uElapsedSeconds) noexcept
                {
                    return TimeSpan(_uElapsedSeconds * GetSecondsPerInternal());
                }

                constexpr static TimeSpan __YYAPI FromMinutes(int64_t _uElapsedMinutes) noexcept
                {
                    return TimeSpan(_uElapsedMinutes * GetSecondsPerInternal() * MinutesPerSecond);
                }

                constexpr static TimeSpan __YYAPI FromHours(int64_t _uElapsedHours) noexcept
                {
                    return TimeSpan(_uElapsedHours * GetSecondsPerInternal() * MinutesPerSecond * HoursPerMinute);
                }

                constexpr static TimeSpan __YYAPI FromDays(int64_t _uElapsedDays) noexcept
                {
                    return TimeSpan(_uElapsedDays * GetSecondsPerInternal() * MinutesPerSecond * HoursPerMinute * DaysPerHour);
                }
        
                constexpr int64_t __YYAPI GetMicroseconds() const noexcept
                {
                    // return uElapsedInternal * SecondsPerMillisecond * MillisecondsPerMicrosecond / GetSecondsPerInternal();
                    return MulDiv64Fast(uElapsedInternal, SecondsPerMillisecond * MillisecondsPerMicrosecond, GetSecondsPerInternal());
                }

                constexpr int64_t __YYAPI GetMilliseconds() const noexcept
                {
                    // return uElapsedInternal * SecondsPerMillisecond / GetSecondsPerInternal();
                    return MulDiv64Fast(uElapsedInternal, SecondsPerMillisecond, GetSecondsPerInternal());
                }

                constexpr int64_t __YYAPI GetSeconds() const noexcept
                {
                    return uElapsedInternal / GetSecondsPerInternal();
                }

                constexpr int64_t __YYAPI GetMinutes() const noexcept
                {
                    return uElapsedInternal / (GetSecondsPerInternal() * MinutesPerSecond);
                }

                constexpr int64_t __YYAPI GetHours() const noexcept
                {
                    return uElapsedInternal / (GetSecondsPerInternal() * MinutesPerSecond * HoursPerMinute);
                }

                constexpr int64_t __YYAPI GetDays() const noexcept
                {
                    return uElapsedInternal / (GetSecondsPerInternal() * MinutesPerSecond * HoursPerMinute * DaysPerHour);
                }

                TimeSpan& __YYAPI operator=(const TimeSpan&) noexcept = default;

                constexpr bool __YYAPI operator==(const TimeSpan& _oOther) const noexcept
                {
                    return uElapsedInternal == _oOther.uElapsedInternal;
                }

#if defined(_HAS_CXX20) && _HAS_CXX20
                constexpr auto __YYAPI operator<=>(const TimeSpan& _oOther) const noexcept = default;
#else
                constexpr bool __YYAPI operator<(const TimeSpan& _oOther) const noexcept
                {
                    return uElapsedInternal < _oOther.uElapsedInternal;
                }

                constexpr bool __YYAPI operator<=(const TimeSpan& _oOther) const noexcept
                {
                    return uElapsedInternal <= _oOther.uElapsedInternal;
                }

                constexpr bool __YYAPI operator>=(const TimeSpan& _oOther) const noexcept
                {
                    return uElapsedInternal >= _oOther.uElapsedInternal;
                }

                constexpr bool __YYAPI operator>(const TimeSpan& _oOther) const noexcept
                {
                    return uElapsedInternal >= _oOther.uElapsedInternal;
                }

                constexpr bool __YYAPI operator!=(const TimeSpan& _oOther) const noexcept
                {
                    return uElapsedInternal != _oOther.uElapsedInternal;
                }
#endif
                constexpr TimeSpan& __YYAPI operator-=(const TimeSpan& _oOther) noexcept
                {
                    uElapsedInternal -= _oOther.uElapsedInternal;
                    return *this;
                }

                constexpr TimeSpan& __YYAPI operator+=(const TimeSpan& _oOther) noexcept
                {
                    uElapsedInternal += _oOther.uElapsedInternal;
                    return *this;
                }
            };
        }
    }
}
#pragma pack(pop)
