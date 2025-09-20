#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Time/Common.h>
#include <YY/Base/Time/TimeSpan.h>
#include <YY/Base/Utils/MathUtils.h>
#include <YY/Base/Containers/Optional.h>

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
                    FILETIME oFileTime = {};
                    uint64_t uTimeInternalValue;
                };

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

                constexpr static uint64_t __YYAPI GetSecondsPerInternal() noexcept
                {
                    // 100纳秒间隔
                    return SecondsPerMillisecond * MillisecondsPerMicrosecond * 10;
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

                static constexpr UtcDataTime __YYAPI FromCrtTime(time_t _oCrtTime)
                {
                    UtcDataTime _oTime;
                    _oTime.uTimeInternalValue = uint64_t(_oCrtTime) * 10000000ULL + 116444736000000000ULL;
                    return _oTime;
                }

                SYSTEMTIME __YYAPI GetSystemTime() const
                {
                    SYSTEMTIME _oSystemTime;
                    FileTimeToSystemTime(&oFileTime, &_oSystemTime);
                    return _oSystemTime;
                }

                LocalDataTime __YYAPI GetLocalDataTime(_In_opt_ const DYNAMIC_TIME_ZONE_INFORMATION* _pTimeZoneInformation = nullptr) const;

                constexpr time_t __YYAPI GetCrtTime() const
                {
                    if (uTimeInternalValue < 116444736000000000ULL)
                        return -1;

                    return (uTimeInternalValue - 116444736000000000ULL) / 10000000ULL;
                }

                constexpr UtcDataTime& operator=(const UtcDataTime&) noexcept = default;

                constexpr bool operator==(const UtcDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue == _oOther.uTimeInternalValue;
                }

#if defined(_HAS_CXX20) && _HAS_CXX20
                constexpr auto operator<=>(const UtcDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue <=> _oOther.uTimeInternalValue;
                }
#else
                constexpr bool operator>(const UtcDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue > _oOther.uTimeInternalValue;
                }

                constexpr bool operator>=(const UtcDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue >= _oOther.uTimeInternalValue;
                }

                constexpr bool operator<=(const UtcDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue <= _oOther.uTimeInternalValue;
                }

                constexpr bool operator<(const UtcDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue < _oOther.uTimeInternalValue;
                }

                constexpr bool operator!=(const UtcDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue != _oOther.uTimeInternalValue;
                }
#endif

                constexpr UtcDataTime& operator+=(const TimeSpan& _nSpan) noexcept
                {
                    // uTimeInternalValue += _nSpan.GetInternalValue() * GetSecondsPerInternal() / TimeSpan::GetSecondsPerInternal();
                    uTimeInternalValue += UMulDiv64Fast(_nSpan.GetInternalValue(), GetSecondsPerInternal(), TimeSpan::GetSecondsPerInternal());
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
                    // uTimeInternalValue -= _nSpan.GetInternalValue() * GetSecondsPerInternal() / TimeSpan::GetSecondsPerInternal();
                    uTimeInternalValue -= UMulDiv64Fast(_nSpan.GetInternalValue(), GetSecondsPerInternal(), TimeSpan::GetSecondsPerInternal());
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
                    int64_t _nSpanInternal = uTimeInternalValue - _oOther.uTimeInternalValue;
                    // return TimeSpan::FromInternalValue(_nSpanInternal * TimeSpan::GetSecondsPerInternal() / int64_t(GetSecondsPerInternal()));
                    return TimeSpan::FromInternalValue(MulDiv64Fast(_nSpanInternal, TimeSpan::GetSecondsPerInternal(), GetSecondsPerInternal()));
                }
            };

            class LocalDataTime
            {
            public:
                union
                {
                    FILETIME oFileTime = {};
                    uint64_t uTimeInternalValue;
                };

                constexpr LocalDataTime() = default;

                explicit constexpr LocalDataTime(const FILETIME& oFileTime)
                    : oFileTime(oFileTime)
                {
                }

                explicit LocalDataTime(const SYSTEMTIME& _oSystemTime)
                {
                    SystemTimeToFileTime(&_oSystemTime, &oFileTime);
                }

                static LocalDataTime __YYAPI GetNow(_In_opt_ const DYNAMIC_TIME_ZONE_INFORMATION* _pTimeZoneInformation = nullptr)
                {
                    return UtcDataTime::GetNow().GetLocalDataTime(_pTimeZoneInformation);
                }

                constexpr static uint64_t __YYAPI GetSecondsPerInternal() noexcept
                {
                    // 100纳秒间隔
                    return SecondsPerMillisecond * MillisecondsPerMicrosecond * 10;
                }

                static constexpr LocalDataTime __YYAPI FromFileTime(const FILETIME& _oFileTime) noexcept
                {
                    LocalDataTime _oTime(_oFileTime);
                    return _oTime;
                }

                static LocalDataTime __YYAPI FromSystemTime(const SYSTEMTIME& _oSystemTime)
                {
                    LocalDataTime _oTime(_oSystemTime);
                    return _oTime;
                }

                static constexpr LocalDataTime __YYAPI FromCrtTime(time_t _oCrtTime)
                {
                    LocalDataTime _oTime;
                    _oTime.uTimeInternalValue = uint64_t(_oCrtTime) * 10000000LL + 116444736000000000LL;
                    return _oTime;
                }

                SYSTEMTIME __YYAPI GetSystemTime() const
                {
                    SYSTEMTIME _oSystemTime;
                    FileTimeToSystemTime(&oFileTime, &_oSystemTime);
                    return _oSystemTime;
                }

                UtcDataTime __YYAPI GetUtcDataTime(_In_opt_ const DYNAMIC_TIME_ZONE_INFORMATION* _pTimeZoneInformation = nullptr) const;

                constexpr time_t __YYAPI GetCrtTime() const
                {
                    if (uTimeInternalValue < 116444736000000000ULL)
                        return -1;

                    return (uTimeInternalValue - 116444736000000000ULL) / 10000000ULL;
                }

                constexpr LocalDataTime& operator=(const LocalDataTime&) noexcept = default;

                constexpr bool operator==(const LocalDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue == _oOther.uTimeInternalValue;
                }

#if defined(_HAS_CXX20) && _HAS_CXX20
                constexpr auto operator<=>(const LocalDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue <=> _oOther.uTimeInternalValue;
                }
#else
                constexpr bool operator>(const LocalDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue > _oOther.uTimeInternalValue;
                }

                constexpr bool operator>=(const LocalDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue >= _oOther.uTimeInternalValue;
                }

                constexpr bool operator<=(const LocalDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue <= _oOther.uTimeInternalValue;
                }

                constexpr bool operator<(const LocalDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue < _oOther.uTimeInternalValue;
                }

                constexpr bool operator!=(const LocalDataTime& _oOther) const noexcept
                {
                    return uTimeInternalValue != _oOther.uTimeInternalValue;
                }
#endif

                constexpr LocalDataTime& operator+=(const TimeSpan& _nSpan) noexcept
                {
                    // uTimeInternalValue += _nSpan.GetInternalValue() * GetSecondsPerInternal() / TimeSpan::GetSecondsPerInternal();
                    uTimeInternalValue += UMulDiv64Fast(_nSpan.GetInternalValue(), GetSecondsPerInternal(), TimeSpan::GetSecondsPerInternal());
                    return *this;
                }

                constexpr LocalDataTime operator+(const TimeSpan& _nSpan) noexcept
                {
                    LocalDataTime _oTmp = *this;
                    _oTmp += _nSpan;
                    return _oTmp;
                }

                constexpr LocalDataTime& operator-=(const TimeSpan& _nSpan) noexcept
                {
                    // uTimeInternalValue -= _nSpan.GetInternalValue() * GetSecondsPerInternal() / TimeSpan::GetSecondsPerInternal();
                    uTimeInternalValue -= UMulDiv64Fast(_nSpan.GetInternalValue(), GetSecondsPerInternal(), TimeSpan::GetSecondsPerInternal());
                    return *this;
                }

                constexpr LocalDataTime operator-(const TimeSpan& _nSpan) noexcept
                {
                    LocalDataTime _oTmp = *this;
                    _oTmp -= _nSpan;
                    return _oTmp;
                }

                constexpr TimeSpan operator-(const LocalDataTime& _oOther) const noexcept
                {
                    int64_t _nSpanInternal = uTimeInternalValue - _oOther.uTimeInternalValue;
                    // return TimeSpan::FromInternalValue(_nSpanInternal * TimeSpan::GetSecondsPerInternal() / int64_t(GetSecondsPerInternal()));
                    return TimeSpan::FromInternalValue(MulDiv64Fast(_nSpanInternal, TimeSpan::GetSecondsPerInternal(), GetSecondsPerInternal()));
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
