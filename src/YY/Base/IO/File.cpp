#include <YY/Base/IO/File.h>

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

namespace YY {
namespace Base {
namespace IO {

class FileIoAsyncOperation
    : public IoAsyncOperation<uint32_t>
    , public CancellationTokenCancelHandle
{
public:
    HANDLE hFile = INVALID_HANDLE_VALUE;

    FileIoAsyncOperation(_In_opt_ YY::RefPtr<CancellationToken> _pCancellationToken = nullptr) noexcept
        : IoAsyncOperation<uint32_t>(std::move(_pCancellationToken))
    {
    }

    ~FileIoAsyncOperation() noexcept
    {
        if (auto _pCancellationToken = GetCancellationToken())
        {
            _pCancellationToken->Unregister(this);
        }
    }

    void __YYAPI OnCanceled() override
    {
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CancelIoEx(hFile, this);
        }
    }

    uint32_t& __YYAPI GetResult() override
    {
        ThrowIfWaitTaskFailed();

        if (InternalHigh == 0)
        {
            SetLastError(lStatus);
        }

        return (uint32_t&)InternalHigh;
    }

    bool __YYAPI Cancel() override
    {
        OnCanceled();
        return IoAsyncOperation<uint32_t>::Cancel();
    }
};

Task<uint32_t>__YYAPI AsyncFile::ReadAsync(uint64_t _uOffset, void* _pBuffer, uint32_t _cbBufferToRead, YY::RefPtr<CancellationToken> _pCancellationToken) noexcept
{
    auto _pFileIoAsyncOperation = RefPtr<FileIoAsyncOperation>::Create(_pCancellationToken);
    _pFileIoAsyncOperation->Offset = (uint32_t)_uOffset;
    _pFileIoAsyncOperation->OffsetHigh = (uint32_t)(_uOffset >> 32);

    if (_pCancellationToken && _pCancellationToken->IsCancellationRequested())
    {
        _pFileIoAsyncOperation->Cancel();
        return Task<uint32_t>(std::move(_pFileIoAsyncOperation));
    }

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
            _pFileIoAsyncOperation->hFile = hFile;
            if (_pFileIoAsyncOperation->IsCanceled())
            {
                _pFileIoAsyncOperation->Cancel();
            }
            else if (_pCancellationToken)
            {
                _pCancellationToken->Register(_pFileIoAsyncOperation);
            }

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

Task<uint32_t>__YYAPI AsyncFile::WriteAsync(uint64_t _uOffset, const void* _pBuffer, uint32_t _cbBufferToWrite, YY::RefPtr<CancellationToken> _pCancellationToken) noexcept
{
    auto _pFileIoAsyncOperation = RefPtr<FileIoAsyncOperation>::Create(_pCancellationToken);
    _pFileIoAsyncOperation->Offset = (uint32_t)_uOffset;
    _pFileIoAsyncOperation->OffsetHigh = (uint32_t)(_uOffset >> 32);

    if (_pCancellationToken && _pCancellationToken->IsCancellationRequested())
    {
        _pFileIoAsyncOperation->Cancel();
        return Task<uint32_t>(std::move(_pFileIoAsyncOperation));
    }

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
            _pFileIoAsyncOperation->hFile = hFile;
            if (_pFileIoAsyncOperation->IsCanceled())
            {
                _pFileIoAsyncOperation->Cancel();
            }
            else if (_pCancellationToken)
            {
                _pCancellationToken->Register(_pFileIoAsyncOperation);
            }

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

Task<LSTATUS>__YYAPI AsyncPipe::ConnectAsync(YY::RefPtr<CancellationToken> _pCancellationToken)
{
    class ConnectAsyncOperation
        : public IoAsyncOperation<LSTATUS>
        , public CancellationTokenCancelHandle
    {
    public:
        HANDLE hFile = INVALID_HANDLE_VALUE;

        ConnectAsyncOperation(YY::RefPtr<CancellationToken> _pCancellationToken = nullptr) noexcept
            : IoAsyncOperation<LSTATUS>(std::move(_pCancellationToken))
        {
        }

        ~ConnectAsyncOperation() noexcept
        {
            if (auto _pCancellationToken = GetCancellationToken())
            {
                _pCancellationToken->Unregister(this);
            }
        }

        void __YYAPI OnCanceled() override
        {
            if (hFile != INVALID_HANDLE_VALUE)
            {
                CancelIoEx(hFile, this);
            }
        }

        LSTATUS& __YYAPI GetResult() override
        {
            ThrowIfWaitTaskFailed();
            return lStatus;
        }

        bool __YYAPI Cancel() override
        {
            OnCanceled();
            return IoAsyncOperation<LSTATUS>::Cancel();
        }
    };

    auto _pConnectAsyncOperation = YY::RefPtr<ConnectAsyncOperation>::Create(_pCancellationToken);

    if (_pCancellationToken && _pCancellationToken->IsCancellationRequested())
    {
        _pConnectAsyncOperation->Cancel();
        return Task<LSTATUS>(std::move(_pConnectAsyncOperation));
    }

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
            _pConnectAsyncOperation->hFile = hFile;
            if (_pConnectAsyncOperation->IsCanceled())
            {
                _pConnectAsyncOperation->Cancel();
            }
            else if (_pCancellationToken)
            {
                _pCancellationToken->Register(_pConnectAsyncOperation);
            }

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
