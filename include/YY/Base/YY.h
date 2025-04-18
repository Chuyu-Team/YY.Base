﻿
/*
本文定义了 YY基础库基本的数据类型（类似于 stdint.h），这样做是为了更好的进行跨平台移植工作

我们优先使用 stdint.h 的内容，因为它属于 C++ 标准，更便于人们理解
*/

#pragma once
#include <stdint.h>
#include <cstddef>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // 从 Windows 头文件中排除极少使用的内容
#endif
#include <Windows.h>
#endif

#include <YY/Base/SAL.h>
#define __YY_PACKING 4

#ifndef __YYAPI
#ifdef _WIN32
#define __YYAPI __fastcall
#else
#define __YYAPI
#endif // _WIN32
#endif // !__YYAPI

#if defined(__has_builtin)
#if __has_builtin(__builtin_offsetof)
#define YY_UFIELD_OFFSET(type, field) ((size_t)__builtin_offsetof(type, field))
#endif // __has_builtin(__builtin_offsetof)
#endif // defined(__has_builtin)

#ifndef YY_UFIELD_OFFSET
#define YY_UFIELD_OFFSET(type, field) ((size_t)&(((type *)0)->field))
#endif

#define YY_FIELD_SIZE(type, field) (sizeof(((type *)0)->field))

#define YY_SIZEOF_THROUGH_FIELD(type, field) \
    (YY_UFIELD_OFFSET(type, field) + YY_FIELD_SIZE(type, field))


#define _YY_CONCATENATE_(a, b) a##b
#define _YY_CONCATENATE(a, b) _YY_CONCATENATE_(a, b)



#define YY_APPLY_ENUM_CALSS_BIT_OPERATOR(_ENUM)                      \
    inline constexpr _ENUM& operator|=(_ENUM& _eLeft, _ENUM _eRight) \
    {                                                                \
        using _Type = std::underlying_type<_ENUM>::type;             \
        (_Type&)_eLeft |= (_Type)_eRight;                            \
        return _eLeft;                                               \
    }                                                                \
                                                                     \
    inline constexpr _ENUM operator|(_ENUM _eLeft, _ENUM _eRight)    \
    {                                                                \
        using _Type = std::underlying_type<_ENUM>::type;             \
        auto _Result = (_Type)_eLeft | (_Type)_eRight;               \
        return _ENUM(_Result);                                       \
    }                                                                \
                                                                     \
    inline constexpr _ENUM& operator&=(_ENUM& _eLeft, _ENUM _eRight) \
    {                                                                \
        using _Type = std::underlying_type<_ENUM>::type;             \
        (_Type&)_eLeft &= (_Type)_eRight;                            \
        return _eLeft;                                               \
    }                                                                \
                                                                     \
    inline constexpr _ENUM operator&(_ENUM _eLeft, _ENUM _eRight)    \
    {                                                                \
        using _Type = std::underlying_type<_ENUM>::type;             \
        return _ENUM((_Type)_eLeft & (_Type)_eRight);                \
    }                                                                \
                                                                     \
    inline constexpr _ENUM& operator^=(_ENUM& _eLeft, _ENUM _eRight) \
    {                                                                \
        using _Type = std::underlying_type<_ENUM>::type;             \
        (_Type&)_eLeft ^= (_Type)_eRight;                            \
        return _eLeft;                                               \
    }                                                                \
                                                                     \
    inline constexpr _ENUM operator^(_ENUM _eLeft, _ENUM _eRight)    \
    {                                                                \
        using _Type = std::underlying_type<_ENUM>::type;             \
        return _ENUM((_Type)_eLeft ^ (_Type)_eRight);                \
    }                                                                \
                                                                     \
    inline constexpr _ENUM operator~(_ENUM _eLeft)                   \
    {                                                                \
        using _Type = std::underlying_type<_ENUM>::type;             \
        return _ENUM(~(_Type)_eLeft);                                \
    }                                                                \
                                                                     \
    inline constexpr bool HasFlags(_ENUM _eLeft, _ENUM _eRight)      \
    {                                                                \
        using _Type = std::underlying_type<_ENUM>::type;             \
        return (_Type)_eLeft & (_Type)_eRight;                       \
    }

namespace YY
{
    namespace Base
    {
        using ::int8_t;
        using ::int16_t;
        using ::int32_t;
        using ::int64_t;

        using ::uint8_t;
        using ::uint16_t;
        using ::uint32_t;
        using ::uint64_t;

        using ::intmax_t;
        using ::uintmax_t;

        using ::intptr_t;
        using ::size_t;
        typedef size_t uintptr_t;

        typedef ::intptr_t int_t;
        typedef ::size_t uint_t;

#ifdef _MSC_VER
        // ANSI字符
        typedef char achar_t;
        
#if defined(__cpp_lib_char8_t) || defined(__cpp_char8_t)
        // UTF8 字符
        typedef char8_t u8char_t;
#else
        // UTF8 字符
        typedef char u8char_t;
#endif

        // UTF16字符，Windows平台 wchar_t，就是UTF16
        typedef wchar_t u16char_t;

        // UTF32字符
        typedef char32_t u32char_t;

        // 当前平台的推荐Unicode字符
        typedef u16char_t uchar_t;
#else // !_MSC_VER
        // ANSI字符
        typedef unsigned char achar_t;

#if defined(__cpp_lib_char8_t) || defined(__cpp_char8_t)
        // UTF8 字符
        typedef char8_t u8char_t;
#else
        // UTF8 字符
        typedef char u8char_t;
#endif
        // UTF16字符
        typedef char16_t u16char_t;

        // UTF32字符
        typedef wchar_t u32char_t;

        // 当前平台的推荐字符
        typedef u8char_t uchar_t;

        static_assert(sizeof(wchar_t) == 4, "wchar_t UTF32");
#endif // _MSC_VER

        typedef _Null_terminated_ achar_t* raw_astring_t;
        typedef _Null_terminated_ const achar_t* raw_const_astring_t;

        typedef _Null_terminated_ u8char_t* raw_u8string;
        typedef _Null_terminated_ const u8char_t* raw_const_u8string_t;

        typedef _Null_terminated_ u16char_t* raw_u18string_t;
        typedef _Null_terminated_ const u16char_t* raw_const_u16string_t;

        typedef _Null_terminated_ u32char_t* raw_u32string_t;
        typedef _Null_terminated_ const u32char_t* raw_const_u32string_t;

        // 当前平台的推荐的Unicode字符串类型
        typedef _Null_terminated_ uchar_t* raw_ustring_t;
        // 当前平台的推荐的只读Unicode字符串类型
        typedef _Null_terminated_ const uchar_t* raw_const_ustring_t;

#define _U8S(S) u8##S

#ifdef _MSC_VER
#define _U16S(S) L##S
#else
#define _U16S(S) u##S
#endif

#define _U32S(S) U##S

#ifdef _MSC_VER
#define _S(S) _U16S(S)
#else
#define _S(S) _U8S(S)
#endif

#ifdef _WIN32
        using ::HRESULT;
        using ::LSTATUS;
#else
        typedef _Return_type_success_(return >= 0) long HRESULT;
        typedef _Return_type_success_(return == /*ERROR_SUCCESS*/ 0) long LSTATUS;
#endif

        // 字节
        typedef unsigned char byte_t;

// 返回bit位数，注意：一个byte不一定是 8bit，但是 uint32_t 一定是32bit。
#define YY_bitsizeof(_Type) (sizeof(_Type) * 32 * sizeof(byte_t) / sizeof(uint32_t))
    } // namespace Base
    
    using namespace YY::Base;
} // namespace YY
