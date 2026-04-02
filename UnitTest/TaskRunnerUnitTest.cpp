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

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace YY::Base;
using namespace YY::Base::Threading;
using namespace YY::Base::Memory;
using namespace YY::Base::Time;

namespace TaskRunnerUnitTest
{
    TEST_CLASS(SequencedTaskRunnerUnitTest)
    {
    public:
        TEST_METHOD(任务序列化保证)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();

            uint32_t _uCount = 0;
            volatile uint32_t _uCount2 = 0;

            for (auto i = 0; i != 1000; ++i)
            {
                _pTaskRunner->PostTask(
                    [&_uCount, &_uCount2]()
                    {
                        Assert::AreEqual(Sync::Increment(&_uCount), 1u);

                        Sleep(5);

                        Assert::AreEqual(Sync::Decrement(&_uCount), 0u);

                        Sync::Increment(&_uCount2);
                    });
            }

            for (int i= 0; _uCount2 != 1000;++i)
            {
                Assert::IsTrue(i < 100);

                Sleep(1000);
            }
        }

        TEST_METHOD(TaskRunner销毁后任务全部自动取消)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();

            uint32_t _uCount = 0;
            volatile uint32_t _uCount2 = 0;

            for (auto i = 0; i != 1000; ++i)
            {
                _pTaskRunner->PostTask(
                    [&_uCount, &_uCount2]()
                    {
                        Assert::AreEqual(Sync::Increment(&_uCount), 1u);


                        if (Sync::Increment(&_uCount2) == 1)
                        {
                            Sleep(500);
                        }

                        Assert::AreEqual(Sync::Decrement(&_uCount), 0u);
                    });
            }

            _pTaskRunner = nullptr;

            Sleep(5000);


            Assert::AreEqual((uint32_t)_uCount2, 1u);
        }

        TEST_METHOD(TaskRunner内部可以正确取到TaskRunner自身指针)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();

            YY::RefPtr<SequencedTaskRunner> _pOutTaskRunner;

            _pTaskRunner->PostTask(
                [&_pOutTaskRunner]()
                {
                    _pOutTaskRunner = SequencedTaskRunner::GetCurrent();

                    Assert::AreEqual((void*)TaskRunner::GetCurrent().Get(), (void*)_pOutTaskRunner.Get());
                    Assert::AreEqual((void*)ThreadTaskRunner::GetCurrent().Get(), (void*)nullptr);
                    Assert::AreEqual((void*)ParallelTaskRunner::GetCurrent().Get(), (void*)nullptr);
                });

            Sleep(500);
            
            Assert::AreEqual((void*)_pOutTaskRunner.Get(), (void*)_pTaskRunner.Get());
        }

#if 0
        // 耗时太长，暂时不启用
        TEST_METHOD(时间间隔检测)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();

            for(int i = 0;i!=500;++i)
            {
                constexpr auto kDelay = 10ul;
                constexpr auto kDeviation = 35ul;

                auto _uStartTick = TickCount::GetNow();
                TickCount _uEnd;

                RefPtr<Timer> _pTimer;
                _pTimer = _pTaskRunner->CreateTimer(
                    TimeSpan::FromMilliseconds(10),
                    [&_uEnd]()
                    {
                        _uEnd = TickCount::GetNow();
                        return false;
                    });

                Assert::IsTrue(_pTimer->Wait(kDelay * 5));

                auto nMilliseconds = (_uEnd - _uStartTick).GetMilliseconds();
                Strings::uString _szTmp;
                _szTmp.Format(L"第 %d次，预期延迟 %u ，实际延迟 %I64d，\n", i, kDelay, nMilliseconds);

                Assert::IsTrue(nMilliseconds <= (kDelay + kDeviation) && nMilliseconds >= (kDelay - 5), _szTmp);
            }


            for (int i = 0; i != 250; ++i)
            {
                constexpr auto kDelay = 100ul;
                constexpr auto kDeviation = 35ul;

                auto _uStartTick = TickCount::GetNow();
                TickCount _uEnd;

                RefPtr<Timer> _pTimer;
                _pTimer = _pTaskRunner->CreateTimer(
                    TimeSpan::FromMilliseconds(kDelay),
                    [&_uEnd]()
                    {
                        _uEnd = TickCount::GetNow();
                        return false;
                    });

                Assert::IsTrue(_pTimer->Wait((kDelay + kDeviation) * 2));

                auto nMilliseconds = (_uEnd - _uStartTick).GetMilliseconds();
                Strings::uString _szTmp;
                _szTmp.Format(L"第 %d次，预期延迟 %u ，实际延迟 %I64d，\n", i, kDelay, nMilliseconds);

                Assert::IsTrue(nMilliseconds <= (kDelay + kDeviation) && nMilliseconds >= (kDelay - 5), _szTmp);
            }

            for (int i = 0; i != 250; ++i)
            {
                constexpr auto kDelay = 1000ul;
                constexpr auto kDeviation = 35ul;

                auto _uStartTick = TickCount::GetNow();
                TickCount _uEnd;

                RefPtr<Timer> _pTimer;
                _pTimer = _pTaskRunner->CreateTimer(
                    TimeSpan::FromMilliseconds(kDelay),
                    [&_uEnd]()
                    {
                        _uEnd = TickCount::GetNow();
                        return false;
                    });

                Assert::IsTrue(_pTimer->Wait((kDelay + kDeviation )* 2));

                auto nMilliseconds = (_uEnd - _uStartTick).GetMilliseconds();
                Strings::uString _szTmp;
                _szTmp.Format(L"第 %d次，预期延迟 %u ，实际延迟 %I64d，\n", i, kDelay, nMilliseconds);

                Assert::IsTrue(nMilliseconds <= (kDelay + kDeviation) && nMilliseconds >= (kDelay - 5), _szTmp);
            }


            for (int i = 0; i != 100; ++i)
            {
                constexpr auto kDelay = 1300ul;
                constexpr auto kDeviation = 35ul;

                auto _uStartTick = TickCount::GetNow();
                TickCount _uEnd;

                RefPtr<Timer> _pTimer;
                _pTimer = _pTaskRunner->CreateTimer(
                    TimeSpan::FromMilliseconds(kDelay),
                    [&_uEnd]()
                    {
                        _uEnd = TickCount::GetNow();
                        return false;
                    });

                Assert::IsTrue(_pTimer->Wait((kDelay + kDeviation) * 2));

                auto nMilliseconds = (_uEnd - _uStartTick).GetMilliseconds();
                Strings::uString _szTmp;
                _szTmp.Format(L"第 %d次，预期延迟 %u ，实际延迟 %I64d，\n", i, kDelay, nMilliseconds);

                Assert::IsTrue(nMilliseconds <= (kDelay + kDeviation) && nMilliseconds >= (kDelay - 5), _szTmp);
            }
        }
