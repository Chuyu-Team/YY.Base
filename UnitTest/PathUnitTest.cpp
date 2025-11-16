#include "ToStringHelper.h"
#include "CppUnitTest.h"

#include <YY/Base/IO/Path.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace YY;

namespace UnitTest
{
    TEST_CLASS(Path)
    {
    public:
        TEST_METHOD(Combine)
        {
            Assert::AreEqual(YY::Path::Combine(_S("C:\\"), _S("mydir"), _S("test.txt")), _S("C:\\mydir\\test.txt"));

            Assert::AreEqual(YY::Path::Combine(_S("C:\\"), _S("D:\\SSS"), _S("mydir"), _S("test.txt")), _S("D:\\SSS\\mydir\\test.txt"));

            Assert::AreEqual(YY::Path::Combine(_S("C:\\"), _S("\\\\TTT\\SSS"), _S("mydir"), _S("test.txt")), _S("\\\\TTT\\SSS\\mydir\\test.txt"));
        }
    };
}
