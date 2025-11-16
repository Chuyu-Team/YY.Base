#include <YY/Base/IO/Path.h>

__YY_IGNORE_INCONSISTENT_ANNOTATION_FOR_FUNCTION()

#ifdef GetCurrentDirectory
#undef GetCurrentDirectory
#endif

namespace YY
{
    namespace Base
    {
        namespace IO
        {
            static constexpr bool __YYAPI IsDirectorySeparatorChar(uchar_t _ch)
            {
                for (auto _chDirectorySeparatorChar : Path::kDirectorySeparatorChars)
                {
                    if (_ch == _chDirectorySeparatorChar)
                        return true;
                }

                return false;
            }

            static constexpr bool __YYAPI IsAlphaChar(uchar_t _ch)
            {
                return (uchar_t('A') <= _ch && _ch <= uchar_t('Z')) || (uchar_t('a') <= _ch && _ch <= uchar_t('z'));
            }

            static constexpr bool __YYAPI IsHexDigit(uchar_t _ch)
            {
                return (_ch >= uchar_t('0') && _ch <= uchar_t('9')) || (_ch >= uchar_t('A') && _ch <= uchar_t('F')) || (_ch >= uchar_t('a') && _ch <= uchar_t('f'));
            }

            static size_t __YYAPI TryParseUncPath(const uStringView& _sPath)
            {
                // Dos路径：\\ComputerName\SharedFolder\mydir\test.txt
                // Dos设备路径：\\?\UNC\ComputerName\SharedFolder\mydir\test.txt

                if (_sPath.GetLength() >= 2
                    && IsDirectorySeparatorChar(_sPath[0])
                    && IsDirectorySeparatorChar(_sPath[1]))
                {
                    if (_sPath.GetLength() >= 3 && (_sPath[2] == uchar_t('?') || _sPath[2] == uchar_t('.')))
                    {
#if defined(WIN32) || defined(_WIN32)
                        if (_sPath.GetLength() >= 8
                            && IsDirectorySeparatorChar(_sPath[3])
                            && (_sPath[4] == uchar_t('U') || _sPath[4] == uchar_t('u'))
                            && (_sPath[5] == uchar_t('N') || _sPath[5] == uchar_t('n'))
                            && (_sPath[6] == uchar_t('C') || _sPath[6] == uchar_t('c'))
                            && IsDirectorySeparatorChar(_sPath[7]))
                        {
                            // Dos设备路径：\\?\UNC\ComputerName\SharedFolder\mydir\test.txt
                            return 8;
                        }
#endif
                    }
                    else
                    {
                        // Dos路径：\\ComputerName\SharedFolder\mydir\test.txt
                        return 2;
                    }
                }

                return 0;
            }

