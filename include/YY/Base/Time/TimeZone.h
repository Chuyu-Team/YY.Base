#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Memory/RefPtr.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Time
        {
            class TimeZone
                : public YY::RefValue
#if defined(_WIN32)
                , public DYNAMIC_TIME_ZONE_INFORMATION
#endif
            {
            private:
                TimeZone() = default;

            public:
#if defined(_WIN32)
                TimeZone(const DYNAMIC_TIME_ZONE_INFORMATION& _oTimeZoneInfo)
                    : DYNAMIC_TIME_ZONE_INFORMATION(_oTimeZoneInfo)
                {
                }
#endif

                static YY::RefPtr<const TimeZone> __YYAPI GetCurrentTimeZone();
            };
        }
    }

    using namespace YY::Base::Time;
}

#pragma pack(pop)
