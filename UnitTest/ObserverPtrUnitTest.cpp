#include "CppUnitTest.h"
#include <atlstr.h>
#include <Windows.h>
#include <tchar.h>
#include <string>

#include <YY/Base/Memory/ObserverPtr.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace YY;

namespace
{
    class TestObserver : public Base::Memory::ObserverPtrFactory
    {
    public:
    };
}

namespace UnitTest
{
    TEST_CLASS(ObserverPtr)
    {
    public:
        TEST_METHOD(构建销毁测试)
        {
            YY::ObserverPtr<TestObserver> _pObject1;
            {
                TestObserver _Object1;
                _pObject1 = &_Object1;

                Assert::AreEqual((void*)_pObject1.Get(), (void*)&_Object1);
            }

            Assert::AreEqual((void*)_pObject1.Get(), (void*)nullptr);
        }
    };
}