            //static size_t __YYAPI TryParseVolumeGuidPath(const uStringView& _sPath)
            //{
            //    // \\?\Volume{GUID}\mydir\test.txt
            //    if (_sPath.GetLength() >= 48
            //        && IsDirectorySeparatorChar(_sPath[0])
            //        && IsDirectorySeparatorChar(_sPath[1])
            //        && (_sPath[2] == uchar_t('?') || _sPath[2] == uchar_t('.'))
            //        && IsDirectorySeparatorChar(_sPath[3])
            //        && (_sPath[4] == uchar_t('V') || _sPath[4] == uchar_t('v'))
            //        && (_sPath[5] == uchar_t('O') || _sPath[5] == uchar_t('o'))
            //        && (_sPath[6] == uchar_t('L') || _sPath[6] == uchar_t('l'))
            //        && (_sPath[7] == uchar_t('U') || _sPath[7] == uchar_t('u'))
            //        && (_sPath[8] == uchar_t('M') || _sPath[8] == uchar_t('m'))
            //        && (_sPath[9] == uchar_t('E') || _sPath[9] == uchar_t('e'))
            //        && _sPath[10] == uchar_t('{')
            //        && Path::IsHexDigit(_sPath[11])
            //        && IsHexDigit(_sPath[12])
            //        && IsHexDigit(_sPath[13])
            //        && IsHexDigit(_sPath[14])
            //        && IsHexDigit(_sPath[15])
            //        && IsHexDigit(_sPath[16])
            //        && IsHexDigit(_sPath[17])
            //        && IsHexDigit(_sPath[18])
            //        && _sPath[19] == uchar_t('-')
            //        && IsHexDigit(_sPath[20])
            //        && IsHexDigit(_sPath[21])
            //        && IsHexDigit(_sPath[22])
            //        && IsHexDigit(_sPath[23])
            //        && _sPath[24] == uchar_t('-')
            //        && IsHexDigit(_sPath[25])
            //        && IsHexDigit(_sPath[26])
            //        && IsHexDigit(_sPath[27])
            //        && IsHexDigit(_sPath[28])
            //        && _sPath[29] == uchar_t('-')
            //        && IsHexDigit(_sPath[30])
            //        && IsHexDigit(_sPath[31])
            //        && IsHexDigit(_sPath[32])
            //        && IsHexDigit(_sPath[33])
            //        && _sPath[34] == uchar_t('-')
            //        && IsHexDigit(_sPath[35])
            //        && IsHexDigit(_sPath[36])
            //        && IsHexDigit(_sPath[37])
            //        && IsHexDigit(_sPath[38])
            //        && IsHexDigit(_sPath[39])
            //        && IsHexDigit(_sPath[40])
            //        && IsHexDigit(_sPath[41])
            //        && IsHexDigit(_sPath[42])
            //        && IsHexDigit(_sPath[43])
            //        && IsHexDigit(_sPath[44])
            //        && IsHexDigit(_sPath[45])
            //        && IsHexDigit(_sPath[46])
            //        && _sPath[47] == uchar_t('}'))
            //    {
            //        // 48必须是目录结束，所以不能出现目录分隔符之外的字符
            //        if (_sPath.GetLength() == 48 || IsDirectorySeparatorChar(_sPath[48]))
            //        {
            //            return 48;
            //        }
            //    }

