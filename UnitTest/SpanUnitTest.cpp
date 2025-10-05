#include "CppUnitTest.h"

#include <atlstr.h>
#include <Windows.h>
#include <tchar.h>
#include <string>
#include <atltypes.h>

#include <YY/Base/Containers/Span.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace YY;

namespace Microsoft::VisualStudio::CppUnitTestFramework
{
    template<>
    inline std::wstring ToString<YY::Span<const YY::uchar_t>>(const YY::Span<const YY::uchar_t>& _oValue) 
    {
        std::wstring _szResult;
        _szResult += L'{';

        _szResult.append(_oValue.GetData(), _oValue.GetLength());

        _szResult += L'}';

        return _szResult;
    }
}

namespace UnitTest
{
    TEST_CLASS(Span)
    {
    public:

        TEST_METHOD(IndexOf)
        {
            const YY::Span<const YY::uchar_t> _sTest(_S("0123456789"), 10);
            {
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('0')), size_t(0));
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('7')), size_t(7));
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('9')), size_t(9));
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('a')), YY::kuInvalidIndex);

                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('0'), 1), YY::kuInvalidIndex);
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('7'), 1), size_t(7));
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('9'), 1), size_t(9));
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('a'), 1), YY::kuInvalidIndex);

                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('0'), 1, 7), YY::kuInvalidIndex);
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('7'), 1, 6), YY::kuInvalidIndex);
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('7'), 1, 7), size_t(7));
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('9'), 1, 8), YY::kuInvalidIndex);
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('9'), 1, 9), size_t(9));
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('9'), 1, 10), size_t(9));
                Assert::AreEqual(_sTest.IndexOf(YY::uchar_t('a'), 1), YY::kuInvalidIndex);

            }

            {
                Assert::AreEqual(_sTest.IndexOf(YY::Span<const YY::uchar_t>(_S("012"), 3)), size_t(0));
                Assert::AreEqual(_sTest.IndexOf(YY::Span<const YY::uchar_t>(_S("123"), 3)), size_t(1));
                Assert::AreEqual(_sTest.IndexOf(YY::Span<const YY::uchar_t>(_S("789"), 3)), size_t(7));
                Assert::AreEqual(_sTest.IndexOf(YY::Span<const YY::uchar_t>(_S("890"), 3)), YY::kuInvalidIndex);


                Assert::AreEqual(_sTest.IndexOf(YY::Span<const YY::uchar_t>(_S("12"), 2), 0, 2), YY::kuInvalidIndex);
                Assert::AreEqual(_sTest.IndexOf(YY::Span<const YY::uchar_t>(_S("12"), 2), 0, 3), size_t(1));
                Assert::AreEqual(_sTest.IndexOf(YY::Span<const YY::uchar_t>(_S("12"), 2), 1, 3), size_t(1));
            }
        }

        TEST_METHOD(IndexOfAny)
        {
            YY::Span<const YY::uchar_t> _sTest(_S("0123456789"), 10);

            {
                Assert::AreEqual(_sTest.IndexOfAny(YY::Span<const YY::uchar_t>(_S("0123456789"), 10)), size_t(0));
                Assert::AreEqual(_sTest.IndexOfAny(YY::Span<const YY::uchar_t>(_S("123456789"), 9)), size_t(1));
                Assert::AreEqual(_sTest.IndexOfAny(YY::Span<const YY::uchar_t>(_S("9"), 1)), size_t(9));
                Assert::AreEqual(_sTest.IndexOfAny(YY::Span<const YY::uchar_t>(_S("abcd"), 4)), YY::kuInvalidIndex);
            }
        }

        TEST_METHOD(SplitAndTakeFirst)
        {
            YY::Span<const YY::uchar_t> _szTemp(_S("01|34|6789"), 10);

            size_t _uNextIndex = 0;
            Assert::AreEqual(_szTemp.SplitAndTakeFirst(YY::uchar_t('|'), _uNextIndex, &_uNextIndex), YY::Span<const YY::uchar_t>(_S("01"), 2));
            Assert::AreEqual(_uNextIndex, size_t(3));

            Assert::AreEqual(_szTemp.SplitAndTakeFirst(YY::uchar_t('|'), _uNextIndex, &_uNextIndex), YY::Span<const YY::uchar_t>(_S("34"), 2));
            Assert::AreEqual(_uNextIndex, size_t(6));

            Assert::AreEqual(_szTemp.SplitAndTakeFirst(YY::uchar_t('|'), _uNextIndex, &_uNextIndex), YY::Span<const YY::uchar_t>(_S("6789"), 4));
            Assert::AreEqual(_uNextIndex, size_t(10));
        }
    };
}
