﻿#define _Disallow_YY_KM_Namespace

#include <YY/Base/Utils/SystemInfo.h>

#define WIN32_NO_STATUS
#include <YY/Base/Shared/Windows/km.h>

namespace YY
{
    namespace Base
    {
        namespace Utils
        {
#if defined(_WIN32)
            Version __YYAPI YY::Base::Utils::GetOperatingSystemVersion() noexcept
            {
                const auto _pPeb = ((TEB*)NtCurrentTeb())->ProcessEnvironmentBlock;
                Version _uOsVersion(uint16_t(_pPeb->OSMajorVersion), uint16_t(_pPeb->OSMinorVersion), uint16_t(_pPeb->OSBuildNumber));
                
                if (_uOsVersion == kWindowsNT5_1)
                {
                    switch (HIBYTE(_pPeb->OSCSDVersion))
                    {
                    case 0:
                        break;
                    case 1:
                        _uOsVersion.uRevision = 1106;
                        break;
                    case 2:
                        _uOsVersion.uRevision = 2180;
                        break;
                    case 3:
                    default:
                        _uOsVersion.uRevision = 5512;
                        break;
                    }
                }
                else if(_uOsVersion == kWindowsNT5_2)
                {
                    switch (HIBYTE(_pPeb->OSCSDVersion))
                    {
                    case 0:
                        break;
                    case 1:
                    default:
                        _uOsVersion.uRevision = 1830;
                        break;
                    }
                }

                return _uOsVersion;
            }
#endif
        }
    }
}