            //    return 0;
            //}

#if defined(WIN32) || defined(_WIN32)
            /// <summary>
            /// 尝试解析给定路径字符串中的卷根目录长度。注意：分区是Windows平台的专属概念。
            /// 
            /// 驱动器根目录的绝对文件路径：C:\mydir\test.txt -> 2
            /// Dos设备路径形式：\\?\C:\mydir\test.txt -> 6
            /// Volume{GUID}形式：\\?\Volume{GUID}\mydir\test.txt -> 48
            /// </summary>
            /// <param name="_sPath">要解析的路径字符串视图。</param>
            /// <returns>如果路径是驱动器根目录或卷 GUID 路径，则返回根目录部分的长度；否则返回 0。</returns>
            static size_t __YYAPI TryParseVolumePath(const uStringView& _sPath)
            {
                size_t _cchRoot = 0;
                if (_sPath.GetLength() >= 2
                    && IsAlphaChar(_sPath[0])
                    && _sPath[1] == Path::kVolumeSeparatorChar)
                {
                    // C:\mydir\test.txt
                    _cchRoot = 2;
                }
                else if (_sPath.GetLength() >= 6
                    && IsDirectorySeparatorChar(_sPath[0])
                    && IsDirectorySeparatorChar(_sPath[1])
                    && (_sPath[2] == uchar_t('?') || _sPath[2] == uchar_t('.'))
                    && IsDirectorySeparatorChar(_sPath[3])
                    && IsAlphaChar(_sPath[4])
                    && _sPath[5] == Path::kVolumeSeparatorChar)
                {
                    // \\?\C:\mydir\test.txt
                    _cchRoot = 6;
                }
                else if (_sPath.GetLength() >= 48
                    && IsDirectorySeparatorChar(_sPath[0])
                    && IsDirectorySeparatorChar(_sPath[1])
                    && (_sPath[2] == uchar_t('?') || _sPath[2] == uchar_t('.'))
                    && IsDirectorySeparatorChar(_sPath[3])
                    && (_sPath[4] == uchar_t('V') || _sPath[4] == uchar_t('v'))
                    && (_sPath[5] == uchar_t('O') || _sPath[5] == uchar_t('o'))
                    && (_sPath[6] == uchar_t('L') || _sPath[6] == uchar_t('l'))
                    && (_sPath[7] == uchar_t('U') || _sPath[7] == uchar_t('u'))
                    && (_sPath[8] == uchar_t('M') || _sPath[8] == uchar_t('m'))
                    && (_sPath[9] == uchar_t('E') || _sPath[9] == uchar_t('e'))
                    && _sPath[10] == uchar_t('{')
                    && IsHexDigit(_sPath[11])
                    && IsHexDigit(_sPath[12])
                    && IsHexDigit(_sPath[13])
                    && IsHexDigit(_sPath[14])
                    && IsHexDigit(_sPath[15])
                    && IsHexDigit(_sPath[16])
                    && IsHexDigit(_sPath[17])
                    && IsHexDigit(_sPath[18])
                    && _sPath[19] == uchar_t('-')
                    && IsHexDigit(_sPath[20])
                    && IsHexDigit(_sPath[21])
                    && IsHexDigit(_sPath[22])
                    && IsHexDigit(_sPath[23])
                    && _sPath[24] == uchar_t('-')
                    && IsHexDigit(_sPath[25])
                    && IsHexDigit(_sPath[26])
                    && IsHexDigit(_sPath[27])
                    && IsHexDigit(_sPath[28])
                    && _sPath[29] == uchar_t('-')
                    && IsHexDigit(_sPath[30])
                    && IsHexDigit(_sPath[31])
                    && IsHexDigit(_sPath[32])
                    && IsHexDigit(_sPath[33])
                    && _sPath[34] == uchar_t('-')
                    && IsHexDigit(_sPath[35])
                    && IsHexDigit(_sPath[36])
                    && IsHexDigit(_sPath[37])
                    && IsHexDigit(_sPath[38])
                    && IsHexDigit(_sPath[39])
                    && IsHexDigit(_sPath[40])
                    && IsHexDigit(_sPath[41])
                    && IsHexDigit(_sPath[42])
                    && IsHexDigit(_sPath[43])
                    && IsHexDigit(_sPath[44])
                    && IsHexDigit(_sPath[45])
                    && IsHexDigit(_sPath[46])
                    && _sPath[47] == uchar_t('}')
                    // 注意：48必须是目录结束，所以不能出现目录分隔符之外的字符。额外保证一下
                    && (_sPath.GetLength() == 48 || IsDirectorySeparatorChar(_sPath[48])))
                {
                    // \\?\Volume{GUID}\mydir\test.txt
                    _cchRoot = 48;
                }
                else
                {
                    return 0;
                }

                return _cchRoot;
            }
#endif

            static bool __YYAPI IsSameVolume(uStringView _sLeft, uStringView _sRight)
            {
                if (_sLeft.GetLength() >= 4
                    && IsDirectorySeparatorChar(_sLeft[0])
                    && IsDirectorySeparatorChar(_sLeft[1])
                    && (_sLeft[2] == uchar_t('?') || _sLeft[2] == uchar_t('.'))
                    && IsDirectorySeparatorChar(_sLeft[3]))
                {
                    _sLeft.Slice(4);
                }

                while (_sLeft.GetLength() && IsDirectorySeparatorChar(_sLeft[_sLeft.GetLength() - 1]))
                {
                    _sLeft.Slice(0, 1);
                }

                if (_sRight.GetLength() >= 4
                    && IsDirectorySeparatorChar(_sRight[0])
                    && IsDirectorySeparatorChar(_sRight[1])
                    && (_sRight[2] == uchar_t('?') || _sRight[2] == uchar_t('.'))
                    && IsDirectorySeparatorChar(_sRight[3]))
                {
                    _sRight.Slice(4);
                }

                while (_sRight.GetLength() && IsDirectorySeparatorChar(_sRight[_sRight.GetLength() - 1]))
                {
                    _sRight.Slice(0, 1);
                }

                if (_sLeft.GetLength() != _sRight.GetLength())
                    return false;

                return _sLeft.CompareI(_sRight);
            }

