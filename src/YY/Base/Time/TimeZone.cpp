#include <YY/Base/Time/TimeZone.h>

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

namespace YY
{
    namespace Base
    {
        namespace Time
        {
            static TimeZone* s_pCachedCurrentTimeZone = nullptr;

            YY::RefPtr<const TimeZone>__YYAPI Base::Time::TimeZone::GetCurrentTimeZone()
            {
                auto _pTimeZone = s_pCachedCurrentTimeZone;
                do
                {
                    if (_pTimeZone)
                    {
                        break;
                    }

                    _pTimeZone = new TimeZone();
                    if (GetDynamicTimeZoneInformation(_pTimeZone) == TIME_ZONE_ID_INVALID)
                    {
                        _pTimeZone->Release();
                        return nullptr;
                    }

                    auto _pLast = YY::Sync::CompareExchangePoint(&s_pCachedCurrentTimeZone, _pTimeZone, nullptr);
                    if (_pLast)
                    {
                        _pTimeZone->Release();
                        _pTimeZone = _pLast;
                        break;
                    }

                    atexit([]()
                        {
                            auto _pTimeZone = YY::Sync::ExchangePoint(&s_pCachedCurrentTimeZone, nullptr);
                            if (_pTimeZone)
                            {
                                _pTimeZone->Release();
                            }
                        });

                } while (false);

                return YY::RefPtr<const TimeZone>(_pTimeZone);
            }
        }
    }
}
