#pragma once
#include <exception>
#include <YY/Base/ErrorCode.h>

#include <YY/Base/YY.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        class Exception : public std::exception
        {
        private:
            const uchar_t* szErrorMessage;
            HRESULT hr;

        public:
            /// <summary>
            /// 构造一个 Exception
            /// </summary>
            /// <param name="_szErrorMessage">必须指向一个常量或者有用足够声明周期的缓冲区，Exception不负责维护</param>
            /// <param name="_hr">错误代码</param>
            Exception(const uchar_t* _szErrorMessage = nullptr, HRESULT _hr = 0x8000FFFFL /* E_UNEXPECTED */) noexcept
                : szErrorMessage(_szErrorMessage)
                , hr(_hr)
            {
            }

            Exception(HRESULT _hr) noexcept
                : szErrorMessage(nullptr)
                , hr(_hr)
            {
            }

            HRESULT __YYAPI GetErrorCode() const noexcept
            {
                return hr;
            }

            const uchar_t* __YYAPI GetErrorMessage() const noexcept
            {
                return szErrorMessage ? szErrorMessage : _S("");
            }
        };

        /// <summary>
        /// 操作已经被取消。
        /// </summary>
        class OperationCanceledException : public Exception
        {
        public:
            OperationCanceledException(const uchar_t* _szErrorMessage = nullptr) noexcept
                : Exception(_szErrorMessage, HRESULT_From_LSTATUS(ERROR_CANCELLED))
            {
            }
        };
    } // namespace Base
} // namespace YY

#pragma pack(pop)
