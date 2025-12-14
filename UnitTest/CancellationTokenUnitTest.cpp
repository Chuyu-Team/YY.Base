#include "CppUnitTest.h"
#include <atlstr.h>
#include <Windows.h>
#include <tchar.h>
#include <string>

#include <YY/Base/Threading/TaskRunner.h>
#include <YY/Base/Threading/ProcessThreads.h>
#include <YY/Base/Time/TickCount.h>
#include <YY/Base/Strings/String.h>
#include <YY/Base/Threading/Task.h>
#include <YY/Base/Memory/UniquePtr.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace YY::Base;
using namespace YY::Base::Threading;
using namespace YY::Base::Memory;
using namespace YY::Base::Time;

namespace CancellationTokenUnitTest
{
    class TestCancellationTokenCallback : public CancellationTokenCancelHandle
    {
    public:
        int32_t cchCanceled = 0;

        void __YYAPI OnCanceled() override
        {
            ++cchCanceled;
        }
    };

    TEST_CLASS(CancellationTokenUnitTest)
    {
    public:
        TEST_METHOD(FromPtr_std_weak_ptr基础行为)
        {
            auto _pData = std::make_shared<int>(1);

            auto _pCancellationToken = CancellationToken::FromPtr(std::weak_ptr<int>(_pData));

            {
                Assert::IsFalse(_pCancellationToken->IsCancellationRequested());

                try
                {
                    _pCancellationToken->ThrowIfCancellationRequested();
                }
                catch (...)
                {
                    Assert::Fail(L"一开始不可能处于取消状态");
                }
            }

            _pData.reset();

            {
                Assert::IsTrue(_pCancellationToken->IsCancellationRequested());

                try
                {
                    _pCancellationToken->ThrowIfCancellationRequested();
                    Assert::Fail(L"都取消了你怎么了？");
                }
                catch (const OperationCanceledException& _ex)
                {
                    // 符合预期
                }
                catch (...)
                {
                    Assert::Fail(L"这里只可能是OperationCanceledException异常。");
                }
            }
        }

        TEST_METHOD(FromPtr_YY_ObserverPtr基础行为)
        {
            struct Data : public YY::ObserverPtrFactory
            {

            };

            YY::UniquePtr<Data> _pData(new Data);

            auto _pCancellationToken = CancellationToken::FromPtr(YY::ObserverPtr<Data>(_pData.Get()));

            {
                Assert::IsFalse(_pCancellationToken->IsCancellationRequested());

                try
                {
                    _pCancellationToken->ThrowIfCancellationRequested();
                }
                catch (...)
                {
                    Assert::Fail(L"一开始不可能处于取消状态");
                }
            }

            _pData.Reset();

            {
                Assert::IsTrue(_pCancellationToken->IsCancellationRequested());

                try
                {
                    _pCancellationToken->ThrowIfCancellationRequested();
                    Assert::Fail(L"都取消了你怎么了？");
                }
                catch (const OperationCanceledException& _ex)
                {
                    // 符合预期
                }
                catch (...)
                {
                    Assert::Fail(L"这里只可能是OperationCanceledException异常。");
                }
            }
        }

        TEST_METHOD(FromPtr_YY_WeakPtr基础行为)
        {
            struct Data : public YY::RefValue
            {

            };

            auto _pData= YY::RefPtr<Data>::FromPtr(new Data);

            auto _pCancellationToken = CancellationToken::FromPtr(YY::WeakPtr<Data>(_pData.Get()));

            {
                Assert::IsFalse(_pCancellationToken->IsCancellationRequested());

                try
                {
                    _pCancellationToken->ThrowIfCancellationRequested();
                }
                catch (...)
                {
                    Assert::Fail(L"一开始不可能处于取消状态");
                }
            }

            _pData.Reset();

            {
                Assert::IsTrue(_pCancellationToken->IsCancellationRequested());

                try
                {
                    _pCancellationToken->ThrowIfCancellationRequested();
                    Assert::Fail(L"都取消了你怎么了？");
                }
                catch (const OperationCanceledException& _ex)
                {
                    // 符合预期
                }
                catch (...)
                {
                    Assert::Fail(L"这里只可能是OperationCanceledException异常。");
                }
            }
        }

        TEST_METHOD(FromPtr不支持Register)
        {
            auto _pData = std::make_shared<int>(1);
            auto _pCancellationToken = CancellationToken::FromPtr(std::weak_ptr<int>(_pData));
            TestCancellationTokenCallback _oCallback;

            Assert::IsFalse(_pCancellationToken->Register(&_oCallback));
            Assert::IsFalse(_pCancellationToken->Unregister(&_oCallback));
        }
    };

