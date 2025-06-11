#include "CppUnitTest.h"
#include <atlstr.h>
#include <Windows.h>
#include <tchar.h>
#include <string>

#include <YY/Base/Functional/Bind.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace YY;

namespace UnitTest
{
    class BindTest
        : public YY::Base::Memory::RefValue
        , public YY::Base::Memory::ObserverPtrFactory
    {
        int nData = 1;

    public:
        BindTest(int _nData)
            : nData(_nData)
        {
        }

        ~BindTest()
        {
            nData = 0;
        }

        int TestFunction(int nValue, int nValue2)
        {
            Assert::AreEqual(nData, nValue);
            return nValue2;
        }
    };

    TEST_CLASS(Bind)
    {
    public:
        TEST_METHOD(UnsafeBindPtr)
        {
            BindTest _Object1(1);
            auto _pfnCallBack = YY::Bind(&BindTest::TestFunction, YY::UnsafeBindPtr<BindTest>(&_Object1), 1, 2);
            Assert::AreEqual(_pfnCallBack(), 2);
        }

        TEST_METHOD(UniquePtr)
        {
            auto _pObject1 = YY::UniquePtr<BindTest>::Create(1);
            auto _pfnCallBack = YY::Bind(&BindTest::TestFunction, std::move(_pObject1), 1, 2);
            Assert::AreEqual(_pfnCallBack(), 2);
        }

        TEST_METHOD(ObserverPtr)
        {
            auto _pObject1 = YY::RefPtr<BindTest>::Create(1);
            auto _pfnCallBack = YY::Bind(&BindTest::TestFunction, YY::ObserverPtr<BindTest>(_pObject1), 1, 2);
            Assert::AreEqual(_pfnCallBack(), 2);
            _pObject1.Reset();

            try
            {
                _pfnCallBack();
                Assert::Fail(L"Expected exception not thrown.");
            }
            catch (const YY::Base::Exception& _Exception)
            {
                Assert::AreEqual(_Exception.GetErrorCode(), HRESULT_From_LSTATUS(ERROR_CANCELLED));
            }
        }

        TEST_METHOD(RefPtr)
        {
            auto _pObject1 = YY::RefPtr<BindTest>::Create(1);
            auto _pfnCallBack = YY::Bind(&BindTest::TestFunction, std::move(_pObject1), 1, 2);
            Assert::AreEqual(_pfnCallBack(), 2);
        }

        TEST_METHOD(WeakPtr)
        {
            auto _pObject1 = YY::RefPtr<BindTest>::Create(1);
            YY::WeakPtr<BindTest> _pWeakObject1(_pObject1);
            auto _pfnCallBack = YY::Bind(&BindTest::TestFunction, std::move(_pWeakObject1), 1, 2);
            Assert::AreEqual(_pfnCallBack(), 2);
            _pObject1.Reset();

            try
            {
                _pfnCallBack();
                Assert::Fail(L"Expected exception not thrown.");
            }
            catch (const YY::Base::Exception& _Exception)
            {
                Assert::AreEqual(_Exception.GetErrorCode(), HRESULT_From_LSTATUS(ERROR_CANCELLED));
            }
        }
    };
}
