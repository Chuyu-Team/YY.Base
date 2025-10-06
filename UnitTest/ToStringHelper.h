#pragma once
#include "CppUnitTest.h"

#include <YY/Base/Strings/String.h>
#include <YY/Base/Containers/Span.h>
#include <string>

namespace Microsoft
{
    namespace VisualStudio
    {
        namespace CppUnitTestFramework
        {
            template<>
            inline std::wstring ToString<YY::uString>(const YY::uString& _oValue)
            {
                return std::wstring(_oValue.GetData(), _oValue.GetLength());
            }

            template<>
            inline std::wstring ToString<YY::uStringView>(const YY::uStringView& _oValue)
            {
                return std::wstring(_oValue.GetData(), _oValue.GetLength());
            }

            template<>
            inline std::wstring ToString<YY::Span<const YY::uchar_t>>(const YY::Span<const YY::uchar_t>& _oValue)
            {
                return std::wstring(_oValue.GetData(), _oValue.GetLength());
            }
        }
    }
}
