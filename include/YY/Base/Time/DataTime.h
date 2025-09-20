#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Time/Common.h>
#include <YY/Base/Time/TimeSpan.h>
#include <YY/Base/Utils/MathUtils.h>
#include <YY/Base/Containers/Optional.h>
#include <YY/Base/Time/TimeZone.h>

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
            class LocalDataTime;

            class UtcDataTime
            {
            public:
                union
                {
                    uint64_t uTicks = UINT64_MAX;
                    FILETIME oFileTime;
                };

                static constexpr int64_t kTicksPerMicrosecond = 10;

                constexpr UtcDataTime() = default;

                explicit constexpr UtcDataTime(const FILETIME& oFileTime)
                    : oFileTime(oFileTime)
                {
                }

                explicit UtcDataTime(const SYSTEMTIME& _oSystemTime)
                {
                    SystemTimeToFileTime(&_oSystemTime, &oFileTime);
                }

                static UtcDataTime __YYAPI GetNow()
                {
                    UtcDataTime _oTime;
                    GetSystemTimePreciseAsFileTime(&_oTime.oFileTime);
                    return _oTime;
                }

                constexpr static uint64_t __YYAPI GetTicksPerSecond() noexcept
                {
                    // 100纳秒间隔
                    return kMillisecondsPerSecond * kMicrosecondsPerMillisecond * kTicksPerMicrosecond;
                }

                static constexpr UtcDataTime __YYAPI FromFileTime(const FILETIME& _oFileTime)
                {
                    UtcDataTime _oTime(_oFileTime);
                    return _oTime;
                }

                static UtcDataTime __YYAPI FromSystemTime(const SYSTEMTIME& _oSystemTime)
                {
                    UtcDataTime _oTime(_oSystemTime);
                    return _oTime;
                }

                static constexpr UtcDataTime __YYAPI FromUnixTimestamp(time_t _oUnixTimestamp)
                {
                    UtcDataTime _oTime;
                    if (_oUnixTimestamp != -1)
                    {
                        _oTime.uTicks = uint64_t(_oUnixTimestamp) * 10000000ULL + 116444736000000000ULL;
                    }
                    return _oTime;
                }

                SYSTEMTIME __YYAPI ToSystemTime() const;

                LocalDataTime __YYAPI ToLocalDataTime(_In_opt_ YY::RefPtr<const TimeZone> _pTimeZone = TimeZone::GetCurrentTimeZone()) const;

                constexpr time_t __YYAPI ToUnixTimestamp() const
                {
                    if (uTicks >= 0x8000000000000000ULL)
                        return -1;

                    if (uTicks < 116444736000000000ULL)
                        return -1;

                    return (uTicks - 116444736000000000ULL) / 10000000ULL;
                }

                constexpr UtcDataTime& operator=(const UtcDataTime&) noexcept = default;

                constexpr bool operator==(const UtcDataTime& _oOther) const noexcept
                {
                    return uTicks == _oOther.uTicks;
                }

#if defined(_HAS_CXX20) && _HAS_CXX20
                constexpr auto operator<=>(const UtcDataTime& _oOther) const noexcept
                {
                    return uTicks <=> _oOther.uTicks;
                }
#else
                constexpr bool operator>(const UtcDataTime& _oOther) const noexcept
                {
                    return uTicks > _oOther.uTicks;
                }

                constexpr bool operator>=(const UtcDataTime& _oOther) const noexcept
                {
                    return uTicks >= _oOther.uTicks;
                }

                constexpr bool operator<=(const UtcDataTime& _oOther) const noexcept
                {
                    return uTicks <= _oOther.uTicks;
                }

                constexpr bool operator<(const UtcDataTime& _oOther) const noexcept
                {
                    return uTicks < _oOther.uTicks;
                }

                constexpr bool operator!=(const UtcDataTime& _oOther) const noexcept
                {
                    return uTicks != _oOther.uTicks;
                }
#endif

                constexpr UtcDataTime& operator+=(const TimeSpan& _nSpan) noexcept
                {
                    uTicks += _nSpan.GetTicks();
                    return *this;
                }

                constexpr UtcDataTime operator+(const TimeSpan& _nSpan) noexcept
                {
                    UtcDataTime _oTmp = *this;
                    _oTmp += _nSpan;
                    return _oTmp;
                }

                constexpr UtcDataTime& operator-=(const TimeSpan& _nSpan) noexcept
                {
                    uTicks -= _nSpan.GetTicks();
                    return *this;
                }

                constexpr UtcDataTime operator-(const TimeSpan& _nSpan) noexcept
                {
                    UtcDataTime _oTmp = *this;
                    _oTmp -= _nSpan;
                    return _oTmp;
                }

                constexpr TimeSpan operator-(const UtcDataTime& _oOther) const noexcept
                {
                    return TimeSpan::FromTicks(uTicks - _oOther.uTicks);
                }
            };

            class LocalDataTime
            {
            public:
                union
                {
                    uint64_t uTicks = UINT64_MAX;
                    FILETIME oFileTime;
                };

                static constexpr int64_t kTicksPerMicrosecond = 10;

                YY::RefPtr<const TimeZone> pTimeZone;

                constexpr LocalDataTime() = default;

                explicit LocalDataTime(_In_ YY::RefPtr<const TimeZone> _pTimeZone)
                    : pTimeZone(std::move(_pTimeZone))
                {
                }

                explicit LocalDataTime(const FILETIME& oFileTime, _In_ YY::RefPtr<const TimeZone> _pTimeZone = TimeZone::GetCurrentTimeZone())
                    : oFileTime(oFileTime)
                    , pTimeZone(std::move(_pTimeZone))
                {
                }

                explicit LocalDataTime(const SYSTEMTIME& _oSystemTime, _In_ YY::RefPtr<const TimeZone> _pTimeZone = TimeZone::GetCurrentTimeZone())
                    : pTimeZone(std::move(_pTimeZone))
                {
                    SystemTimeToFileTime(&_oSystemTime, &oFileTime);
                }

                LocalDataTime(LocalDataTime&&) noexcept = default;

                LocalDataTime(const LocalDataTime&) = default;

                static LocalDataTime __YYAPI GetNow(_In_opt_ YY::RefPtr<const TimeZone> pTimeZone = TimeZone::GetCurrentTimeZone())
                {
                    return UtcDataTime::GetNow().ToLocalDataTime(pTimeZone);
                }

                constexpr static uint64_t __YYAPI GetTicksPerSecond() noexcept
                {
                    // 100纳秒间隔
                    return kMillisecondsPerSecond * kMicrosecondsPerMillisecond * kTicksPerMicrosecond;
                }

                static LocalDataTime __YYAPI FromFileTime(const FILETIME& _oFileTime, _In_ YY::RefPtr<const TimeZone> _pTimeZone = TimeZone::GetCurrentTimeZone()) noexcept
                {
                    LocalDataTime _oTime(_oFileTime, std::move(_pTimeZone));
                    return _oTime;
                }

                static LocalDataTime __YYAPI FromSystemTime(const SYSTEMTIME& _oSystemTime, _In_ YY::RefPtr<const TimeZone> _pTimeZone = TimeZone::GetCurrentTimeZone())
                {
                    LocalDataTime _oTime(_oSystemTime, std::move(_pTimeZone));
                    return _oTime;
                }

                static LocalDataTime __YYAPI FromUnixTimestamp(time_t _oCrtTime, _In_ YY::RefPtr<const TimeZone> _pTimeZone = TimeZone::GetCurrentTimeZone())
                {
                    LocalDataTime _oTime;
                    if (_oCrtTime != -1)
                    {
                        _oTime.uTicks = uint64_t(_oCrtTime) * 10000000LL + 116444736000000000LL;
                    }

                    _oTime.pTimeZone = std::move(_pTimeZone);
                    return _oTime;
                }

                SYSTEMTIME __YYAPI ToSystemTime() const;

                UtcDataTime __YYAPI ToUtcDataTime() const;

                constexpr time_t __YYAPI ToUnixTimestamp() const
                {
                    if (uTicks >= 0x8000000000000000ULL)
                        return -1;

                    if (uTicks < 116444736000000000ULL)
                        return -1;

                    return (uTicks - 116444736000000000ULL) / 10000000ULL;
                }

                constexpr LocalDataTime& operator=(const LocalDataTime&) noexcept = default;

                constexpr bool operator==(const LocalDataTime& _oOther) const noexcept
                {
                    return uTicks == _oOther.uTicks;
                }

#if defined(_HAS_CXX20) && _HAS_CXX20
                constexpr auto operator<=>(const LocalDataTime& _oOther) const noexcept
                {
                    return uTicks <=> _oOther.uTicks;
                }
#else
                constexpr bool operator>(const LocalDataTime& _oOther) const noexcept
                {
                    return uTicks > _oOther.uTicks;
                }

                constexpr bool operator>=(const LocalDataTime& _oOther) const noexcept
                {
                    return uTicks >= _oOther.uTicks;
                }

                constexpr bool operator<=(const LocalDataTime& _oOther) const noexcept
                {
                    return uTicks <= _oOther.uTicks;
                }

                constexpr bool operator<(const LocalDataTime& _oOther) const noexcept
                {
                    return uTicks < _oOther.uTicks;
                }

                constexpr bool operator!=(const LocalDataTime& _oOther) const noexcept
                {
                    return uTicks != _oOther.uTicks;
                }
#endif

                constexpr LocalDataTime& operator+=(const TimeSpan& _nSpan) noexcept
                {
                    uTicks += _nSpan.GetTicks();
                    return *this;
                }

                LocalDataTime operator+(const TimeSpan& _nSpan) noexcept
                {
                    LocalDataTime _oTmp = *this;
                    _oTmp += _nSpan;
                    return _oTmp;
                }

                constexpr LocalDataTime& operator-=(const TimeSpan& _nSpan) noexcept
                {
                    uTicks -= _nSpan.GetTicks();
                    return *this;
                }

                LocalDataTime operator-(const TimeSpan& _nSpan) noexcept
                {
                    LocalDataTime _oTmp = *this;
                    _oTmp -= _nSpan;
                    return _oTmp;
                }

                constexpr TimeSpan operator-(const LocalDataTime& _oOther) const noexcept
                {
                    return TimeSpan::FromTicks(uTicks - _oOther.uTicks);
                }
            };
        }
    }
}

namespace YY
{
    using namespace YY::Base::Time;
}

#pragma pack(pop)
