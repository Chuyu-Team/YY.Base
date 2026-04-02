#pragma once
#include <functional>

#include <Windows.h>

#include <YY/Base/YY.h>
#include <YY/Base/Strings/StringView.h>
#include <YY/Base/Threading/TaskRunner.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace IO
        {
            using namespace YY::Base::Threading;

            enum class ShareMode : uint32_t
            {
                None = 0u,
                Delete = FILE_SHARE_DELETE,
                Read = FILE_SHARE_READ,
                Write = FILE_SHARE_WRITE,
            };

            YY_APPLY_ENUM_CALSS_BIT_OPERATOR(ShareMode);

            enum class Access : uint32_t
            {
                None = 0u,
                Read = GENERIC_READ,
                Write = GENERIC_WRITE,
                Execute = GENERIC_EXECUTE,
                MaximumAllowed = MAXIMUM_ALLOWED,
            };

            YY_APPLY_ENUM_CALSS_BIT_OPERATOR(Access);

            class AsyncFile
            {
            protected:
                HANDLE hFile = INVALID_HANDLE_VALUE;
                bool bSkipCompletionNotificationOnSuccess = false;

                constexpr AsyncFile(HANDLE _hFile) noexcept
                    : hFile(_hFile)
                {
                    if (_hFile != INVALID_HANDLE_VALUE)
                    {
                        bSkipCompletionNotificationOnSuccess = SetFileCompletionNotificationModes(hFile, FILE_SKIP_COMPLETION_PORT_ON_SUCCESS | FILE_SKIP_SET_EVENT_ON_HANDLE);
                    }
                }

            public:
                constexpr AsyncFile() noexcept = default;

                AsyncFile(AsyncFile&& _oOther) noexcept
                    : hFile(_oOther.hFile)
                {
                    _oOther.hFile = INVALID_HANDLE_VALUE;
                }

                ~AsyncFile() noexcept
                {
                    Close();
                }

                AsyncFile(const AsyncFile&) = delete;
                AsyncFile& operator=(const AsyncFile&) = delete;

                AsyncFile& __YYAPI operator=(AsyncFile&& _oOther) noexcept
                {
                    if (hFile != _oOther.hFile)
                    {
                        Close();
                        hFile = _oOther.hFile;
                        _oOther.hFile = INVALID_HANDLE_VALUE;

                        bSkipCompletionNotificationOnSuccess = _oOther.bSkipCompletionNotificationOnSuccess;
                    }
                    return *this;
                }

                HANDLE __YYAPI GetNativeHandle() const noexcept
                {
                    return hFile;
                }

                bool __YYAPI IsValid() const noexcept
                {
                    return hFile != INVALID_HANDLE_VALUE;
                }

                static AsyncFile __YYAPI Open(_In_z_ const uchar_t* _szFilePath, _In_ Access _eAccess, _In_ ShareMode _eShareMode = ShareMode::None) noexcept
                {
                    auto _hFile = CreateFileW(_szFilePath, static_cast<DWORD>(_eAccess), static_cast<DWORD>(_eShareMode), nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

                    if (_hFile != INVALID_HANDLE_VALUE)
                    {
                        if (!TaskRunner::BindIO(_hFile))
                        {
                            CloseHandle(_hFile);
                            _hFile = INVALID_HANDLE_VALUE;
                        }
                    }

                    return AsyncFile(_hFile);
                }

                LSTATUS __YYAPI Close() noexcept
                {
                    if (hFile != INVALID_HANDLE_VALUE)
                    {
                        if (!CloseHandle(hFile))
                        {
                            return GetLastError();
                        }

                        hFile = INVALID_HANDLE_VALUE;
                    }

                    return ERROR_SUCCESS;
                }

                /// <summary>
                /// 异步读取文件。
                /// </summary>
                /// <param name="_uOffset">读取文件的偏移。</param>
                /// <param name="_pBuffer">输入缓冲区。请确保读取期间，_pBuffer处于有效状态。</param>
                /// <param name="_cbBufferToRead">要读取的最大字节数。</param>
                /// <returns>返回实际读取的字节数。
                /// 如果实际读取字节数为 0，那么请额外检查 GetLastError()。</returns>
                Task<uint32_t> __YYAPI ReadAsync(
                    _In_ uint64_t _uOffset,
                    _Out_writes_bytes_(_cbBufferToRead) void* _pBuffer,
                    _In_ uint32_t _cbBufferToRead) noexcept;

                /// <summary>
                /// 异步写入文件。
                /// </summary>
                /// <param name="_uOffset">写入文件的偏移。</param>
                /// <param name="_pBuffer">需要写入的数据缓冲区。</param>
                /// <param name="_cbBufferToWrite">要写入的字节数</param>
                /// <returns>返回实际写入的字节数。
                /// 如果实际写入字节数为 0，那么请额外检查 GetLastError()。
                /// </returns>
                Task<uint32_t> __YYAPI WriteAsync(
                    _In_ uint64_t _uOffset,
                    _In_reads_bytes_(_cbBufferToWrite) const void* _pBuffer,
                    _In_ uint32_t _cbBufferToWrite) noexcept;

                /// <summary>
                /// 异步读取文件。
                /// </summary>
                /// <param name="_uOffset">读取文件的偏移。</param>
                /// <param name="_pBuffer">输入缓冲区。请确保读取期间，_pBuffer处于有效状态。</param>
                /// <param name="_cbBufferToRead">要读取的最大字节数。</param>
                /// <param name="_pfnResultCallback">异步完成后，执行的回调。回调保证恢复调用AsyncRead时的线程上下文。
                ///   _lStatus : 操作返回代码，如果返回 ERROR_SUCCESS，那么代表成功。
                ///   _cbRead : 实际读取成功的字节数。
                /// </param>
                /// <returns>
                /// ERROR_SUCCESS: 读取成功，但是数据尚未就绪，需要等待 _pfnResultCallback 完成。
                /// 其他值：失败。
                /// </returns>
                __declspec(deprecated("已经废弃，建议直接使用ReadAsync。"))
                LSTATUS __YYAPI AsyncRead(
                    _In_ uint64_t _uOffset,
                    _In_reads_bytes_(_cbBufferToRead) void* _pBuffer,
                    _In_ uint32_t _cbBufferToRead,
                    _In_ std::function<void(LSTATUS _lStatus, uint32_t _cbRead)> _pfnResultCallback) noexcept
                {
                    auto _oTask = ReadAsync(_uOffset, _pBuffer, _cbBufferToRead);
                    switch (_oTask.GetStatus())
                    {
                    case AsyncStatus::Canceled:
                        return ERROR_CANCELLED;
                    case AsyncStatus::Completed:
                        _pfnResultCallback(ERROR_SUCCESS, _oTask.GetResult());
                        return ERROR_SUCCESS;
                    case AsyncStatus::Error:
                        return HRESULT_CODE(_oTask.GetErrorCode());
                    default:
                        _oTask.Then([_pfnResultCallback = std::move(_pfnResultCallback)](uint32_t _cbRead) mutable
                            {
                                const auto _lStatus = _cbRead == 0 ? GetLastError() : ERROR_SUCCESS;
                                _pfnResultCallback(_lStatus, _cbRead);
                            });

                        return ERROR_SUCCESS;
                        break;
                    }
                }

                /// <summary>
                /// 异步写入文件。
                /// </summary>
                /// <param name="_uOffset">写入文件的偏移。</param>
                /// <param name="_pBuffer">需要写入的数据缓冲区。</param>
                /// <param name="_cbBufferToWrite">要写入的字节数</param>
                /// <param name="_pfnResultCallback">异步完成后，执行的回调。回调保证恢复调用AsyncWrite时的线程上下文。
                ///   _lStatus : 操作返回代码，如果返回 ERROR_SUCCESS，那么代表成功。
                ///   _cbWrite : 实际写入成功的字节数。
                /// </param>
                /// <returns>
                /// ERROR_SUCCESS: 读取成功，但是数据尚未就绪，需要等待 _pfnResultCallback 完成。
                /// 其他值：失败。
                /// </returns>
                __declspec(deprecated("已经废弃，建议直接使用WriteAsync。"))
                LSTATUS __YYAPI AsyncWrite(
                    _In_ uint64_t _uOffset,
                    _In_reads_bytes_(_cbBufferToWrite) const void* _pBuffer,
                    _In_ uint32_t _cbBufferToWrite,
                    _In_ std::function<void(LSTATUS _lStatus, uint32_t _cbWrite)> _pfnResultCallback) noexcept
                {
                    auto _oTask = WriteAsync(_uOffset, _pBuffer, _cbBufferToWrite);
                    
                    switch (_oTask.GetStatus())
                    {
                    case AsyncStatus::Canceled:
                        return ERROR_CANCELLED;
                    case AsyncStatus::Completed:
                        _pfnResultCallback(ERROR_SUCCESS, _oTask.GetResult());
                        return ERROR_SUCCESS;
                    case AsyncStatus::Error:
                        return HRESULT_CODE(_oTask.GetErrorCode());
                    default:
                        _oTask.Then([_pfnResultCallback = std::move(_pfnResultCallback)](uint32_t _cbWrite) mutable
                            {
                                const auto _lStatus = _cbWrite == 0 ? GetLastError() : ERROR_SUCCESS;
                                _pfnResultCallback(_lStatus, _cbWrite);
                            });

                        return ERROR_SUCCESS;
                        break;
                    }
                }
            };

            class AsyncPipe : public AsyncFile
            {
            protected:
                constexpr AsyncPipe(HANDLE _hFile) noexcept
                    : AsyncFile(_hFile)
                {
                }

                AsyncPipe(AsyncFile&& _hFile) noexcept
                    : AsyncFile(std::move(_hFile))
                {
                }

            public:
                constexpr AsyncPipe() = default;

                AsyncPipe(AsyncPipe&& _oOther) noexcept
                    : AsyncFile(std::move(_oOther))
                {
                }

                AsyncPipe& __YYAPI operator=(AsyncPipe&& _oOther) noexcept
                {
                    if (hFile != _oOther.hFile)
                    {
                        AsyncFile::operator=(std::move(_oOther));
                    }
                    return *this;
                }

                static AsyncPipe Create(
                    _In_z_ const uchar_t* _szPipeName,
                    _In_ DWORD _fOpenMode,
                    _In_ DWORD _fPipeMode,
                    _In_ DWORD _uMaxInstances,
                    _In_ DWORD _cbOutBufferSize,
                    _In_ DWORD _cbInBufferSize,
                    _In_ DWORD _uDefaultTimeOut,
                    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes = nullptr)
                {
                    auto hPipe = CreateNamedPipeW(
                        _szPipeName,
                        _fOpenMode | FILE_FLAG_OVERLAPPED,
                        _fPipeMode,
                        _uMaxInstances,
                        _cbOutBufferSize,
                        _cbInBufferSize,
                        _uDefaultTimeOut,
                        nullptr);

                    if (hPipe != INVALID_HANDLE_VALUE)
                    {
                        if (!TaskRunner::BindIO(hPipe))
                        {
                            CloseHandle(hPipe);
                            hPipe = INVALID_HANDLE_VALUE;
                        }
                    }

                    return AsyncPipe(hPipe);
                }

                static AsyncPipe __YYAPI Open(_In_z_ const uchar_t* _szFilePath, _In_ Access _eAccess, _In_ ShareMode _eShareMode = ShareMode::None) noexcept
                {
                    auto _File = AsyncFile::Open(_szFilePath, _eAccess, _eShareMode);
                    return AsyncPipe(std::move(_File));
                }

                /// <summary>
                /// 异步链接管道。
                /// </summary>
                /// <returns>一个任务对象,表示异步操作,完成时返回操作的状态码。</returns>
                Task<LSTATUS> __YYAPI ConnectAsync();

                /// <summary>
                /// 异步链接管道。
                /// </summary>
                /// <param name="_pfnResultCallback">异步完成后，执行的回调。回调保证恢复调用AsyncConnect时的线程上下文。
                ///     _lStatus: 管道链接的错误代码，ERROR_SUCCESS代表成功。
                /// </param>
                /// <returns>
                /// ERROR_SUCCESS: 读取成功，但是数据尚未就绪，需要等待 _pfnResultCallback 完成。
                /// 其他值：失败。
                /// </returns>
                __declspec(deprecated("已经废弃，建议直接使用ConnectAsync。"))
                LSTATUS __YYAPI AsyncConnect(_In_ std::function<void(LSTATUS _lStatus)> _pfnResultCallback) noexcept
                {
                    auto _oTask = ConnectAsync();
                    switch (_oTask.GetStatus())
                    {
                    case AsyncStatus::Canceled:
                        return ERROR_CANCELLED;
                    case AsyncStatus::Completed:
                        _pfnResultCallback(_oTask.GetResult());
                        return ERROR_SUCCESS;
                    case AsyncStatus::Error:
                        return HRESULT_CODE(_oTask.GetErrorCode());
                    default:
                        _oTask.Then([_pfnResultCallback = std::move(_pfnResultCallback)](LSTATUS _lStatus) mutable
                            {
                                _pfnResultCallback(_lStatus);
                            });

                        return ERROR_SUCCESS;
                        break;
                    }
                }
            };
        }
    }
}

namespace YY
{
    using namespace YY::Base::IO;
}

#pragma pack(pop)
