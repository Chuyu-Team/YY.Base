#pragma once

#include <YY/Base/YY.h>
#include <YY/Base/Strings/String.h>

#pragma pack(push, __YY_PACKING)

/*
一开始Path被设计为类似于Java的跨平台的路径处理类，但是这样做的话其他打开路径的API不太方便直接传递裸C风格指针。

目前暂时设计为根.NET类似的工具类，提供一些静态方法来处理路径字符串。

# Windows平台：
    相对路径
       - 当前驱动器的相对路径：\mydir\test.txt
       - 当前目录的相对路径：mydir\test.txt
    传统Dos路径
       - 驱动器根目录的绝对文件路径：C:\mydir\test.txt
    Dos设备路径
       - \\?\C:\mydir\test.txt
       - \\?\Volume{GUID}\mydir\test.txt
       - UNC形式：\\?\UNC\ComputerName\SharedFolder\mydir\test.txt
    UNC路径：
       - \\ComputerName\SharedFolder\mydir\test.txt

# Linux/Unix平台：
    相对路径
       - 当前目录的相对路径：mydir/test.txt
    绝对路径
       - /mydir/test.txt
*/

#pragma push_macro("GetCurrentDirectory")
#undef GetCurrentDirectory

namespace YY
{
    namespace Base
    {
        namespace IO
        {
            class Path
            {
            public:
#if defined(WIN32) || defined(_WIN32)
                static constexpr uchar_t kAltPathSeparatorChar = '/';
                static constexpr uchar_t kDirectorySeparatorChar = '\\';
                static constexpr uchar_t kPathSeparatorChar = ';';
                static constexpr uchar_t kVolumeSeparatorChar = ':';

                static constexpr uchar_t kDirectorySeparatorChars[] = { kDirectorySeparatorChar, kAltPathSeparatorChar };
                static constexpr uchar_t kInvalidFileNameChars[] = { '\"', '<', '>', '|', ':', '*', '?', '\\', '/', '\0', '\x1', '\x2', '\x3', '\x4', '\x5', '\x6', '\a', '\b', '\t', '\n', '\v', '\f', '\r', '\xE', '\xF', '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F' };
                static constexpr uchar_t kInvalidPathChars[] = { '|', '\0', '\x1', '\x2', '\x3', '\x4', '\x5', '\x6', '\a', '\b', '\t', '\n', '\v', '\f', '\r', '\xE', '\xF', '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F' };
#elif defined(__linux__) || defined(__unix__)
                static constexpr uchar_t kAltPathSeparatorChar = '/';
                static constexpr uchar_t kDirectorySeparatorChar = '/';
                static constexpr uchar_t kPathSeparatorChar = ';';
                static constexpr uchar_t kVolumeSeparatorChar = '/';

                static constexpr uchar_t kDirectorySeparatorChars[] = { kDirectorySeparatorChar };
                static constexpr uchar_t kInvalidFileNameChars[] = { '\"', '<', '>', '|', ':', '*', '?', '\\', '/', '\0', '\x1', '\x2', '\x3', '\x4', '\x5', '\x6', '\a', '\b', '\t', '\n', '\v', '\f', '\r', '\xE', '\xF', '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F' };
                static constexpr uchar_t kInvalidPathChars[] = { '|', '\0', '\x1', '\x2', '\x3', '\x4', '\x5', '\x6', '\a', '\b', '\t', '\n', '\v', '\f', '\r', '\xE', '\xF', '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F' };
#elif defined(__APPLE__)
                static constexpr uchar_t kAltPathSeparatorChar = '/';
                static constexpr uchar_t kDirectorySeparatorChar = '/';
                static constexpr uchar_t kPathSeparatorChar = ';';
                static constexpr uchar_t kVolumeSeparatorChar = ':';