            uStringView __YYAPI Path::GetPathRoot(const uStringView& _sPath) noexcept
            {
                uStringView _sPathRoot;
                do
                {
                    if (_sPath.IsEmpty())
                        break;

                    size_t _cchRoot = 0;
                    if (IsDirectorySeparatorChar(_sPath[0]) && (_sPath.GetLength() == 1 || IsDirectorySeparatorChar(_sPath[1]) == false))
                    {
                        // \mydir\test.txt
                        // 对于Windows平台，它是当前驱动器的相对路径。
                        // 对于Linux或者Max，它是绝对路径。
                        _cchRoot = 1;
                    }
                    else if (_cchRoot = TryParseUncPath(_sPath))
                    {
                        // [UNC前缀]ComputerName\SharedFolder\mydir\test.txt
                        // ComputerName\SharedFolder\ 分别是服务器名称与共享名称，因此再跳过2个 DirectorySeparatorChar。
                        for (size_t _cSkip = 2; _cSkip && _cchRoot < _sPath.GetLength(); ++_cchRoot)
                        {
                            if (IsDirectorySeparatorChar(_sPath[_cchRoot]))
                            {
                                --_cSkip;
                            }
                        }
                    }
#if defined(WIN32) || defined(_WIN32)
                    else if(_cchRoot = TryParseVolumePath(_sPath))
                    {
                        // [Volume前缀]\mydir\test.txt
                        // 如果分区路径，尽可能的多吃掉一个目录分隔符。因为带上分隔符后才是绝对路径
                        if (_sPath.GetLength() > _cchRoot && IsDirectorySeparatorChar(_sPath[_cchRoot]))
                        {
                            ++_cchRoot;
                        }
                    }
#endif
                    else
                    {
                        // 这可能是一个相对路径，没有根目录。
                        // 也可能是一个不合法的路径。
                        break;
                    }

                    _sPathRoot = _sPath.Substring(0, _cchRoot);
                } while (false);

                return _sPathRoot;
            }

            uString __YYAPI Path::GetPathRoot(uString _szPath)
            {
                auto _sPathRoot = GetPathRoot(_szPath.GetStringView());
                if (_sPathRoot.IsEmpty())
                    return uString();

                _szPath.Slice(_sPathRoot.begin() - _szPath.begin(), _szPath.end() - _sPathRoot.end());
                return _szPath;
            }

            bool __YYAPI Path::IsPathRooted(const uStringView& _sPath) noexcept
            {
                return GetPathRoot(_sPath).GetLength() != 0;
            }

            bool __YYAPI Path::RemoveFileSpec(uStringView& _sPath) noexcept
            {
                if(_sPath.IsEmpty())
                    return false;

                auto _sPathRoot = GetPathRoot(_sPath);
                if (_sPathRoot.GetLength() == _sPath.GetLength())
                {
                    // 已经是根目录了
                    return false;
                }

                const auto _uLastDirectorySeparatorCharIndex = _sPath.LastIndexOfAny(uStringView(kDirectorySeparatorChars, std::size(kDirectorySeparatorChars)), _sPathRoot.GetLength());
                if(_uLastDirectorySeparatorCharIndex != kuInvalidIndex)
                {
                    _sPath.Slice(_uLastDirectorySeparatorCharIndex);
                }
                else
                {
                    _sPath = _sPathRoot;
                }

                return true;
            }

            bool __YYAPI Path::RemoveFileSpec(uString& _szPath)
            {
                auto _oPathView = _szPath.GetStringView();
                if (!RemoveFileSpec(_oPathView))
                {
                    return false;
                }

                _szPath.Slice(_oPathView.begin() - _szPath.begin(), _szPath.end() - _oPathView.end());
                return true;
            }

            uStringView __YYAPI Path::GetFileName(const uStringView& _sPath) noexcept
            {
                if(_sPath.IsEmpty())
                    return uStringView();

                auto _sPathRoot = GetPathRoot(_sPath);
                const auto _uLastDirectorySeparatorCharIndex = _sPath.LastIndexOfAny(uStringView(kDirectorySeparatorChars, std::size(kDirectorySeparatorChars)), _sPathRoot.GetLength());
                if (_uLastDirectorySeparatorCharIndex == kuInvalidIndex)
                {
                    return _sPath;
                }

                return _sPath.Substring(_uLastDirectorySeparatorCharIndex + 1);
            }

