# YY.Base
本仓库主要提供了YY.Base的基础功能，主要包括：
* 线程池能力
  - YY/Base/Threading/TaskRunner.h
* 字符串
  - YY/Base/Strings/String.h
  - YY/Base/Strings/StringView
* 容器
  - YY/Base/Containers/Array.h
  - YY/Base/Containers/Span.h
  - YY/Base/Containers/BitMap.h
  - YY/Base/Containers/Optional.h
  - YY/Base/Containers/SingleLinkedList.h
> 更多请参考：include文件夹中的内容。

## 示例
### 将主线程绑定到TaskRunner，并投递任务
```cpp
#include <YY/Base/Threading/TaskRunner.h>

int main()
{
    auto _pMainThreadRunner = YY::Base::Threading::ThreadTaskRunner::BindCurrentThread();
    _pMainThreadRunner->PostTask(
        []()
        {
            std::cout << "此任务会在调用 RunUIMessageLoop 后开始。\n";
        });

    // 运行UI消息循环，在Windows平台等效于GetMessage
    return YY::Base::Threading::ThreadTaskRunner::RunUIMessageLoop();
}
```

### 向一个虚拟线程投递任务
```cpp
#include <YY/Base/Threading/TaskRunner.h>

int main()
{
    // 从线程池创建一个逻辑线程，任务执行完成后，自动归还线程池，平时不占用任何资源。
    // 该逻辑线程只保证提交的任务是串行执行的，不保证在每次同一个物理线程上执行！
    // `异步线程1` 是逻辑线程的名称，便于调试时区分。
    auto _pAsynTaskRunner = Threading::SequencedTaskRunner::Create(L"异步线程1");

    // 向 异步线程1投递一个任务，该任务将在后续执行。
    // 投递的任务只保证在顺序执行，无法保证在同一个物理线程上执行。
    _pAsynTaskRunner->PostTask(
        []()
        {
            std::cout << "任务会在某个物理线程执行。\n";
        });

    // ...
    return 0;
}
```

### 投递一个延时任务
```cpp
#include <YY/Base/Threading/TaskRunner.h>

int main()
{
    // 从线程池创建一个逻辑线程，任务执行完成后，自动归还线程池，平时不占用任何资源。
    // 该逻辑线程只保证提交的任务是串行执行的，不保证在每次同一个物理线程上执行！
    // `异步线程1` 是逻辑线程的名称，便于调试时区分。
    auto _pAsynTaskRunner = Threading::SequencedTaskRunner::Create(L"异步线程1");

    // 向 异步线程1投递一个任务，该任务将在后续执行。
    // 投递的任务只保证在顺序执行，无法保证在同一个物理线程上执行。
    _pAsynTaskRunner->PostDelayTask(
        TimeSpan::FromMilliseconds(100),
        []()
        {
            std::cout << "延迟100ms任务后执行。\n";
        });

    // ...
    return 0;
}
```

### 投递一个延时任务
```cpp
#include <YY/Base/Threading/TaskRunner.h>

int main()
{
    // 从线程池创建一个逻辑线程，任务执行完成后，自动归还线程池，平时不占用任何资源。
    // 该逻辑线程只保证提交的任务是串行执行的，不保证在每次同一个物理线程上执行！
    // `异步线程1` 是逻辑线程的名称，便于调试时区分。
    auto _pAsynTaskRunner = Threading::SequencedTaskRunner::Create(L"异步线程1");

    // 向 异步线程1投递一个任务，该任务将在后续执行。
    // 投递的任务只保证在顺序执行，无法保证在同一个物理线程上执行。
    _pAsynTaskRunner->PostDelayTask(
        TimeSpan::FromMilliseconds(100),
        []()
        {
            std::cout << "延迟100ms任务后执行。\n";
        });

    // ...
    return 0;
}
```

### 监听句柄是否有信号
```cpp
#include <YY/Base/Threading/TaskRunner.h>
int main()
{
    // 从线程池创建一个逻辑线程，任务执行完成后，自动归还线程池，平时不占用任何资源。
    // 该逻辑线程只保证提交的任务是串行执行的，不保证在每次同一个物理线程上执行！
    // `异步线程1` 是逻辑线程的名称，便于调试时区分。
    auto _pAsynTaskRunner = Threading::SequencedTaskRunner::Create(L"异步线程1");

    auto _hEevent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    // 让等待一个Eevent句柄，有信号时让主线程输出“_hEevent 有信号时该任务将执行。”。
    _pAsynTaskRunner->CreateWait(
        _hEevent,
        [](DWORD)
        {
            std::cout << "_hEevent 有信号时该任务将执行。\n";

            // 如果需要继续监听该句柄，请返回true。
            return false;
        });

    // ...
    return 0;
}
```