                static constexpr uchar_t kDirectorySeparatorChars[] = { kDirectorySeparatorChar };
                static constexpr uchar_t kInvalidFileNameChars[] = { '\"', '<', '>', '|', ':', '*', '?', '\\', '/', '\0', '\x1', '\x2', '\x3', '\x4', '\x5', '\x6', '\a', '\b', '\t', '\n', '\v', '\f', '\r', '\xE', '\xF', '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F' };
                static constexpr uchar_t kInvalidPathChars[] = { '|', '\0', '\x1', '\x2', '\x3', '\x4', '\x5', '\x6', '\a', '\b', '\t', '\n', '\v', '\f', '\r', '\xE', '\xF', '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1A', '\x1B', '\x1C', '\x1D', '\x1E', '\x1F' };
#else
#error "unknow system"
#endif

                /// <summary>
                /// 获取路径的根部分。请注意，根路径可以是绝对路径（即完全限定路径）或相对路径。绝对根路径是从驱动器根目录到特定目录的完全限定路径。
                /// 相对根路径指定驱动器，但其完全限定路径是针对当前目录解析的。常见结果如下：
                ///   \mydir\test.txt -> \
                ///   C:\mydir\test.txt -> C:\
                ///   \\?\C:\mydir\test.txt -> \\?\C:\
                ///   \\?\Volume{GUID}\mydir\test.txt -> \\?\Volume{GUID}\
                ///   \\?\UNC\ComputerName\SharedFolder\mydir\test.txt -> \\?\UNC\ComputerName\SharedFolder\
                ///   \\ComputerName\SharedFolder\mydir\test.txt -> \\ComputerName\SharedFolder\
                /// </summary>
                /// <param name="_sPath">要提取根路径的路径字符串视图。</param>
                /// <returns>路径字符串的根路径部分，类型为 uStringView。</returns>
                static uStringView __YYAPI GetPathRoot(const uStringView& _sPath) noexcept;

                static uString __YYAPI GetPathRoot(uString _szPath);

                static bool __YYAPI IsPathRooted(const uStringView& _sPath) noexcept;

                static bool __YYAPI RemoveFileSpec(uStringView& _sPath) noexcept;

                static bool __YYAPI RemoveFileSpec(uString& _szPath);

                static uStringView __YYAPI GetFileName(const uStringView& _sPath) noexcept;

                static uString __YYAPI GetFileName(uString _szPath);

                static uStringView __YYAPI GetExtension(const uStringView& _sPath) noexcept;

                static uString __YYAPI GetExtension(uString _szPath);

                static uStringView __YYAPI GetFileNameWithoutExtension(const uStringView& _sPath) noexcept;

                static uString __YYAPI GetFileNameWithoutExtension(uString _szPath);

                static uStringView __YYAPI GetDirectoryPath(uStringView _sPath) noexcept;

                static uString __YYAPI GetDirectoryPath(uString _szPath);

                /// <summary>
                /// 判断路径是否为完全限定路径（或UNC路径）。
                /// </summary>
                /// <param name="_sPath">路径。</param>
                /// <returns>如果路径是完全限定路径，则返回 true；否则返回 false。</returns>
                static bool __YYAPI IsPathFullyQualified(const uStringView& _sPath) noexcept;

                static bool __YYAPI Append(uString& _szPath, const uStringView& _sPathAppend);

                template<typename... Args>
                static bool __YYAPI Append(uString& _szPath, const uStringView& _sFirstPathAppend, Args&&... _OtherPaths)
                {
                    Append(_szPath, _sFirstPathAppend);
                    return Append(_szPath, std::forward<Args>(_OtherPaths)...);
                }

                template<typename... Args>
                static uString __YYAPI Combine(uString _szPath1, Args&&... _OtherPaths)
                {
                    Append(_szPath1, std::forward<Args>(_OtherPaths)...);
                    return _szPath1;
                }

                static uString __YYAPI GetFullPath(const uStringView& _sPath);

                static uString __YYAPI GetCurrentDirectory();
            };
        }
    }

    using namespace YY::Base::IO;
}

#pragma pop_macro("GetCurrentDirectory")

#pragma pack(pop)