    TEST_CLASS(CancellationTokenSourceUnitTest)
    {
    public:
        TEST_METHOD(基础行为)
        {
            auto _pCancellationTokenSource = CancellationTokenSource::Create();

            {
                Assert::IsFalse(_pCancellationTokenSource->IsCancellationRequested());

                try
                {
                    _pCancellationTokenSource->ThrowIfCancellationRequested();
                }
                catch (...)
                {
                    Assert::Fail(L"一开始不可能处于取消状态");
                }
            }

            Assert::IsTrue(_pCancellationTokenSource->Cancel());

            {
                Assert::IsTrue(_pCancellationTokenSource->IsCancellationRequested());

                try
                {
                    _pCancellationTokenSource->ThrowIfCancellationRequested();
                    Assert::Fail(L"都取消了你怎么了？");
                }
                catch (const OperationCanceledException& _ex)
                {
                    // 符合预期
                }
                catch (...)
                {
                    Assert::Fail(L"这里只可能是OperationCanceledException异常。");
                }
            }
        }

        TEST_METHOD(CreateTask支持CancellationTokenSource主动取消)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();
            auto _pCancellationTokenSource = CancellationTokenSource::Create();

            _pCancellationTokenSource->Cancel();

            auto _Task = _pTaskRunner->CreateTask(
                []() -> int
                {
                    return 1;
                },
                _pCancellationTokenSource);

            try
            {
                (void)_Task.GetResult();
                Assert::Fail();
            }
            catch (const YY::OperationCanceledException&)
            {
            }
        }

        TEST_METHOD(CreateDelayTask支持CancellationTokenSource主动取消)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();
            auto _pCancellationTokenSource = CancellationTokenSource::Create();

            auto _Task = _pTaskRunner->CreateDelayTask(
                TimeSpan::FromMilliseconds(50),
                []() -> int
                {
                    return 2;
                },
                _pCancellationTokenSource);

            _pCancellationTokenSource->Cancel();

            try
            {
                (void)_Task.GetResult();
                Assert::Fail();
            }
            catch (const YY::OperationCanceledException&)
            {
            }
        }

        TEST_METHOD(Then链路继承CancellationToken)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();
            auto _pCancellationTokenSource = CancellationTokenSource::Create();

            auto _Task = _pTaskRunner->CreateTask(
                []() -> int
                {
                    return 10;
                },
                _pCancellationTokenSource)
                .Then(
                    _pTaskRunner,
                    [_pCancellationTokenSource](int _uValue) -> int
                    {
                        _pCancellationTokenSource->Cancel();
                        return _uValue + 1;
                    });

            try
            {
                (void)_Task.GetResult();
                Assert::Fail();
            }
            catch (const YY::OperationCanceledException&)
            {
            }
        }

        TEST_METHOD(Register和Cancel基础行为)
        {
            auto _pCancellationTokenSource = CancellationTokenSource::Create();
            TestCancellationTokenCallback _oCallback;

            Assert::IsTrue(_pCancellationTokenSource->Register(&_oCallback));
            Assert::IsTrue(_pCancellationTokenSource->Cancel());
            Assert::AreEqual(1, _oCallback.cchCanceled);
        }

        TEST_METHOD(Unregister后Cancel不再回调)
        {
            auto _pCancellationTokenSource = CancellationTokenSource::Create();
            TestCancellationTokenCallback _oCallback;

            Assert::IsTrue(_pCancellationTokenSource->Register(&_oCallback));
            Assert::IsTrue(_pCancellationTokenSource->Unregister(&_oCallback));
            Assert::IsTrue(_pCancellationTokenSource->Cancel());
            Assert::AreEqual(0, _oCallback.cchCanceled);
        }

        TEST_METHOD(取消后Register失败)
        {
            auto _pCancellationTokenSource = CancellationTokenSource::Create();
            TestCancellationTokenCallback _oCallback;

            Assert::IsTrue(_pCancellationTokenSource->Cancel());
            Assert::IsFalse(_pCancellationTokenSource->Register(&_oCallback));
            Assert::AreEqual(0, _oCallback.cchCanceled);
        }

        TEST_METHOD(重复Cancel只触发一次)
        {
            auto _pCancellationTokenSource = CancellationTokenSource::Create();
            TestCancellationTokenCallback _oCallback;

            Assert::IsTrue(_pCancellationTokenSource->Register(&_oCallback));
            Assert::IsTrue(_pCancellationTokenSource->Cancel());
            Assert::IsFalse(_pCancellationTokenSource->Cancel());
            Assert::AreEqual(1, _oCallback.cchCanceled);
        }
    };
} // namespace UnitTest
