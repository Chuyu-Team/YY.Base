#include "CppUnitTest.h"

#include <YY/Base/Utils/AutoCleanup.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
    TEST_CLASS(AutoCleanup)
    {
    public:
        TEST_METHOD(普通)
        {
            int _iValue = 0;

            {
                auto _oCleanup = YY::MakeAutoCleanup(
                    [&]()
                    {
                        _iValue++;
                    });

                Assert::AreEqual(0, _iValue);
            }
            Assert::AreEqual(1, _iValue);
        }
    };
}
