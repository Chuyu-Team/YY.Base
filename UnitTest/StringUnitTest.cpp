#include "CppUnitTest.h"

#include <atlstr.h>
#include <Windows.h>
#include <tchar.h>
#include <string>
#include <atltypes.h>

#include <YY/Base/Strings/NString.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace YY;

namespace Microsoft::VisualStudio::CppUnitTestFramework
{
    template<>
    inline std::wstring ToString<YY::uString>(const YY::uString& _oValue)
    {
        std::wstring _szResult;
        _szResult += L'{';

        _szResult.append(_oValue.GetData(), _oValue.GetLength());

        _szResult += L'}';

        return _szResult;
    }

    template<>
    inline std::wstring ToString<YY::uStringView>(const YY::uStringView& _oValue)
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
	TEST_CLASS(StringUnitTest)
	{
	public:
		
		TEST_METHOD(从原生指针构造)
		{
			{
#define _TEST_TEXT "从原生指针构造"

				aString Tmp(_TEST_TEXT);
				Assert::AreEqual(Tmp.GetSize(), _countof(_TEST_TEXT) - 1);
				Assert::IsTrue(memcmp(Tmp.GetConstString(), _TEST_TEXT, sizeof(_TEST_TEXT)) == 0);



				NString Tmp2(_TEST_TEXT);
				Assert::AreEqual(Tmp2.GetSize(), _countof(_TEST_TEXT) - 1);
				Assert::IsTrue(Tmp2.GetEncoding() == Encoding::ANSI);
				Assert::IsTrue(memcmp(*(void**)&Tmp2, _TEST_TEXT, sizeof(_TEST_TEXT)) == 0);
#undef _TEST_TEXT
			}

			{
#define _TEST_TEXT _U8S("从原生指针构造")

				u8String Tmp(_TEST_TEXT);
				Assert::AreEqual(Tmp.GetSize(), _countof(_TEST_TEXT) - 1);
				Assert::IsTrue(memcmp(Tmp.GetConstString(), _TEST_TEXT, sizeof(_TEST_TEXT)) == 0);


				NString Tmp2(_TEST_TEXT);
				Assert::AreEqual(Tmp2.GetSize(), _countof(_TEST_TEXT) - 1);
				Assert::IsTrue(Tmp2.GetEncoding() == Encoding::UTF8);
				Assert::IsTrue(memcmp(*(void**)&Tmp2, _TEST_TEXT, sizeof(_TEST_TEXT)) == 0);
#undef _TEST_TEXT
			}

			{
#define _TEST_TEXT _U16S("从原生指针构造")

				u16String Tmp(_TEST_TEXT);
				Assert::AreEqual(Tmp.GetSize(), _countof(_TEST_TEXT) - 1);
				Assert::IsTrue(memcmp(Tmp.GetConstString(), _TEST_TEXT, sizeof(_TEST_TEXT)) == 0);


				NString Tmp2(_TEST_TEXT);
				Assert::AreEqual(Tmp2.GetSize(), _countof(_TEST_TEXT) - 1);
				Assert::IsTrue(Tmp2.GetEncoding() == Encoding::UTF16);
				Assert::IsTrue(memcmp(*(void**)&Tmp2, _TEST_TEXT, sizeof(_TEST_TEXT)) == 0);
#undef _TEST_TEXT
			}

			{
#define _TEST_TEXT _U32S("从原生指针构造")

				u32String Tmp(_TEST_TEXT);
				Assert::AreEqual(Tmp.GetSize(), _countof(_TEST_TEXT) - 1);
				Assert::IsTrue(memcmp(Tmp.GetConstString(), _TEST_TEXT, sizeof(_TEST_TEXT)) == 0);


				NString Tmp2(_TEST_TEXT);
				Assert::AreEqual(Tmp2.GetSize(), _countof(_TEST_TEXT) - 1);
				Assert::IsTrue(Tmp2.GetEncoding() == Encoding::UTF32);
				Assert::IsTrue(memcmp(*(void**)&Tmp2, _TEST_TEXT, sizeof(_TEST_TEXT)) == 0);
#undef _TEST_TEXT
			}
		}


		TEST_METHOD(引用计数能力验证)
		{
			u16String Tmp(_U16S("一段测试文本"));

			auto p1 = Tmp.GetConstString();

			u16String Tmp2 = Tmp;

			auto p2 = Tmp.GetConstString();
			auto p3 = Tmp2.GetConstString();


			Assert::IsTrue(p1 == p2);
			Assert::IsTrue(p1 == p3);
		}

		TEST_METHOD(LockBuffer复制验证)
		{
			// 这个缓冲区只共享一份，所以 LockBuffer，前后指针不变
			{
				u16String Tmp(_U16S("一段测试文本"));
				auto p1 = Tmp.GetConstString();

				auto p2 = Tmp.LockBuffer(Tmp.GetSize());
				Tmp.UnlockBuffer(Tmp.GetSize());

				Assert::AreEqual(Tmp.GetSize(), _countof(_U16S("一段测试文本")) - 1);
				Assert::IsTrue(memcmp(Tmp.GetConstString(), _U16S("一段测试文本"), sizeof(_U16S("一段测试文本"))) == 0);

				Assert::IsTrue(p1 == p2);
			}


			// 缓冲区存在，共享，所以 LockBuffer 后指针会重新开辟
			{
				u16String Tmp(_U16S("一段测试文本"));
				auto p1 = Tmp.GetConstString();

				u16String Tmp2 = Tmp;
				auto p2 = Tmp2.LockBuffer(Tmp.GetSize());
				Tmp2.UnlockBuffer(Tmp.GetSize());

				Assert::AreEqual(Tmp.GetSize(), _countof(_U16S("一段测试文本")) - 1);
				Assert::AreEqual(Tmp2.GetSize(), _countof(_U16S("一段测试文本")) - 1);

				Assert::IsTrue(memcmp(Tmp.GetConstString(), _U16S("一段测试文本"), sizeof(_U16S("一段测试文本"))) == 0);
				Assert::IsTrue(memcmp(Tmp2.GetConstString(), _U16S("一段测试文本"), sizeof(_U16S("一段测试文本"))) == 0);

				Assert::IsTrue(p1 != p2);
			}

		}

		TEST_METHOD(写复制能力验证)
		{
			{
				u16String Tmp(_U16S("一段测试文本"));
				auto p1 = Tmp.GetConstString();

				u16String Tmp2 = Tmp;
				Tmp2 += _U16S("2");
				auto p2 = Tmp.GetConstString();
				auto p3 = Tmp2.GetConstString();

				Assert::AreEqual(Tmp.GetSize(), _countof(_U16S("一段测试文本")) - 1);
				Assert::AreEqual(Tmp2.GetSize(), _countof(_U16S("一段测试文本2")) - 1);

				Assert::IsTrue(memcmp(Tmp.GetConstString(), _U16S("一段测试文本"), sizeof(_U16S("一段测试文本"))) == 0);
				Assert::IsTrue(memcmp(Tmp2.GetConstString(), _U16S("一段测试文本2"), sizeof(_U16S("一段测试文本2"))) == 0);

				Assert::IsTrue(p1 == p2);
				Assert::IsTrue(p1 != p3);
			}
		}

        TEST_METHOD(Remove)
        {
            YY::uString _szTemp(_S("0123456789"));

            _szTemp.Remove(0, 2);
            Assert::AreEqual(_szTemp, _S("23456789"));

            _szTemp.Remove(2, 2);
            Assert::AreEqual(_szTemp, _S("236789"));

            _szTemp.Remove(5, 2);
            Assert::AreEqual(_szTemp, _S("23678"));

            _szTemp.Remove(5, 2);
            Assert::AreEqual(_szTemp, _S("23678"));

            _szTemp.Remove(0);
            Assert::AreEqual(_szTemp, _S(""));
        }

        TEST_METHOD(Substring)
        {
            YY::uString _szTemp(_S("0123456789"));

            Assert::AreEqual(_szTemp.Substring(0, 2), _S("01"));
            Assert::AreEqual(_szTemp.Substring(1, 2), _S("12"));
            Assert::AreEqual(_szTemp.Substring(8, 2), _S("89"));
            Assert::AreEqual(_szTemp.Substring(9, 2), _S("9"));
            Assert::AreEqual(_szTemp.Substring(10, 2), _S(""));
        }

        TEST_METHOD(SplitAndTakeFirst)
        {
            YY::uString _szTemp(_S("01|34|6789"));

            size_t _uNextIndex = 0;
            Assert::AreEqual(_szTemp.SplitAndTakeFirst(YY::uString::char_t('|'), _uNextIndex, &_uNextIndex), _S("01"));
            Assert::AreEqual(_uNextIndex, size_t(3));

            Assert::AreEqual(_szTemp.SplitAndTakeFirst(YY::uString::char_t('|'), _uNextIndex, &_uNextIndex), _S("34"));
            Assert::AreEqual(_uNextIndex, size_t(6));

            Assert::AreEqual(_szTemp.SplitAndTakeFirst(YY::uString::char_t('|'), _uNextIndex, &_uNextIndex), _S("6789"));
            Assert::AreEqual(_uNextIndex, size_t(10));
        }
	};

