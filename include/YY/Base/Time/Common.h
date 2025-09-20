#pragma once

namespace YY
{
    namespace Base
    {
        namespace Time
        {
            constexpr int64_t kNanosecondsPerMicrosecond = 1000000;
            constexpr int64_t kMicrosecondsPerMillisecond = 1000;
            constexpr int64_t kMillisecondsPerSecond = 1000;
            constexpr int64_t kSecondsPerMinute = 60;
            constexpr int64_t kMinutesPerHour = 60;
            constexpr int64_t kHoursPerDay = 24;
        }
    }
}

namespace YY
{
    using namespace YY::Base::Time;
}
