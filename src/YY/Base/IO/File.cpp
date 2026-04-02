#include <YY/Base/IO/File.h>

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

namespace YY {
namespace Base {
namespace IO {

class FileIoAsyncOperation : public IoAsyncOperation<uint32_t>
{
public:
    uint32_t& __YYAPI GetResult() override
    {
        ThrowIfWaitTaskFailed();

        if (InternalHigh == 0)
        {
            SetLastError(lStatus);
        }

        return (uint32_t&)InternalHigh;
    }
};

Task<uint32_t>__YYAPI AsyncFile::ReadAsync(uint64_t _uOffset, void* _pBuffer, uint32_t _cbBufferToRead) noexcept
{
    auto _pFileIoAsyncOperation = RefPtr<FileIoAsyncOperation>::Create();
    _pFileIoAsyncOperation->Offset = (uint32_t)_uOffset;
    _pFileIoAsyncOperation->OffsetHigh = (uint32_t)(_uOffset >> 32);

    if (ReadFile(hFile, _pBuffer, _cbBufferToRead, nullptr, _pFileIoAsyncOperation.Clone()))
    {
        // 读取成功
        _pFileIoAsyncOperation->Resolve(ERROR_SUCCESS);

        if (bSkipCompletionNotificationOnSuccess)
        {
            _pFileIoAsyncOperation.Get()->Release();
        }
        else
        {
            TaskRunner::StartIo();
        }
    }
    else
    {
        const auto _lStatus = GetLastError();
        if (_lStatus == ERROR_IO_PENDING)
        {
            // 进入异步读取模式，唤醒一下 Dispatch，IO完成后Dispatch自动会将任务重新转发到调用者
            TaskRunner::StartIo();
        }
        else
        {
            // 失败！
            _pFileIoAsyncOperation->Resolve(_lStatus);
            _pFileIoAsyncOperation.Get()->Release();
        }
    }

    return Task<uint32_t>(std::move(_pFileIoAsyncOperation));
}

Task<uint32_t>__YYAPI AsyncFile::WriteAsync(uint64_t _uOffset, const void* _pBuffer, uint32_t _cbBufferToWrite) noexcept
{
    auto _pFileIoAsyncOperation = RefPtr<FileIoAsyncOperation>::Create();
    _pFileIoAsyncOperation->Offset = (uint32_t)_uOffset;
    _pFileIoAsyncOperation->OffsetHigh = (uint32_t)(_uOffset >> 32);

    if (WriteFile(hFile, _pBuffer, _cbBufferToWrite, nullptr, _pFileIoAsyncOperation.Clone()))
    {
        // 写入成功
        _pFileIoAsyncOperation->Resolve(ERROR_SUCCESS);

        if (bSkipCompletionNotificationOnSuccess)
        {
            _pFileIoAsyncOperation.Get()->Release();
        }
        else
        {
            TaskRunner::StartIo();
        }
    }
    else
    {
        const auto _lStatus = GetLastError();
        if (_lStatus == ERROR_IO_PENDING)
        {
            // 进入异步读取模式，唤醒一下 Dispatch，IO完成后Dispatch自动会将任务重新转发到调用者
            TaskRunner::StartIo();
        }
        else
        {
            // 失败！
            _pFileIoAsyncOperation->Resolve(_lStatus);
            _pFileIoAsyncOperation.Get()->Release();
        }
    }

    return Task<uint32_t>(std::move(_pFileIoAsyncOperation));
}

Task<LSTATUS>__YYAPI AsyncPipe::ConnectAsync()
{
    class ConnectAsyncOperation : public IoAsyncOperation<LSTATUS>
    {
    public:
        LSTATUS& __YYAPI GetResult() override
        {
            ThrowIfWaitTaskFailed();
            return lStatus;
        }
    };

    auto _pConnectAsyncOperation = YY::RefPtr<ConnectAsyncOperation>::Create();

    auto _bSuccess = ConnectNamedPipe(hFile, _pConnectAsyncOperation.Clone());
    if (_bSuccess)
    {
        _pConnectAsyncOperation->Resolve(ERROR_SUCCESS);

        if (bSkipCompletionNotificationOnSuccess)
        {
            _pConnectAsyncOperation.Get()->Release();
        }
        else
        {
            TaskRunner::StartIo();
        }
    }
    else
    {
        const auto _lStatus = GetLastError();
        if (_lStatus == ERROR_IO_PENDING)
        {
            // 进入异步读取模式，唤醒一下 Dispatch，IO完成后Dispatch自动会将任务重新转发到调用者
            TaskRunner::StartIo();
        }
        else
        {
            // 失败！
            _pConnectAsyncOperation->Resolve(_lStatus);
            _pConnectAsyncOperation.Get()->Release();
        }
    }

    return Task<LSTATUS>(std::move(_pConnectAsyncOperation));
}

}
}
}
