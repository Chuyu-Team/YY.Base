#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Time/Common.h>
#include <YY/Base/Time/TimeSpan.h>
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
            class TickCount
            {
            private:
                // 防止错误的从 uint64_t 构造，声明 为 private
                // 如有需要请使用 TickCount::FromInternalValue
                constexpr TickCount(uint64_t _uTickCountInternal) noexcept
                    : uTickCountInternal(_uTickCountInternal)
                {
                }

            public:
                uint64_t uTickCountInternal = 0u;

                constexpr TickCount() noexcept = default;

                constexpr TickCount(const TickCount&) noexcept = default;

                static TickCount __YYAPI GetNow()
                {
#if _WIN32
                    LARGE_INTEGER _PerformanceCounter = {};
                    QueryPerformanceCounter(&_PerformanceCounter);
                    return TickCount(_PerformanceCounter.QuadPart);
#else
#error "not implement"
#endif
                }

                constexpr static TickCount __YYAPI GetMax() noexcept
                {
                    return TickCount(UINT64_MAX);
                }

                static int64_t __YYAPI GetSecondsPerInternal()
                {
#if _WIN32
                    static LARGE_INTEGER s_Frequency = {};
                    if (s_Frequency.QuadPart == 0)
                    {
                        QueryPerformanceFrequency(&s_Frequency);
                    }

                    return s_Frequency.QuadPart;
#else
#error "not implement"
#endif
                }


                constexpr static TickCount __YYAPI FromInternalValue(uint64_t _uTickCountInternal) noexcept
                {
                    return TickCount(_uTickCountInternal);
                }

#if _WIN32
                constexpr static TickCount __YYAPI FromPerformanceCounter(uint64_t _uPerformanceCounter) noexcept
                {
                    return FromInternalValue(_uPerformanceCounter);
                }
#endif

                /// <summary>
                /// </summary>
                /// <param name="_uTickCountMicroseconds">开机以来的微秒数</param>
                /// <returns></returns>
                constexpr static TickCount __YYAPI FromMicroseconds(int64_t _uTickCountMicroseconds) noexcept
                {
                    return TickCount(_uTickCountMicroseconds * GetSecondsPerInternal() / (SecondsPerMillisecond * MillisecondsPerMicrosecond));
                }

                constexpr static TickCount __YYAPI FromMilliseconds(int64_t _uTickCountMilliseconds) noexcept
                {
                    return TickCount(_uTickCountMilliseconds * GetSecondsPerInternal() / SecondsPerMillisecond);
                }

                constexpr static TickCount __YYAPI FromSeconds(int64_t _uTickCountSeconds) noexcept
                {
                    return TickCount(_uTickCountSeconds * GetSecondsPerInternal());
                }

                constexpr static TickCount __YYAPI FromMinutes(int64_t _uTickCountMinutes) noexcept
                {
                    return TickCount(_uTickCountMinutes * GetSecondsPerInternal() * MinutesPerSecond);
                }

                constexpr static TickCount __YYAPI FromHours(int64_t _uTickCountHours) noexcept
                {
                    return TickCount(_uTickCountHours * GetSecondsPerInternal() * MinutesPerSecond * HoursPerMinute);
                }

                constexpr static TickCount __YYAPI FromDays(int64_t _uTickCountDays) noexcept
                {
                    return TickCount(_uTickCountDays * GetSecondsPerInternal() * MinutesPerSecond * HoursPerMinute * DaysPerHour);
                }

                constexpr uint64_t __YYAPI GetInternalValue() const noexcept
                {
                    return uTickCountInternal;
                }

                uint64_t __YYAPI GetMicroseconds() const
                {
                    return uTickCountInternal * SecondsPerMillisecond * MillisecondsPerMicrosecond / GetSecondsPerInternal();
                }

                uint64_t __YYAPI GetMilliseconds() const
                {
                    return uTickCountInternal * SecondsPerMillisecond / GetSecondsPerInternal();
                }

                uint64_t __YYAPI GetSeconds() const
                {
                    return uTickCountInternal / GetSecondsPerInternal();
                }

                uint64_t __YYAPI GetMinutes() const
                {
                    return uTickCountInternal / (GetSecondsPerInternal() * MinutesPerSecond);
                }

                uint64_t __YYAPI GetHours() const
                {
                    return uTickCountInternal / (GetSecondsPerInternal() * MinutesPerSecond * HoursPerMinute);
                }

                uint64_t __YYAPI GetDays() const
                {
                    return uTickCountInternal / (GetSecondsPerInternal() * MinutesPerSecond * HoursPerMinute * DaysPerHour);
                }
        
                constexpr TickCount& operator=(const TickCount&) noexcept = default;

                constexpr bool operator==(const TickCount& _oOther) const noexcept
                {
                    return uTickCountInternal == _oOther.uTickCountInternal;
                }

#if defined(_HAS_CXX20) && _HAS_CXX20
                constexpr auto operator<=>(const TickCount& _oOther) const noexcept
                {
                    return uTickCountInternal <=> _oOther.uTickCountInternal;
                }
#else
                constexpr bool operator>(const TickCount& _oOther) const noexcept
                {
                    return uTickCountInternal > _oOther.uTickCountInternal;
                }

                constexpr bool operator>=(const TickCount& _oOther) const noexcept
                {
                    return uTickCountInternal >= _oOther.uTickCountInternal;
                }

                constexpr bool operator<=(const TickCount& _oOther) const noexcept
                {
                    return uTickCountInternal <= _oOther.uTickCountInternal;
                }

                constexpr bool operator<(const TickCount& _oOther) const noexcept
                {
                    return uTickCountInternal < _oOther.uTickCountInternal;
                }

                constexpr bool operator!=(const TickCount& _oOther) const noexcept
                {
                    return uTickCountInternal != _oOther.uTickCountInternal;
                }
#endif

                constexpr TickCount& operator+=(const TimeSpan& _nSpan) noexcept
                {
                    // uTickCountInternal += _nSpan.GetInternalValue() * GetSecondsPerInternal() / TimeSpan::GetSecondsPerInternal();
                    uTickCountInternal += UMulDiv64Fast(_nSpan.GetInternalValue(), GetSecondsPerInternal(), TimeSpan::GetSecondsPerInternal());
                    return *this;
                }

                constexpr TickCount operator+(const TimeSpan& _nSpan) noexcept
                {
                    TickCount _oTmp = *this;
                    _oTmp += _nSpan;
                    return _oTmp;
                }

                constexpr TickCount& operator-=(const TimeSpan& _nSpan) noexcept
                {
                    // uTickCountInternal -= _nSpan.GetInternalValue() * GetSecondsPerInternal() / TimeSpan::GetSecondsPerInternal();
                    uTickCountInternal -= UMulDiv64Fast(_nSpan.GetInternalValue(), GetSecondsPerInternal(), TimeSpan::GetSecondsPerInternal());
                    return *this;
                }

                constexpr TickCount operator-(const TimeSpan& _nSpan) noexcept
                {
                    TickCount _oTmp = *this;
                    _oTmp -= _nSpan;
                    return _oTmp;
                }

                constexpr TimeSpan operator-(const TickCount& _oOther) const noexcept
                {
                    int64_t _nSpanInternal = uTickCountInternal - _oOther.uTickCountInternal;
                    // return TimeSpan::FromInternalValue(_nSpanInternal * TimeSpan::GetSecondsPerInternal() / int64_t(GetSecondsPerInternal()));
                    return TimeSpan::FromInternalValue(MulDiv64Fast(_nSpanInternal, TimeSpan::GetSecondsPerInternal(), GetSecondsPerInternal()));
                }
            };
        }
    }
}

#pragma pack(pop)
