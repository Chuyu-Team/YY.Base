#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Time/Common.h>
#include <YY/Base/Time/TimeSpan.h>
#include <YY/Base/Utils/MathUtils.h>
#include <YY/Base/Sync/Interlocked.h>

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
                constexpr TickCount(uint64_t _uTickCountTicks) noexcept
                    : uTicks(_uTickCountTicks)
                {
                }

            public:
                uint64_t uTicks = 0u;

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

                constexpr static TickCount __YYAPI GetMin() noexcept
                {
                    return TickCount(0);
                }

                static int64_t __YYAPI GetTicksPerSecond()
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


                constexpr static TickCount __YYAPI FromTicks(uint64_t _uTickCountTicks) noexcept
                {
                    return TickCount(_uTickCountTicks);
                }

#if _WIN32
                constexpr static TickCount __YYAPI FromPerformanceCounter(uint64_t _uPerformanceCounter) noexcept
                {
                    return TickCount(_uPerformanceCounter);
                }
#endif

                /// <summary>
                /// </summary>
                /// <param name="_uTickCountMicroseconds">开机以来的微秒数</param>
                /// <returns></returns>
                constexpr static TickCount __YYAPI FromMicroseconds(int64_t _uTickCountMicroseconds) noexcept
                {
                    return TickCount(_uTickCountMicroseconds * (GetTicksPerSecond() / (kMillisecondsPerSecond * kMicrosecondsPerMillisecond)));
                }

                constexpr static TickCount __YYAPI FromMilliseconds(int64_t _uTickCountMilliseconds) noexcept
                {
                    return TickCount(_uTickCountMilliseconds * (GetTicksPerSecond() / kMillisecondsPerSecond));
                }

                constexpr static TickCount __YYAPI FromSeconds(int64_t _uTickCountSeconds) noexcept
                {
                    return TickCount(_uTickCountSeconds * GetTicksPerSecond());
                }

                constexpr static TickCount __YYAPI FromMinutes(int64_t _uTickCountMinutes) noexcept
                {
                    return TickCount(_uTickCountMinutes * GetTicksPerSecond() * kSecondsPerMinute);
                }

                constexpr static TickCount __YYAPI FromHours(int64_t _uTickCountHours) noexcept
                {
                    return TickCount(_uTickCountHours * GetTicksPerSecond() * kSecondsPerMinute * kMinutesPerHour);
                }

                constexpr static TickCount __YYAPI FromDays(int64_t _uTickCountDays) noexcept
                {
                    return TickCount(_uTickCountDays * GetTicksPerSecond() * kSecondsPerMinute * kMinutesPerHour * kHoursPerDay);
                }

                /// <summary>
                /// 开机以来的计时刻度数。刻度具体值参照：GetTicksPerSecond()
                /// </summary>
                /// <returns></returns>
                constexpr uint64_t __YYAPI GetTicks() const noexcept
                {
                    return uTicks;
                }

                /// <summary>
                /// 开机以来的微秒数。
                /// </summary>
                /// <returns></returns>
                uint64_t __YYAPI GetTotalMicroseconds() const
                {
                    return uTicks / (GetTicksPerSecond() / (kMillisecondsPerSecond * kMicrosecondsPerMillisecond));
                }

                /// <summary>
                /// 开机以来的毫秒数。
                /// </summary>
                /// <returns></returns>
                uint64_t __YYAPI GetTotalMilliseconds() const
                {
                    return uTicks / (GetTicksPerSecond() / kMillisecondsPerSecond);
                }

                uint64_t __YYAPI GetTotalSeconds() const
                {
                    return uTicks / GetTicksPerSecond();
                }

                uint64_t __YYAPI GetTotalMinutes() const
                {
                    return uTicks / (GetTicksPerSecond() * kSecondsPerMinute);
                }

                uint64_t __YYAPI GetTotalHours() const
                {
                    return uTicks / (GetTicksPerSecond() * kSecondsPerMinute * kMinutesPerHour);
                }

                uint64_t __YYAPI GetTotalDays() const
                {
                    return uTicks / (GetTicksPerSecond() * kSecondsPerMinute * kMinutesPerHour * kHoursPerDay);
                }
        
                constexpr TickCount& operator=(const TickCount&) noexcept = default;

                constexpr bool operator==(const TickCount& _oOther) const noexcept
                {
                    return uTicks == _oOther.uTicks;
                }

#if defined(_HAS_CXX20) && _HAS_CXX20
                constexpr auto operator<=>(const TickCount& _oOther) const noexcept
                {
                    return uTicks <=> _oOther.uTicks;
                }
#else
                constexpr bool operator>(const TickCount& _oOther) const noexcept
                {
                    return uTicks > _oOther.uTicks;
                }

                constexpr bool operator>=(const TickCount& _oOther) const noexcept
                {
                    return uTicks >= _oOther.uTicks;
                }

                constexpr bool operator<=(const TickCount& _oOther) const noexcept
                {
                    return uTicks <= _oOther.uTicks;
                }

                constexpr bool operator<(const TickCount& _oOther) const noexcept
                {
                    return uTicks < _oOther.uTicks;
                }

                constexpr bool operator!=(const TickCount& _oOther) const noexcept
                {
                    return uTicks != _oOther.uTicks;
                }
#endif

                constexpr TickCount& operator+=(const TimeSpan& _nSpan) noexcept
                {
                    // uTicks += _nSpan.GetTicks() * GetTicksPerSecond() / TimeSpan::GetTicksPerSecond();
                    uTicks += UMulDiv64Fast(_nSpan.GetTicks(), GetTicksPerSecond(), TimeSpan::GetTicksPerSecond());
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
                    // uTicks -= _nSpan.GetTicks() * GetTicksPerSecond() / TimeSpan::GetTicksPerSecond();
                    uTicks -= UMulDiv64Fast(_nSpan.GetTicks(), GetTicksPerSecond(), TimeSpan::GetTicksPerSecond());
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
                    int64_t _nSpanInternal = uTicks - _oOther.uTicks;
                    // return TimeSpan::FromTicks(_nSpanInternal * TimeSpan::GetTicksPerSecond() / int64_t(GetTicksPerSecond()));
                    return TimeSpan::FromTicks(MulDiv64Fast(_nSpanInternal, TimeSpan::GetTicksPerSecond(), GetTicksPerSecond()));
                }
            };
        }
    }
}

#pragma pack(pop)