            uString __YYAPI Path::GetFileName(uString _szPath)
            {
                auto _sFileName = GetFileName(_szPath.GetStringView());
                if (_sFileName.IsEmpty())
                    return uString();

                _szPath.Slice(_sFileName.begin() - _szPath.begin(), _szPath.end() - _sFileName.end());
                return _szPath;
            }

            uStringView __YYAPI Path::GetExtension(const uStringView& _sPath) noexcept
            {
                auto _sFileName = GetFileName(_sPath);
                const auto _uExtensionIndex = _sFileName.LastIndexOf('.');
                if (_uExtensionIndex != kuInvalidIndex)
                {
                    return _sFileName.Substring(_uExtensionIndex);
                }

                return YY::uStringView();
            }

            uString __YYAPI Path::GetExtension(uString _szPath)
            {
                auto _sExtension = GetExtension(_szPath.GetStringView());
                if (_sExtension.IsEmpty())
                    return uString();

                _szPath.Slice(_sExtension.begin() - _szPath.begin(), _szPath.end() - _sExtension.end());
                return _szPath;
            }

            uStringView __YYAPI Path::GetFileNameWithoutExtension(const uStringView& _sPath) noexcept
            {
                auto _sFileName = GetFileName(_sPath);
                const auto _uExtensionIndex = _sFileName.LastIndexOf('.');
                if (_uExtensionIndex != kuInvalidIndex)
                {
                    return _sFileName.Substring(_uExtensionIndex);
                }

                return _sFileName;
            }

            uString __YYAPI Path::GetFileNameWithoutExtension(uString _szPath)
            {
                auto _sFileNameWithoutExtension = GetFileNameWithoutExtension(_szPath.GetStringView());
                if (_sFileNameWithoutExtension.IsEmpty())
                    return uString();

                _szPath.Slice(_sFileNameWithoutExtension.begin() - _szPath.begin(), _szPath.end() - _sFileNameWithoutExtension.end());
                return _szPath;
            }

            uStringView __YYAPI Path::GetDirectoryPath(uStringView _sPath) noexcept
            {
                RemoveFileSpec(_sPath);
                return _sPath;
            }

            uString __YYAPI Path::GetDirectoryPath(uString _szPath)
            {
                RemoveFileSpec(_szPath);
                return _szPath;
            }

            bool __YYAPI Path::IsPathFullyQualified(const uStringView& _sPath) noexcept
            {
                if (TryParseUncPath(_sPath))
                {
                    return true;
                }
#if defined(WIN32) || defined(_WIN32)
                else if (auto _cchRoot = TryParseVolumePath(_sPath))
                {
                    return _cchRoot == _sPath.GetLength() || IsDirectorySeparatorChar(_sPath[_cchRoot]);
                }
#endif
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
                else if (IsDirectorySeparatorChar(_sPath[0]) && (_sPath.GetLength() == 1 || IsDirectorySeparatorChar(_sPath[1]) == false))
                {
                    return true;
                }
#endif
                else
                {
                    return false;
                }
            }