    TEST_CLASS(StringView)
    {
    public:

        TEST_METHOD(IndexOf)
        {
            YY::uStringView _sTest(_S("0123456789"));
            {
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('0')), size_t(0));
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('7')), size_t(7));
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('9')), size_t(9));
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('a')), YY::kuInvalidIndex);

                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('0'), 1), YY::kuInvalidIndex);
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('7'), 1), size_t(7));
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('9'), 1), size_t(9));
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('a'), 1), YY::kuInvalidIndex);

                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('0'), 1, 7), YY::kuInvalidIndex);
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('7'), 1, 6), YY::kuInvalidIndex);
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('7'), 1, 7), size_t(7));
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('9'), 1, 8), YY::kuInvalidIndex);
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('9'), 1, 9), size_t(9));
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('9'), 1, 10), size_t(9));
                Assert::AreEqual(_sTest.IndexOf(YY::uStringView::char_t('a'), 1), YY::kuInvalidIndex);

            }

            {
                Assert::AreEqual(_sTest.IndexOf(_S("012")), size_t(0));
                Assert::AreEqual(_sTest.IndexOf(_S("123")), size_t(1));
                Assert::AreEqual(_sTest.IndexOf(_S("789")), size_t(7));
                Assert::AreEqual(_sTest.IndexOf(_S("890")), YY::kuInvalidIndex);


                Assert::AreEqual(_sTest.IndexOf(_S("12"), 0, 2), YY::kuInvalidIndex);
                Assert::AreEqual(_sTest.IndexOf(_S("12"), 0, 3), size_t(1));
                Assert::AreEqual(_sTest.IndexOf(_S("12"), 1, 3), size_t(1));
            }
        }

        TEST_METHOD(IndexOfAny)
        {
            YY::uStringView _sTest(_S("0123456789"));

            {
                Assert::AreEqual(_sTest.IndexOfAny(YY::uStringView::FromStaticString(_S("0123456789"))), size_t(0));
                Assert::AreEqual(_sTest.IndexOfAny(YY::uStringView::FromStaticString(_S("123456789"))), size_t(1));
                Assert::AreEqual(_sTest.IndexOfAny(YY::uStringView::FromStaticString(_S("9"))), size_t(9));
                Assert::AreEqual(_sTest.IndexOfAny(YY::uStringView::FromStaticString(_S("abcd"))), YY::kuInvalidIndex);
            }
        }

        TEST_METHOD(SplitAndTakeFirst)
        {
            YY::uStringView _szTemp(_S("01|34|6789"));

            size_t _uNextIndex = 0;
            Assert::AreEqual(_szTemp.SplitAndTakeFirst(YY::uStringView::char_t('|'), _uNextIndex, &_uNextIndex), YY::uStringView::FromStaticString(_S("01")));
            Assert::AreEqual(_uNextIndex, size_t(3));

            Assert::AreEqual(_szTemp.SplitAndTakeFirst(YY::uStringView::char_t('|'), _uNextIndex, &_uNextIndex), YY::uStringView::FromStaticString(_S("34")));
            Assert::AreEqual(_uNextIndex, size_t(6));

            Assert::AreEqual(_szTemp.SplitAndTakeFirst(YY::uStringView::char_t('|'), _uNextIndex, &_uNextIndex), YY::uStringView::FromStaticString(_S("6789")));
            Assert::AreEqual(_uNextIndex, size_t(10));
        }
    };
}