#endif

        TEST_METHOD(周期性唤醒检查)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();


            HANDLE _hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

            auto _uStartTick = TickCount::GetNow();

            int nCount = 0;
            RefPtr<Timer> _pTimer;
            _pTimer = _pTaskRunner->CreateTimer(
                TimeSpan::FromMilliseconds(500),
                [&nCount, _uStartTick, _hEvent]()
                {
                    ++nCount;
                    auto _nSpan = TickCount::GetNow() - _uStartTick;

                    Strings::uString _szTmp;
                    _szTmp.Format(L"Run 延迟 %I64d\n", _nSpan.GetTotalMilliseconds());

                    OutputDebugStringW(_szTmp);
                    auto _uArg = _nSpan.GetTotalMilliseconds() / nCount;

                    Assert::IsTrue(_uArg >= 500 - 100);
                    Assert::IsTrue(_uArg <= 500 + 100);

                    if (nCount == 5)
                    {
                        SetEvent(_hEvent);
                        return false;
                    }
                    else
                    {
                        return true;
                    }
                });


            WaitForSingleObject(_hEvent, 5000);

            Assert::AreEqual(nCount, 5);
            _pTimer = nullptr;
            _pTaskRunner = nullptr;

        }

        TEST_METHOD(Wait句柄测试)
        {
            HANDLE _hWaitEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
            auto _pTaskRunner = SequencedTaskRunner::Create();

            for (int i = 0; i < 3; ++i)
            {
                HANDLE _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
                volatile uint32_t _uWaitResultCount = 0;
                volatile UINT64 _uTickCount = 0;

                auto _pWait = _pTaskRunner->CreateWait(_hEvent, [&](DWORD _uWaitResultT)
                    {
                        if (_uWaitResultT == WAIT_OBJECT_0)
                        {
                            YY::Increment(&_uWaitResultCount);
                            _uTickCount = GetTickCount64();
                        }

                        return true;
                    });

                WaitForSingleObject(_hWaitEvent, 600);
                SetEvent(_hEvent);
                auto _uTickCountEnd = GetTickCount64();
                WaitForSingleObject(_hWaitEvent, 10);

                Strings::uString _szTmp;
                auto _uTickCount2 = _uTickCount;
                const auto _nDelta = abs((long long)(_uTickCountEnd - _uTickCount2));
                _szTmp.Format(L"TickCount1=%I64d，TickCount2=%I64d, Delta=%I64d\n", _uTickCountEnd, _uTickCount2, _nDelta);

                Assert::IsTrue(_nDelta < 100, _szTmp.GetConstString());
                // CloseHandle(_hEvent);
            }

            CloseHandle(_hWaitEvent);
        }

        TEST_METHOD(超多句柄等待情况测试)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();
            HANDLE _hEvents[300];
            YY::RefPtr<Wait> _pWaits[300];
            volatile uint32_t _uWaitResultCount = 0;
            volatile uint32_t _uWaitFaildCount = 0;

            for (size_t i = 0; i != std::size(_hEvents); ++i)
            {
                auto& _hEvent = _hEvents[i];
                _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
                _pWaits[i] = _pTaskRunner->CreateWait(_hEvent, [&](DWORD _uWaitResultT)
                    {
                        if (_uWaitResultT == WAIT_OBJECT_0)
                        {
                            YY::Increment(&_uWaitResultCount);
                        }
                        else
                        {
                            YY::Increment(&_uWaitFaildCount);
                        }

                        return true;
                    });
            }

            Sleep(1000);
            Assert::AreEqual((uint32_t)_uWaitResultCount, uint32_t(0));

            for (auto _hEvent : _hEvents)
            {
                SetEvent(_hEvent);
            }

            Sleep(1000);
            Assert::AreEqual((uint32_t)_uWaitFaildCount, uint32_t(0));
            Assert::AreEqual((uint32_t)_uWaitResultCount, uint32_t(std::size(_hEvents)));

            for (auto _hEvent : _hEvents)
            {
                SetEvent(_hEvent);
            }

            Sleep(1000);
            Assert::AreEqual((uint32_t)_uWaitFaildCount, uint32_t(0));
            Assert::AreEqual((uint32_t)_uWaitResultCount, uint32_t(std::size(_hEvents) * 2));

            //for (auto _hEvent : _hEvents)
            //{
            //    CloseHandle(_hEvent);
            //}
        }

        TEST_METHOD(Wait句柄超时测试)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();

            HANDLE _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
            volatile DWORD _uWaitResult = -1;
            auto _pWait = _pTaskRunner->CreateWait(
                _hEvent,
                TimeSpan::FromMilliseconds(500),
                [&_uWaitResult](DWORD _uWaitResultT)
                {
                    _uWaitResult = _uWaitResultT;
                    return false;
                });

            Assert::IsTrue(((TaskEntry*)_pWait.Get())->WaitTask(YY::TimeSpan::FromMilliseconds(600ul)));
            Assert::AreEqual(DWORD(_uWaitResult), DWORD(WAIT_TIMEOUT));

            _pWait = _pTaskRunner->CreateWait(
                _hEvent,
                TimeSpan::FromMilliseconds(500),
                [&_uWaitResult](DWORD _uWaitResultT)
                {
                    _uWaitResult = _uWaitResultT;
                    return false;
                });
            SetEvent(_hEvent);
            Assert::IsTrue(((TaskEntry*)_pWait.Get())->WaitTask(YY::TimeSpan::FromMilliseconds(100ul)));
            Assert::AreEqual(DWORD(_uWaitResult), DWORD(WAIT_OBJECT_0));
            // CloseHandle(_hEvent);
        }
    };


    TEST_CLASS(ParallelTaskRunnerUnitTest)
    {
    public :
        TEST_METHOD(并行数量保证)
        {
            {
                // 只允许一个并行
                uint32_t _uCount = 0;
                volatile uint32_t _uCount2 = 0;
                auto _pTaskRunner = ParallelTaskRunner::Create(1);

                for (auto i = 0; i != 1000; ++i)
                {
                    _pTaskRunner->PostTask(
                        [&_uCount, &_uCount2]()
                        {
                            Assert::AreEqual(Sync::Increment(&_uCount), 1u);

                            Sleep(5);

                            Assert::AreEqual(Sync::Decrement(&_uCount), 0u);

                            Sync::Increment(&_uCount2);
                        });
                }

                for (int i = 0; _uCount2 != 1000; ++i)
                {
                    Assert::IsTrue(i < 100);

                    Sleep(1000);
                }
            }

            {
                // 允许4个并行
                uint32_t _uCount = 0;
                volatile uint32_t _uMaxCount = 0;
                volatile uint32_t _uCount2 = 0;
                auto _pTaskRunner = ParallelTaskRunner::Create(4);

                for (auto i = 0; i != 1000; ++i)
                {
                    _pTaskRunner->PostTask(
                        [&_uCount, &_uCount2, &_uMaxCount]()
                        {
                            auto _uNew = Sync::Increment(&_uCount);
                            Assert::IsTrue(_uNew <= 4u);
                            auto _uOldMaxCount = _uMaxCount;
                            for (; _uOldMaxCount < _uNew;)
                            {
                                auto _uLast = Sync::CompareExchange(&_uMaxCount, _uNew, _uOldMaxCount);
                                if (_uLast == _uOldMaxCount)
                                {
                                    break;
                                }

                                _uOldMaxCount = _uLast;
                            }

                            Sleep(5);

                            Assert::IsTrue(Sync::Decrement(&_uCount) < 4u);

                            Sync::Increment(&_uCount2);
                        });
                }

                for (int i = 0; _uCount2 != 1000; ++i)
                {
                    Assert::IsTrue(i < 100);

                    Sleep(1000);
                }

                Assert::AreEqual((uint32_t)_uMaxCount, 4u);
            }
        }

        
        TEST_METHOD(TaskRunner销毁后任务全部自动取消)
        {
            auto _pTaskRunner = ParallelTaskRunner::Create(4);

            uint32_t _uCount = 0;
            volatile uint32_t _uCount2 = 0;

            for (auto i = 0; i != 1000; ++i)
            {
                _pTaskRunner->PostTask(
                    [&_uCount, &_uCount2]()
                    {
                        Assert::IsTrue(Sync::Increment(&_uCount) <= 4u);

                        Sleep(500);

                        Sync::Increment(&_uCount2);
                        Assert::IsTrue(Sync::Decrement(&_uCount) < 4u);
                    });
            }

            _pTaskRunner = nullptr;

            Sleep(5000);


            Assert::IsTrue((uint32_t)_uCount2 < 100u);
        }

        TEST_METHOD(TaskRunner内部可以正确取到TaskRunner自身指针)
        {
            auto _pTaskRunner = ParallelTaskRunner::Create();

            YY::RefPtr<ParallelTaskRunner> _pOutTaskRunner;

            _pTaskRunner->PostTask(
                [&_pOutTaskRunner]()
                {
                    _pOutTaskRunner = ParallelTaskRunner::GetCurrent();

                    Assert::AreEqual((void*)TaskRunner::GetCurrent().Get(), (void*)_pOutTaskRunner.Get());
                    Assert::AreEqual((void*)SequencedTaskRunner::GetCurrent().Get(), (void*)nullptr);
                    Assert::AreEqual((void*)ThreadTaskRunner::GetCurrent().Get(), (void*)nullptr);
                });

            Sleep(500);
            
            Assert::AreEqual((void*)_pOutTaskRunner.Get(), (void*)_pTaskRunner.Get());
        }
    };

    TEST_CLASS(ThreadTaskRunnerUnitTest)
    {
    public:
        TEST_METHOD(PostTask可用性检测)
        {
            auto _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

            RefPtr<ThreadTaskRunner> _pTaskRunners[] = { ThreadTaskRunner::Create(false), ThreadTaskRunner::Create(true) };
            for (auto& _pTaskRunner : _pTaskRunners)
            {
                volatile uint32_t _uCount2 = 0;

                _pTaskRunner->PostTask([&_uCount2, _hEvent]()
                    {
                        YY::Increment(&_uCount2);
                        SetEvent(_hEvent);
                    });

                WaitForSingleObject(_hEvent, 100);

                Assert::AreEqual((uint32_t)_uCount2, 1u);

                _pTaskRunner->PostTask([&_uCount2, _hEvent]()
                    {
                        YY::Increment(&_uCount2);
                        SetEvent(_hEvent);
                    });

                WaitForSingleObject(_hEvent, 100);

                Assert::AreEqual((uint32_t)_uCount2, 2u);

                _pTaskRunner->PostTask([&_uCount2, _hEvent]()
                    {
                        YY::Increment(&_uCount2);
                        SetEvent(_hEvent);
                    });

                WaitForSingleObject(_hEvent, 100);

                Assert::AreEqual((uint32_t)_uCount2, 3u);
            }

            CloseHandle(_hEvent);
        }


        TEST_METHOD(线程Id获取)
        {
            auto _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

            auto _pTaskRunner = ThreadTaskRunner::Create(true);

            auto _oId = _pTaskRunner->GetThreadId();
            volatile decltype(_oId) _oId2 = 0xCC;


            _pTaskRunner->PostTask([&_oId2, _hEvent]()
                {
                    _oId2 = Threading::GetCurrentThreadId();
                    SetEvent(_hEvent);
                });

            Assert::AreEqual(WaitForSingleObject(_hEvent, 100), WAIT_OBJECT_0);

            Assert::AreEqual(_oId, decltype(_oId)(_oId2));
            CloseHandle(_hEvent);
        }

        TEST_METHOD(BindCurrentThreadForProxyMode嵌套)
        {
            auto _pTaskRunner = ThreadTaskRunner::BindCurrentThreadForProxyMode();

            int i = 0;

            _pTaskRunner->PostTask(
                [&i]()
                {
                    ++i;
                });

            _pTaskRunner->PostTask(
                [&i]()
                {
                    ++i;

                    TaskRunner::GetCurrent()->PostTask(
                        [&i]()
                        {
                            ++i;

                            TaskRunner::GetCurrent()->PostTask(
                                [&i]()
                                {
                                    ++i;
                                    ThreadTaskRunner::PostQuitMessage(0);

                                });
                        });

                    for (;;)
                    {
                        MSG _msg;
                        if (!GetMessageW(&_msg, nullptr, 0, 0))
                            break;


                        TranslateMessage(&_msg);
                        DispatchMessageW(&_msg);
                    }

                    ThreadTaskRunner::PostQuitMessage(0);
                });

            for (;;)
            {
                MSG _msg;
                if (!GetMessageW(&_msg, nullptr, 0, 0))
                    break;


                TranslateMessage(&_msg);
                DispatchMessageW(&_msg);
            }

            Assert::AreEqual(i, 4);
            return;
        }

        TEST_METHOD(BindCurrentThread嵌套)
        {
            auto _pTaskRunner = ThreadTaskRunner::BindCurrentThread();

            int i = 0;

            _pTaskRunner->PostTask(
                [&i]()
                {
                    ++i;
                });

            _pTaskRunner->PostTask(
                [&i]()
                {
                    ++i;

                    TaskRunner::GetCurrent()->PostTask(
                        [&i]()
                        {
                            ++i;

                            TaskRunner::GetCurrent()->PostTask(
                                [&i]()
                                {
                                    ++i;
                                    ThreadTaskRunner::PostQuitMessage(0);

                                });
                        });

                    ThreadTaskRunner::RunUIMessageLoop();

                    ThreadTaskRunner::PostQuitMessage(0);
                });

            ThreadTaskRunner::RunUIMessageLoop();

            Assert::AreEqual(i, 4);
            return;
        }

        TEST_METHOD(ThreadTaskRunner前台嵌套)
        {
            auto _pTaskRunner = ThreadTaskRunner::Create(false, L"ThreadTaskRunner前台嵌套");

            int i = 0;

            HANDLE _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

            _pTaskRunner->PostTask(
                [&i, _hEvent]()
                {
                    ++i;
                });

            _pTaskRunner->PostTask(
                [&i, _hEvent]()
                {
                    ++i;

                    TaskRunner::GetCurrent()->PostTask(
                        [&i, _hEvent]()
                        {
                            ++i;

                            TaskRunner::GetCurrent()->PostTask(
                                [&i]()
                                {
                                    ++i;
                                    ThreadTaskRunner::PostQuitMessage(0);

                                });
                        });

                    ThreadTaskRunner::RunUIMessageLoop();


                    SetEvent(_hEvent);
                });

            WaitForSingleObject(_hEvent, INFINITE);

            Assert::AreEqual(i, 4);
            return;
        }

        TEST_METHOD(ThreadTaskRunner后台嵌套)
        {
            auto _pTaskRunner = ThreadTaskRunner::Create(true, L"ThreadTaskRunner后台嵌套");

            int i = 0;

            HANDLE _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

            _pTaskRunner->PostTask(
                [&i, _hEvent]()
                {
                    ++i;
                });

            _pTaskRunner->PostTask(
                [&i, _hEvent]()
                {
                    ++i;

                    TaskRunner::GetCurrent()->PostTask(
                        [&i, _hEvent]()
                        {
                            ++i;

                            TaskRunner::GetCurrent()->PostTask(
                                [&i]()
                                {
                                    ++i;
                                    ThreadTaskRunner::PostQuitMessage(0);

                                });
                        });

                    ThreadTaskRunner::RunUIMessageLoop();


                    SetEvent(_hEvent);
                });

            WaitForSingleObject(_hEvent, INFINITE);

            Assert::AreEqual(i, 4);
            return;
        }

#if 0
        // 耗时太长，暂时不启用
        TEST_METHOD(时间间隔检测)
        {
            auto _pTaskRunner = ThreadTaskRunner::Create();

            for (int i = 0; i != 500; ++i)
            {
                constexpr auto kDelay = 10ul;
                constexpr auto kDeviation = 35ul;

                auto _uStartTick = TickCount::GetNow();
                TickCount _uEnd;

                RefPtr<Timer> _pTimer;
                _pTimer = _pTaskRunner->CreateTimer(
                    TimeSpan::FromMilliseconds(10),
                    [&_uEnd]()
                    {
                        _uEnd = TickCount::GetNow();
                        return false;
                    });

                Assert::IsTrue(_pTimer->Wait(kDelay * 5));

                auto nMilliseconds = (_uEnd - _uStartTick).GetMilliseconds();
                Strings::uString _szTmp;
                _szTmp.Format(L"第 %d次，预期延迟 %u ，实际延迟 %I64d，\n", i, kDelay, nMilliseconds);

                Assert::IsTrue(nMilliseconds <= (kDelay + kDeviation) && nMilliseconds >= (kDelay - 5), _szTmp);
            }


            for (int i = 0; i != 250; ++i)
            {
                constexpr auto kDelay = 100ul;
                constexpr auto kDeviation = 35ul;

                auto _uStartTick = TickCount::GetNow();
                TickCount _uEnd;

                RefPtr<Timer> _pTimer;
                _pTimer = _pTaskRunner->CreateTimer(
                    TimeSpan::FromMilliseconds(kDelay),
                    [&_uEnd]()
                    {
                        _uEnd = TickCount::GetNow();
                        return false;
                    });

                Assert::IsTrue(_pTimer->Wait((kDelay + kDeviation) * 2));

                auto nMilliseconds = (_uEnd - _uStartTick).GetMilliseconds();
                Strings::uString _szTmp;
                _szTmp.Format(L"第 %d次，预期延迟 %u ，实际延迟 %I64d，\n", i, kDelay, nMilliseconds);

                Assert::IsTrue(nMilliseconds <= (kDelay + kDeviation) && nMilliseconds >= (kDelay - 5), _szTmp);
            }

            for (int i = 0; i != 250; ++i)
            {
                constexpr auto kDelay = 1000ul;
                constexpr auto kDeviation = 35ul;

                auto _uStartTick = TickCount::GetNow();
                TickCount _uEnd;

                RefPtr<Timer> _pTimer;
                _pTimer = _pTaskRunner->CreateTimer(
                    TimeSpan::FromMilliseconds(kDelay),
                    [&_uEnd]()
                    {
                        _uEnd = TickCount::GetNow();
                        return false;
                    });

                Assert::IsTrue(_pTimer->Wait((kDelay + kDeviation) * 2));

                auto nMilliseconds = (_uEnd - _uStartTick).GetMilliseconds();
                Strings::uString _szTmp;
                _szTmp.Format(L"第 %d次，预期延迟 %u ，实际延迟 %I64d，\n", i, kDelay, nMilliseconds);

                Assert::IsTrue(nMilliseconds <= (kDelay + kDeviation) && nMilliseconds >= (kDelay - 5), _szTmp);
            }


            for (int i = 0; i != 100; ++i)
            {
                constexpr auto kDelay = 1300ul;
                constexpr auto kDeviation = 35ul;

                auto _uStartTick = TickCount::GetNow();
                TickCount _uEnd;

                RefPtr<Timer> _pTimer;
                _pTimer = _pTaskRunner->CreateTimer(
                    TimeSpan::FromMilliseconds(kDelay),
                    [&_uEnd]()
                    {
                        _uEnd = TickCount::GetNow();
                        return false;
                    });

                Assert::IsTrue(_pTimer->Wait((kDelay + kDeviation) * 2));

                auto nMilliseconds = (_uEnd - _uStartTick).GetMilliseconds();
                Strings::uString _szTmp;
                _szTmp.Format(L"第 %d次，预期延迟 %u ，实际延迟 %I64d，\n", i, kDelay, nMilliseconds);

                Assert::IsTrue(nMilliseconds <= (kDelay + kDeviation) && nMilliseconds >= (kDelay - 5), _szTmp);
            }
        }
#endif

        TEST_METHOD(周期性唤醒检查)
        {
            HANDLE _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

            RefPtr<ThreadTaskRunner> _pTaskRunners[] = { ThreadTaskRunner::Create(false), ThreadTaskRunner::Create(true)};
            for (auto& _pTaskRunner : _pTaskRunners)
            {
                auto _uStartTick = TickCount::GetNow();

                volatile int nCount = 0;
                RefPtr<Timer> _pTimer;
                _pTimer = _pTaskRunner->CreateTimer(
                    TimeSpan::FromMilliseconds(500),
                    [&nCount, _uStartTick, _hEvent]()
                    {
                        ++nCount;
                        if (nCount == 5)
                        {
                            SetEvent(_hEvent);
                            return false;
                        }
                        else
                        {
                            return true;
                        }
                    });


                WaitForSingleObject(_hEvent, 6000);
                Sleep(1000);
                Assert::AreEqual((int)nCount, 5);
                _pTimer = nullptr;
                _pTaskRunner = nullptr;
            }

            CloseHandle(_hEvent);
        }

        TEST_METHOD(Wait句柄测试)
        {
            RefPtr<ThreadTaskRunner> _pTaskRunners[] = { ThreadTaskRunner::Create(false), ThreadTaskRunner::Create(true) };
            for (auto& _pTaskRunner : _pTaskRunners)
            {
                HANDLE _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

                for (int i = 0; i < 10; ++i)
                {
                    volatile uint32_t _uWaitResultCount = 0;
                    volatile YY::TickCount _uTickCount;

                    auto _pWait = _pTaskRunner->CreateWait(_hEvent, [&](DWORD _uWaitResultT)
                        {
                            if (_uWaitResultT == WAIT_OBJECT_0)
                            {
                                _uTickCount.uTicks = YY::TickCount::GetNow().uTicks;
                            }

                            YY::Increment(&_uWaitResultCount);
                            WakeByAddressAll((PVOID)&_uWaitResultCount);
                            return false;
                        });
                    uint32_t _uTargetWaitResultCount = 0;

                    Assert::IsFalse(WaitOnAddress(&_uWaitResultCount, &_uTargetWaitResultCount, sizeof(_uTargetWaitResultCount), 600));

                    auto _uTickCountEnd = YY::TickCount::GetNow();
                    SetEvent(_hEvent);
                    Assert::IsTrue(WaitOnAddress(&_uWaitResultCount, &_uTargetWaitResultCount, sizeof(_uTargetWaitResultCount), 1000));

                    auto _uCOunt = (_uTickCountEnd - YY::TickCount::FromTicks(_uTickCount.uTicks)).GetTotalMilliseconds();
                    YY::uString _szMessage;
                    _szMessage.Format(L"第 %d次，预期延迟 %u，实际延迟 %d\n", i, 50, (int32_t)_uCOunt);
                    Assert::IsTrue(_uCOunt < 50, _szMessage.GetConstString());
                }

                CloseHandle(_hEvent);
            }
        }

        TEST_METHOD(超多句柄Wait句柄测试)
        {
            RefPtr<ThreadTaskRunner> _pTaskRunners[] = { ThreadTaskRunner::Create(false), ThreadTaskRunner::Create(true) };
            for (auto& _pTaskRunner : _pTaskRunners)
            {
                // 超多句柄等待情况测试
                {
                    HANDLE _hEvents[300];
                    YY::RefPtr<Wait> _pWaits[300];
                    volatile uint32_t _uWaitResultCount = 0;

                    // for (auto& _hEvent : _hEvents)
                    for (size_t i = 0; i != std::size(_hEvents); ++i)
                    {
                        auto& _hEvent = _hEvents[i];
                        _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
                        _pWaits[i] = _pTaskRunner->CreateWait(_hEvent, [&](DWORD _uWaitResultT)
                            {
                                if (_uWaitResultT == WAIT_OBJECT_0)
                                {
                                    YY::Increment(&_uWaitResultCount);
                                }
                                else
                                {
                                    int j = 0;
                                }
                                return true;
                            });
                    }

                    Sleep(100);
                    Assert::AreEqual((uint32_t)_uWaitResultCount, uint32_t(0));

                    for (auto _hEvent : _hEvents)
                    {
                        SetEvent(_hEvent);
                    }

                    Sleep(1000);
                    Assert::AreEqual((uint32_t)_uWaitResultCount, uint32_t(std::size(_hEvents)));

                    for (auto _hEvent : _hEvents)
                    {
                        SetEvent(_hEvent);
                    }

                    Sleep(1000);
                    Assert::AreEqual((uint32_t)_uWaitResultCount, uint32_t(std::size(_hEvents) * 2));

                    /*for (auto _hEvent : _hEvents)
                    {
                        CloseHandle(_hEvent);
                    }*/
                }
            }
        }

        TEST_METHOD(Wait句柄超时测试)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();

            HANDLE _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
            volatile DWORD _uWaitResult = -1;
            auto _pWait = _pTaskRunner->CreateWait(
                _hEvent,
                TimeSpan::FromMilliseconds(500),
                [&_uWaitResult](DWORD _uWaitResultT)
                {
                    _uWaitResult = _uWaitResultT;
                    return false;
                });

            Assert::IsTrue(_pWait->WaitTask(YY::TimeSpan::FromMilliseconds(600ul)));
            Assert::AreEqual(DWORD(_uWaitResult), DWORD(WAIT_TIMEOUT));

            _pWait = _pTaskRunner->CreateWait(
                _hEvent,
                TimeSpan::FromMilliseconds(500),
                [&_uWaitResult](DWORD _uWaitResultT)
                {
                    _uWaitResult = _uWaitResultT;
                    return false;
                });
            SetEvent(_hEvent);
            Assert::IsTrue((_pWait->WaitTask(YY::TimeSpan::FromMilliseconds(100ul))));
            // CloseHandle(_hEvent);
        }

        TEST_METHOD(Then语义)
        {
            const auto _hrPending = E_PENDING;

            auto _pHr = std::make_shared<HRESULT>(_hrPending);
            std::weak_ptr<HRESULT> _pWeakHr = _pHr;

            auto _pTaskRunner = SequencedTaskRunner::Create();
            auto _Task = _pTaskRunner->CreateTask(
                []()
                {
                    return 8848;
                });

            _Task.Then(
                _pTaskRunner,
                [_pWeakHr](int i) -> void
                {
                    auto _pHr = _pWeakHr.lock();
                    if (!_pHr)
                        return;

                    *_pHr = i;
                    WakeByAddressAll(_pHr.get());
                    return;
                });

            ::WaitOnAddress((volatile void*)_pHr.get(), (void*)&_hrPending, sizeof(_hrPending), 2000);
            Assert::AreEqual(HRESULT(8848), HRESULT(*_pHr));
        }

        TEST_METHOD(WhenAll语义)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();

            {
                volatile uint32_t _uCount = 0;

                auto _Task1 = _pTaskRunner->CreateTask(
                    [&_uCount]()
                    {
                        Sleep(10);
                        YY::Increment(&_uCount);
                        return 1;
                    });

                auto _Task2 = _pTaskRunner->CreateTask(
                    [&_uCount]()
                    {
                        Sleep(10);
                        YY::Increment(&_uCount);
                        return 2;
                    });

                auto _WhenAllTask = YY::WhenAll(_Task1, _Task2);

                Assert::IsTrue(_WhenAllTask.GetAsyncOperation()->WaitTask(YY::TimeSpan::FromMilliseconds(2000ul)));
                Assert::IsTrue(_WhenAllTask.GetStatus() == AsyncStatus::Completed);
                _WhenAllTask.GetResult();
                Assert::AreEqual((uint32_t)_uCount, uint32_t(2));
            }

            {
                auto _Task1 = _pTaskRunner->CreateTask(
                    []() -> int
                    {
                        throw YY::Exception(E_ACCESSDENIED);
                    });

                auto _Task2 = _pTaskRunner->CreateTask(
                    []()
                    {
                        Sleep(10);
                        return 2;
                    });

                auto _WhenAllTask = YY::WhenAll(_Task1, _Task2);

                Assert::IsTrue(_WhenAllTask.GetAsyncOperation()->WaitTask(YY::TimeSpan::FromMilliseconds(2000ul)));
                Assert::AreEqual(HRESULT(E_ACCESSDENIED), _WhenAllTask.GetResult());
            }
        }

        TEST_METHOD(WhenAny语义)
        {
            auto _pTaskRunner = SequencedTaskRunner::Create();

            {
                auto _Task1 = _pTaskRunner->CreateTask(
                    []()
                    {
                        Sleep(10);
                        return 1;
                    });

                auto _Task2 = _pTaskRunner->CreateTask(
                    []()
                    {
                        Sleep(100);
                        return 2;
                    });

                auto _WhenAnyTask = YY::WhenAny(_Task1, _Task2);

                Assert::IsTrue(_WhenAnyTask.GetAsyncOperation()->WaitTask(YY::TimeSpan::FromMilliseconds(2000ul)));
                Assert::IsTrue(_WhenAnyTask.GetStatus() == AsyncStatus::Completed);
                Assert::AreEqual(int32_t(0), _WhenAnyTask.GetResult());
            }

            {
                auto _Task1 = _pTaskRunner->CreateTask(
                    []() -> int
                    {
                        throw YY::Exception(E_ACCESSDENIED);
                    });

                auto _Task2 = _pTaskRunner->CreateTask(
                    []()
                    {
                        Sleep(10);
                        return 2;
                    });

                auto _WhenAnyTask = YY::WhenAny(_Task1, _Task2);

                Assert::IsTrue(_WhenAnyTask.GetAsyncOperation()->WaitTask(YY::TimeSpan::FromMilliseconds(2000ul)));
                Assert::AreEqual(int32_t(0), _WhenAnyTask.GetResult());
            }
        }
    };

    TEST_CLASS(CommonTaskRunnerUnitTest)
    {
    public:
#if defined(_HAS_CXX20) && _HAS_CXX20
        TEST_METHOD(AsyncSleep)
        {
            const auto _hrPending = E_PENDING;

            auto _pTaskRunner = SequencedTaskRunner::Create();

            auto _pHr = std::make_shared<HRESULT>(_hrPending);
            std::weak_ptr<HRESULT> _pWeakHr = _pHr;
            _pTaskRunner->PostTask(
                [_pWeakHr]() -> YY::Task<void>
                {
                    auto _pHr = _pWeakHr.lock();
                    if (!_pHr)
                        co_return;

                    *_pHr = co_await YY::TaskRunner::SleepAsync(YY::TimeSpan::FromMilliseconds(500));
                    WakeByAddressAll(_pHr.get());
                    co_return;
                });

            ::WaitOnAddress((volatile void*)_pHr.get(), (void*)&_hrPending, sizeof(_hrPending), 20000);

            Assert::AreEqual(HRESULT(S_OK), HRESULT(*_pHr));
        }
#endif

#if defined(_HAS_CXX20) && _HAS_CXX20 && defined(_WIN32)
        TEST_METHOD(AsyncWaitForObject)
        {
            const auto _hrPending = E_PENDING;

            auto _pTaskRunner = SequencedTaskRunner::Create();

            auto _pHr = std::make_shared<HRESULT>(_hrPending);
            std::weak_ptr<HRESULT> _pWeakHr = _pHr;
            _pTaskRunner->PostTask(
                [_pWeakHr]() -> YY::Task<void>
                {
                    auto _pHr = _pWeakHr.lock();
                    if (!_pHr)
                        co_return;

                    auto _hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

                    auto _uResult = co_await YY::TaskRunner::WaitForObjectAsync(_hEvent, YY::TimeSpan::FromMilliseconds(100));

                    Assert::AreEqual(DWORD(WAIT_TIMEOUT), DWORD(_uResult));

                    SetEvent(_hEvent);
                    _uResult = co_await YY::TaskRunner::WaitForObjectAsync(_hEvent, YY::TimeSpan::FromMilliseconds(100));
                    Assert::AreEqual(DWORD(WAIT_OBJECT_0), DWORD(_uResult));

                    *_pHr.get() = S_OK;
                    WakeByAddressAll(_pHr.get());
                    co_return;
                });

            ::WaitOnAddress((volatile void*)_pHr.get(), (void*)&_hrPending, sizeof(_hrPending), 2000);

            Assert::AreEqual(HRESULT(S_OK), HRESULT(*_pHr));
        }
#endif
    };
} // namespace UnitTest