            bool __YYAPI Path::Append(uString& _szPath, const uStringView& _sPathAppend)
            {
                if (_sPathAppend.IsEmpty())
                    return true;

                if (TryParseUncPath(_sPathAppend))
                {
                    _szPath = _sPathAppend;
                    return true;
                }
#if defined(WIN32) || defined(_WIN32)
                else if (TryParseVolumePath(_sPathAppend))
                {
                    _szPath = _sPathAppend;
                    return true;
                }
#endif
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
                else if (IsDirectorySeparatorChar(_sPathAppend[0]) && (_sPathAppend.GetLength() == 1 || IsDirectorySeparatorChar(_sPathAppend[1]) == false))
                {
                    _szPath = _sPathAppend;
                    return true;
                }
#endif
                else
                {
                    size_t _uSkipIndex = 0;
                    while (_uSkipIndex != _sPathAppend.GetLength() && IsDirectorySeparatorChar(_sPathAppend[_uSkipIndex]))
                    {
                        ++_uSkipIndex;
                    }

                    const auto _cchAppend = _sPathAppend.GetLength() - _uSkipIndex;
                    if(_cchAppend == 0)
                    {
                        // 全是目录分隔符
                        return true;
                    }

                    if (!_szPath.LockBuffer(_szPath.GetLength() + 1 + _cchAppend))
                    {
                        return false;
                    }

                    _szPath.UnlockBuffer();

                    if (_szPath.GetLength() && IsDirectorySeparatorChar(_szPath[_szPath.GetLength() - 1]) == false)
                    {
                        _szPath += Path::kDirectorySeparatorChar;
                    }

                    _szPath.AppendString(_sPathAppend.GetConstString() + _uSkipIndex, _cchAppend);
                    return true;
                }
            }

            uString __YYAPI Path::GetFullPath(const uStringView& _sPath)
            {
                if (_sPath.IsEmpty() || IsPathFullyQualified(_sPath))
                {
                    return _sPath;
                }
                
                auto _szCurrentDirectory = Path::GetCurrentDirectory();
                if (_szCurrentDirectory.IsEmpty())
                    return uString();

#if defined(WIN32) || defined(_WIN32)
                if (auto _cchRoot = TryParseVolumePath(_sPath))
                {
                    // C:mydir\test.txt

                    auto _szCurrentDirectoryPathRoot = GetPathRoot(_szCurrentDirectory.GetStringView());
                    if(_szCurrentDirectoryPathRoot.IsEmpty())
                    {
                        return uString();
                    }

                    auto _sVolumeRootPath = _sPath.Substring(0, _cchRoot);
                    if (!IsSameVolume(_szCurrentDirectoryPathRoot, _sVolumeRootPath))
                    {
                        _szCurrentDirectory = _sVolumeRootPath;
                    }

                    Path::Append(_szCurrentDirectory, _sPath.Substring(_cchRoot));
                    return _szCurrentDirectory;
                }
                else if (IsDirectorySeparatorChar(_sPath[0]))
                {
                    // \mydir\test.txt
                    // 此时需要只补全当前驱动器
                    _szCurrentDirectory = GetPathRoot(std::move(_szCurrentDirectory));
                    if (_szCurrentDirectory.IsEmpty())
                    {
                        return uString();
                    }
                }
#endif

                Path::Append(_szCurrentDirectory, _sPath);
                return _szCurrentDirectory;
            }

            uString __YYAPI Path::GetCurrentDirectory()
            {
#if defined(WIN32) || defined(_WIN32)
                wchar_t _szCurrentDirectoryBuffer[512];
                auto _cchCurrentDirectory = ::GetCurrentDirectoryW(_countof(_szCurrentDirectoryBuffer), _szCurrentDirectoryBuffer);
                if (_cchCurrentDirectory == 0)
                {
                    return uString();
                }
                else if (_cchCurrentDirectory <= _countof(_szCurrentDirectoryBuffer))
                {
                    return uString(_szCurrentDirectoryBuffer, _cchCurrentDirectory);
                }
                else
                {
                    uString _szCurrentDirectory;

                    for (;;)
                    {
                        auto _pBuffer = _szCurrentDirectory.LockBuffer(_cchCurrentDirectory);
                        if(!_pBuffer)
                        {
                            return uString();
                        }

                        const auto _uLength = ::GetCurrentDirectoryW(_cchCurrentDirectory, _pBuffer);
                        _szCurrentDirectory.UnlockBuffer(_uLength);
                        if (_uLength == 0)
                        {
                            return uString();
                        }
                        else if (_uLength <= _cchCurrentDirectory)
                        {
                            return _szCurrentDirectory;
                        }
                        else
                        {
                            _cchCurrentDirectory = _uLength;
                        }
                    }
                }
#else
#error "unknow system"
#endif
            }
        }
    }
}
