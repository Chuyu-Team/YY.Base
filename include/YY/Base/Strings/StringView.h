#pragma once
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <algorithm>

#include <YY/Base/YY.h>
#include <YY/Base/Encoding.h>
#include <YY/Base/tchar.h>
#include <YY/Base/ErrorCode.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Strings
        {
            template<typename _char_t, Encoding _eEncoding>
            class StringView
            {
            public:
                using char_t = _char_t;
                using Char = _char_t;
                using ValueType = _char_t;

            private:
                // 不一定指向 0结尾！！
                _Field_size_(cchString) const char_t* sString;
                size_t cchString;
                constexpr static Encoding eEncoding = _eEncoding;

            public:
                explicit constexpr StringView() noexcept
                    : sString(nullptr)
                    , cchString(0)
                {
                }

                explicit constexpr StringView(_In_reads_opt_(_cchSrc) const char_t* _szSrc, _In_ size_t _cchSrc) noexcept
                    : sString(_szSrc)
                    , cchString(_szSrc ? _cchSrc : 0)
                {
                }

                explicit constexpr StringView(_In_reads_opt_(_cchSrc) const char_t* _szSrc, _In_ size_t _cchSrc, _In_ Encoding _eEncoding) noexcept
                    : sString(_szSrc)
                    , cchString(_szSrc ? _cchSrc : 0)
                {
                    assert(_eEncoding == eEncoding);
                }

                StringView(_In_opt_z_ const char_t* _szSrc)
                    : sString(_szSrc)
                    , cchString(GetStringLength(_szSrc))
                {
                }

                template<size_t _uArrayCount>
                static constexpr StringView __YYAPI FromStaticString(const char_t(&_szSrc)[_uArrayCount]) noexcept
                {
                    return StringView(_szSrc, _uArrayCount - 1);
                }

                HRESULT __YYAPI SetString(_In_reads_opt_(_cchSrc) const char_t* _szSrc, _In_ size_t _cchSrc)
                {
                    if (_szSrc == nullptr && _cchSrc)
                        return E_INVALIDARG;

                    sString = _szSrc;
                    cchString = _cchSrc;
                    return S_OK;
                }

                constexpr static Encoding __YYAPI GetEncoding()
                {
                    return eEncoding;
                }

                inline size_t __YYAPI GetSize() const
                {
                    return cchString;
                }

                inline size_t __YYAPI GetLength() const
                {
                    return cchString;
                }

                inline bool __YYAPI IsEmpty() const
                {
                    return cchString == 0;
                }

                inline _Ret_notnull_ _Post_readable_size_(cchString)
                const char_t* __YYAPI GetConstString() const
                {
                    return sString;
                }

                inline _Ret_notnull_ _Post_readable_size_(cchString)
                const char_t* __YYAPI GetData() const
                {
                    return sString;
                }

                void __YYAPI Clear()
                {
                    sString = nullptr;
                    cchString = 0;
                }

                inline char_t __YYAPI operator[](_In_ size_t _uIndex) const
                {
                    assert(_uIndex < GetSize());

                    return sString[_uIndex];
                }

                inline const char_t* __YYAPI begin() const
                {
                    return this->GetConstString();
                }

                inline const char_t* __YYAPI end() const
                {
                    return this->GetConstString() + this->GetSize();
                }

                bool __YYAPI operator==(StringView _sOther) const
                {
                    if (cchString != _sOther.cchString)
                        return false;

                    return memcmp(sString, _sOther.sString, cchString * sizeof(sString[0])) == 0;
                }

                bool __YYAPI operator!=(StringView _sOther) const
                {
                    if (cchString != _sOther.cchString)
                        return true;

                    return memcmp(sString, _sOther.sString, cchString * sizeof(sString[0])) != 0;
                }

                bool __YYAPI operator==(_In_z_ const char_t* _Other) const
                {
                    return Compare(_Other) == 0;
                }

                bool __YYAPI operator!=(_In_z_ const char_t* _Other) const
                {
                    return Compare(_Other) != 0;
                }

                int __YYAPI Compare(_In_z_ const char_t* _szOther) const
                {
                    if (_szOther == nullptr)
                    {
                        return cchString ? 1 : 0;
                    }

                    size_t _uIndex = 0;
                    for (; _uIndex != GetSize(); ++_uIndex)
                    {
                        auto _result = sString[_uIndex] - _szOther[_uIndex];
                        if (_result != 0)
                            return _result;
                    }

                    return char_t('\0') - _szOther[_uIndex];
                }

                int __YYAPI Compare(_In_ StringView _sOther) const
                {
                    const auto _uMinSize = (std::min)(GetSize(), _sOther.GetSize());

                    uint_t _uIndex = 0;
                    for (; _uIndex != _uMinSize; ++_uIndex)
                    {
                        auto _result = sString[_uIndex] - _sOther[_uIndex];
                        if (_result != 0)
                            return _result;
                    }

                    if (GetSize() > _sOther.GetSize())
                    {
                        return 1;
                    }
                    else if (GetSize() < _sOther.GetSize())
                    {
                        return -1;
                    }
                    else
                    {
                        return 0;
                    }
                }

                int32_t __YYAPI CompareI(_In_z_ const char_t* _szOther) const
                {
                    if (_szOther == nullptr)
                    {
                        return cchString ? 1 : 0;
                    }

                    uint_t _uIndex = 0;
                    for (; _uIndex != GetSize(); ++_uIndex)
                    {
                        auto _result = CharUpperAsASCII(sString[_uIndex]) - CharUpperAsASCII(_szOther[_uIndex]);
                        if (_result != 0)
                            return _result;
                    }

                    return char_t('\0') - _szOther[_uIndex];
                }

                int32_t __YYAPI CompareI(_In_ StringView _sOther) const
                {
                    const auto _uMinSize = (std::min)(GetSize(), _sOther.GetSize());

                    uint_t _uIndex = 0;
                    for (; _uIndex != _uMinSize; ++_uIndex)
                    {
                        auto _result = CharUpperAsASCII(sString[_uIndex]) - CharUpperAsASCII(_sOther[_uIndex]);
                        if (_result != 0)
                            return _result;
                    }

                    if (GetSize() > _sOther.GetSize())
                    {
                        return 1;
                    }
                    else if (GetSize() < _sOther.GetSize())
                    {
                        return -1;
                    }
                    else
                    {
                        return 0;
                    }
                }

                /// <summary>
                /// 查找指定字符在此实例中的首次出现位置。
                /// </summary>
                /// <param name="_ch">要查找的字符。</param>
                /// <returns>如果找到，返回字符在字符串中的索引；如果未找到，返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(char_t _ch) const
                {
                    const auto _sEnd = sString + GetLength();
                    for (auto _sItem = sString; _sItem != _sEnd; ++_sItem)
                    {
                        if (*_sItem == _ch)
                        {
                            return _sItem - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找子字符串在当前字符串中的首次出现位置。
                /// </summary>
                /// <param name="_sStr">要查找的子字符串视图。</param>
                /// <returns>如果找到，返回子字符串首次出现的索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(StringView _sStr) const
                {
                    if (_sStr.IsEmpty())
                        return kuInvalidIndex;

                    if (GetLength() < _sStr.GetLength())
                        return kuInvalidIndex;

                    const auto _cbStr = _sStr.GetLength() * sizeof(_sStr[0]);
                    const auto _sEnd = sString + GetLength() - _sStr.GetLength() + 1;
                    for (auto _sItem = sString; _sItem != _sEnd; ++_sItem)
                    {
                        if (memcmp(_sItem, _sStr.GetConstString(), _cbStr) == 0)
                        {
                            return _sItem - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找字符串中任意指定字符集首次出现的位置。注意：字符集中的任意字符匹配即可。
                /// </summary>
                /// <param name="_sAnyOfChar">包含要查找的字符集的 StringView 对象。</param>
                /// <returns>返回第一个匹配字符的索引，如果未找到则返回无效索引（kuInvalidIndex）。</returns>
                size_t __YYAPI IndexOfAny(StringView _sAnyOfChar) const
                {
                    if (_sAnyOfChar.IsEmpty())
                        return kuInvalidIndex;

                    const auto _sEnd = sString + GetLength();
                    for (auto _sItem = sString; _sItem != _sEnd; ++_sItem)
                    {
                        if (_sAnyOfChar.IndexOf(*_sItem) != kuInvalidIndex)
                        {
                            return _sItem - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找字符在字符串中最后一次出现的位置。
                /// </summary>
                /// <param name="_ch">要查找的字符。</param>
                /// <returns>如果找到，返回字符最后一次出现的索引；如果未找到，返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOf(char_t _ch) const
                {
                    auto _sEnd = sString + GetLength();
                    while (_sEnd != sString)
                    {
                        --_sEnd;
                        if (*_sEnd == _ch)
                        {
                            return _sEnd - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找指定子字符串在当前字符串中最后一次出现的位置。
                /// </summary>
                /// <param name="_sStr">要查找的子字符串。</param>
                /// <returns>如果找到，返回子字符串最后一次出现的起始索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOf(StringView _sStr) const
                {
                    if (_sStr.IsEmpty())
                        return kuInvalidIndex;

                    if (GetLength() < _sStr.GetLength())
                        return kuInvalidIndex;

                    const auto _cbStr = _sStr.GetLength() * sizeof(_sStr[0]);
                    auto _sEnd = sString + GetLength() - _sStr.GetLength() + 1;
                    for (; _sEnd != sString; )
                    {
                        --_sEnd;

                        if (memcmp(_sEnd, _sStr.GetConstString(), _cbStr) == 0)
                        {
                            return _sEnd - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找字符串中任意指定字符最后一次出现的位置。注意：字符集中的任意字符匹配即可。
                /// </summary>
                /// <param name="_sAnyOfChar">包含要查找的任意字符集。</param>
                /// <returns>返回最后一次出现的字符的索引，如果未找到则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOfAny(StringView _sAnyOfChar) const
                {
                    if (_sAnyOfChar.IsEmpty())
                        return kuInvalidIndex;

                    auto _sEnd = sString + GetLength();
                    while (_sEnd != sString)
                    {
                        --_sEnd;
                        if (_sAnyOfChar.IndexOf(*_sEnd) != kuInvalidIndex)
                        {
                            return _sEnd - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找指定字符在此实例中的首次出现位置。 搜索从指定字符位置开始，并检查指定数量的字符位置。
                /// </summary>
                /// <param name="_ch">要查找的字符。</param>
                /// <param name="_uIndex">搜索的起始索引位置。</param>
                /// <param name="_uCount">搜索的最大字符数。</param>
                /// <returns>如果找到字符，则返回其索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(char_t _ch, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if(IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).IndexOf(_ch);
                    if(_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找指定字符串在此实例中的首次出现位置。 搜索从指定字符位置开始，并检查指定数量的字符位置。
                /// </summary>
                /// <param name="_sStr">要查找的字符串。</param>
                /// <param name="_uIndex">搜索的起始索引，默认为 0。</param>
                /// <param name="_uCount">搜索的最大范围。</param>
                /// <returns>如果找到子串，则返回其在字符串中的索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(StringView _sStr, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (_sStr.IsEmpty())
                        return kuInvalidIndex;

                    if (IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).IndexOf(_sStr);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找字符串中任意指定字符集首次出现的位置。注意：字符集中的任意字符匹配即可。
                /// </summary>
                /// <param name="_sAnyOfChar">包含要查找的任意字符的字符串视图。</param>
                /// <param name="_uIndex">搜索的起始索引。</param>
                /// <param name="_uCount">要搜索的字符数，默认为字符串的最大长度。</param>
                /// <returns>返回第一个匹配字符的索引，如果未找到则返回无效索引（kuInvalidIndex）。</returns>
                size_t __YYAPI IndexOfAny(StringView _sAnyOfChar, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (IsEmpty())
                        return kuInvalidIndex;

                    if (_sAnyOfChar.IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).IndexOfAny(_sAnyOfChar);
                    if(_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找指定字符在字符串中最后一次出现的位置。
                /// </summary>
                /// <param name="_ch">要查找的字符。</param>
                /// <param name="_uIndex">开始向左查找的起始索引，默认为字符串末尾。</param>
                /// <param name="_uCount">要查找的最大字符数，默认为整个字符串。</param>
                /// <returns>如果找到，返回字符最后一次出现的索引；否则返回无效索引（kuInvalidIndex）。</returns>
                size_t __YYAPI LastIndexOf(char_t _ch, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).LastIndexOf(_ch);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找指定子字符串在当前字符串中最后一次出现的位置。
                /// </summary>
                /// <param name="_sStr">要查找的子字符串。</param>
                /// <param name="_uIndex">搜索的起始索引位置。</param>
                /// <param name="_uCount">要搜索的字符数。</param>
                /// <returns>如果找到，则返回子字符串最后一次出现的索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOf(StringView _sStr, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (IsEmpty())
                        return kuInvalidIndex;

                    if (_sStr.IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).LastIndexOf(_sStr);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找字符串中指定字符集最后一次出现的位置。注意：字符集中的任意字符匹配即可。
                /// </summary>
                /// <param name="_sAnyOfChar">要查找的字符集。</param>
                /// <param name="_uIndex">开始向前查找的起始索引。</param>
                /// <param name="_uCount">要查找的字符数。</param>
                /// <returns>返回最后一次出现指定字符集中的任意字符的索引；如果未找到，则返回无效索引（kuInvalidIndex）。</returns>
                size_t __YYAPI LastIndexOfAny(StringView _sAnyOfChar, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if(_sAnyOfChar.IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).LastIndexOfAny(_sAnyOfChar);
                    if(_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                inline size_t __YYAPI Find(char_t _ch, size_t _uIndex = 0) const
                {
                    return IndexOf(_ch, _uIndex);
                }

                inline size_t __YYAPI Find(StringView _sStr, size_t _uIndex = 0) const
                {
                    return IndexOf(_sStr, _uIndex);
                }

                StringView& __YYAPI Slice(size_t _uRemoveStart, size_t _uRemoveEnd = 0u) noexcept
                {
                    if (_uRemoveStart + _uRemoveEnd >= cchString)
                    {
                        cchString = 0;
                    }
                    else
                    {
                        sString += _uRemoveStart;
                        cchString -= _uRemoveStart;
                        cchString -= _uRemoveEnd;
                    }

                    return *this;
                }

                StringView __YYAPI Substring(size_t _uStartIndex) const noexcept
                {
                    if (_uStartIndex < cchString)
                    {
                        return StringView(sString + _uStartIndex, cchString - _uStartIndex);
                    }
                    else
                    {
                        return StringView(sString + cchString, 0);
                    }
                }

                StringView __YYAPI Substring(size_t _uStartIndex, size_t _cchLength) const noexcept
                {
                    if (_uStartIndex < cchString)
                    {
                        return StringView(sString + _uStartIndex, (std::min)(cchString - _uStartIndex, _cchLength));
                    }
                    else
                    {
                        return StringView(sString + cchString, 0);
                    }
                }

                StringView& __YYAPI TrimStart(StringView _sTrimChars)
                {
                    if(_sTrimChars.IsEmpty())
                        return *this;

                    while (cchString)
                    {
                        if (_sTrimChars.IndexOf(sString[0]) != kuInvalidIndex)
                        {
                            ++sString;
                            --cchString;
                        }
                        else
                        {
                            break;
                        }
                    }

                    return *this;
                }

                StringView& __YYAPI TrimEnd(StringView _sTrimChars)
                {
                    if(_sTrimChars.IsEmpty())
                        return *this;

                    while (cchString)
                    {
                        if (_sTrimChars.IndexOf(sString[cchString - 1]) != kuInvalidIndex)
                        {
                            --cchString;
                        }
                        else
                        {
                            break;
                        }
                    }
                    return *this;
                }

                StringView& __YYAPI Trim(StringView _sTrimChars)
                {
                    TrimStart(_sTrimChars);
                    TrimEnd(_sTrimChars);
                    return *this;
                }

                /// <summary>
                /// 按指定分隔符拆分字符串，并返回第一个子串。
                /// </summary>
                /// <param name="_chSplit">用于分割字符串的字符。</param>
                /// <param name="_uSplitIndex">开始查找分隔符的位置（默认为0）。</param>
                /// <param name="_puNextSplitIndex">可选参数，用于接收下一个分隔符的位置索引。</param>
                /// <returns>分割后第一个子串的 StringView。如果未找到分隔符，则返回整个字符串。</returns>
                StringView __YYAPI SplitAndTakeFirst(_In_ char_t _chSplit, _In_ size_t _uSplitIndex = 0, _Out_opt_ size_t* _puNextSplitIndex = nullptr) const
                {
                    const auto _uIndex = IndexOf(_chSplit, _uSplitIndex);
                    if (_uIndex == kuInvalidIndex)
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = GetLength();
                        }

                        return Substring(_uSplitIndex);
                    }
                    else
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = _uIndex + 1;
                        }

                        return Substring(_uSplitIndex, _uIndex - _uSplitIndex);
                    }
                }

                /// <summary>
                /// 根据指定分隔符，从字符串中分割并返回第一个子串。
                /// </summary>
                /// <param name="_sSplit">用于分割的字符串分隔符。</param>
                /// <param name="_uSplitIndex">开始查找分隔符的位置（默认为0）。</param>
                /// <param name="_puNextSplitIndex">可选参数，用于接收下一个分割点的索引。</param>
                /// <returns>分割后第一个子串。如果未找到分隔符，则返回原字符串。</returns>
                StringView __YYAPI SplitAndTakeFirst(_In_ StringView _sSplit, _In_ size_t _uSplitIndex = 0, _Out_opt_ size_t* _puNextSplitIndex = nullptr) const
                {
                    const auto _uIndex = IndexOf(_sSplit, _uSplitIndex);
                    if (_uIndex == kuInvalidIndex)
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = GetLength();
                        }

                        return Substring(_uSplitIndex);
                    }
                    else
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = _uIndex + _sSplit.GetLength();
                        }

                        return Substring(_uSplitIndex, _uIndex - _uSplitIndex);
                    }
                }

                /// <summary>
                /// 将字符串按指定分隔符拆分，并返回第一个子串。
                /// </summary>
                /// <param name="_chSplit">用于拆分字符串的分隔字符。</param>
                /// <param name="_psRemaining">（可选）指向接收剩余字符串的指针。如果提供，将被设置为分隔符后的剩余部分。</param>
                /// <returns>分隔符前的第一个子串。</returns>
                StringView __YYAPI SplitAndTakeFirst(_In_ char_t _chSplit, _Out_opt_ StringView* _psRemaining = nullptr) const
                {
                    size_t _uNextSplitIndex;
                    auto _sResult = SplitAndTakeFirst(_chSplit, 0, &_uNextSplitIndex);
                    if (_psRemaining)
                    {
                        *_psRemaining = Substring(_uNextSplitIndex);
                    }

                    return _sResult;
                }

                /// <summary>
                /// 分割字符串并返回第一个子串。
                /// </summary>
                /// <param name="_sSplit">用于分割的字符串视图。</param>
                /// <param name="_psRemaining">（可选）指向接收剩余字符串视图的指针。</param>
                /// <returns>分割后得到的第一个子串的字符串视图。</returns>
                StringView __YYAPI SplitAndTakeFirst(_In_ StringView _sSplit, _Out_opt_ StringView* _psRemaining = nullptr) const
                {
                    size_t _uNextSplitIndex;
                    auto _sResult = SplitAndTakeFirst(_sSplit, 0, &_uNextSplitIndex);
                    if (_psRemaining)
                    {
                        *_psRemaining = Substring(_uNextSplitIndex);
                    }
                    return _sResult;
                }

                /// <summary>
                /// 判断字符是否包含在对象中。
                /// </summary>
                /// <param name="_ch">要查找的字符。</param>
                /// <returns>如果对象包含该字符，则返回 true；否则返回 false。</returns>
                inline bool __YYAPI Contains(char_t _ch) const
                {
                    return IndexOf(_ch) != kuInvalidIndex;
                }

                /// <summary>
                /// 判断字符串视图是否包含指定的子字符串。
                /// </summary>
                /// <param name="_szStr">要查找的子字符串视图。</param>
                /// <returns>如果包含指定的子字符串，则返回 true；否则返回 false。</returns>
                inline bool __YYAPI Contains(StringView _szStr) const
                {
                    return IndexOf(_szStr) != kuInvalidIndex;
                }

                /// <summary>
                /// 判断当前字符串视图是否包含指定的任意字符。
                /// </summary>
                /// <param name="_oAnyOfValue">要查找的字符集合，类型为 StringView。</param>
                /// <returns>如果当前字符串视图包含集合中的任意字符，则返回 true；否则返回 false。</returns>
                inline bool __YYAPI ContainsAny(StringView _oAnyOfValue) const
                {
                    return IndexOfAny(_oAnyOfValue) != kuInvalidIndex;
                }

                /// <summary>
                /// 判断当前字符串是否以指定字符串视图开头。
                /// </summary>
                /// <param name="_sStr">要检查的字符串视图。</param>
                /// <returns>如果当前字符串以 _sStr 开头，则返回 true；否则返回 false。</returns>
                bool __YYAPI StartsWith(StringView _sStr) const
                {
                    if (GetLength() < _sStr.GetLength())
                        return false;

                    return Substring(0, _sStr.GetLength()) == _sStr;
                }

                /// <summary>
                /// 判断当前字符串是否以指定字符串结尾。
                /// </summary>
                /// <param name="_sStr">要检查的结尾字符串视图。</param>
                /// <returns>如果当前字符串以 _sStr 结尾，则返回 true；否则返回 false。</returns>
                bool __YYAPI EndsWith(StringView _sStr) const
                {
                    if (GetLength() < _sStr.GetLength())
                        return false;

                    return Substring(GetLength() - _sStr.GetLength(), _sStr.GetLength()) == _sStr;
                }
            };

            template<>
            class StringView<YY::Base::achar_t, Encoding::ANSI>
            {
            public:
                using char_t = YY::Base::achar_t;
                using Char = YY::Base::achar_t;
                using ValueType = YY::Base::achar_t;

            private:
                // 不一定指向 0结尾！！
                _Field_size_(cchString) const char_t* sString;
                size_t cchString;
                Encoding eEncoding;

            public:
                explicit constexpr StringView() noexcept
                    : sString(nullptr)
                    , cchString(0)
                    , eEncoding(Encoding::ANSI)
                {
                }

                explicit constexpr StringView(_In_reads_opt_(_cchSrc) const char_t* _szSrc, _In_ size_t _cchSrc, _In_ Encoding _eEncoding = Encoding::ANSI) noexcept
                    : sString(_szSrc)
                    , cchString(_cchSrc)
                    , eEncoding(_eEncoding)
                {
                }

                StringView(_In_opt_z_ const char_t* _szSrc, _In_ Encoding _eEncoding = Encoding::ANSI)
                    : sString(_szSrc)
                    , cchString(YY::Base::GetStringLength(_szSrc))
                    , eEncoding(_eEncoding)
                {
                }

                template<size_t _uArrayCount>
                static constexpr StringView __YYAPI FromStaticString(const char_t (&_szSrc)[_uArrayCount], _In_ Encoding _eEncoding = Encoding::ANSI) noexcept
                {
                    return StringView(_szSrc, _uArrayCount - 1, _eEncoding);
                }

                inline Encoding __YYAPI GetEncoding() const
                {
                    return eEncoding;
                }

                inline size_t __YYAPI GetSize() const
                {
                    return cchString;
                }

                inline size_t __YYAPI GetLength() const
                {
                    return cchString;
                }

                inline bool __YYAPI IsEmpty() const
                {
                    return cchString == 0;
                }

                inline _Ret_notnull_ _Post_readable_size_(cchString)
                const char_t* __YYAPI GetConstString() const
                {
                    return sString;
                }

                inline _Ret_notnull_ _Post_readable_size_(cchString)
                const char_t* __YYAPI GetData() const
                {
                    return sString;
                }

                void __YYAPI Clear()
                {
                    sString = nullptr;
                    cchString = 0;
                }

                inline char_t __YYAPI operator[](_In_ size_t _uIndex) const
                {
                    assert(_uIndex < GetSize());

                    return sString[_uIndex];
                }

                inline const char_t* __YYAPI begin() const
                {
                    return this->GetConstString();
                }

                inline const char_t* __YYAPI end() const
                {
                    return this->GetConstString() + this->GetSize();
                }

                bool __YYAPI operator==(StringView _sOther) const
                {
                    if (cchString != _sOther.cchString)
                        return false;

                    return memcmp(sString, _sOther.sString, cchString * sizeof(sString[0])) == 0;
                }

                bool __YYAPI operator!=(StringView _sOther) const
                {
                    if (cchString != _sOther.cchString)
                        return true;

                    return memcmp(sString, _sOther.sString, cchString * sizeof(sString[0])) != 0;
                }

                bool __YYAPI operator==(_In_z_ const char_t* _szOther) const
                {
                    return Compare(_szOther) == 0;
                }

                bool __YYAPI operator!=(_In_z_ const char_t* _szOther) const
                {
                    return Compare(_szOther) != 0;
                }

                int __YYAPI Compare(_In_z_ const char_t* _szOther) const
                {
                    if (_szOther == nullptr)
                    {
                        return cchString ? 1 : 0;
                    }

                    size_t _uIndex = 0;
                    for (; _uIndex != GetSize(); ++_uIndex)
                    {
                        auto _result = sString[_uIndex] - _szOther[_uIndex];
                        if (_result != 0)
                            return _result;
                    }

                    return char_t('\0') - _szOther[_uIndex];
                }

                int __YYAPI Compare(_In_ StringView _sOther) const
                {
                    const auto _uMinSize = (std::min)(GetSize(), _sOther.GetSize());

                    uint_t _uIndex = 0;
                    for (; _uIndex != _uMinSize; ++_uIndex)
                    {
                        auto _result = sString[_uIndex] - _sOther[_uIndex];
                        if (_result != 0)
                            return _result;
                    }

                    if (GetSize() > _sOther.GetSize())
                    {
                        return 1;
                    }
                    else if (GetSize() < _sOther.GetSize())
                    {
                        return -1;
                    }
                    else
                    {
                        return 0;
                    }
                }

                int32_t __YYAPI CompareI(_In_z_ const char_t* _szOther) const
                {
                    if (_szOther == nullptr)
                    {
                        return cchString ? 1 : 0;
                    }

                    uint_t _uIndex = 0;
                    for (; _uIndex != GetSize(); ++_uIndex)
                    {
                        auto _result = CharUpperAsASCII(sString[_uIndex]) - CharUpperAsASCII(_szOther[_uIndex]);
                        if (_result != 0)
                            return _result;
                    }

                    return char_t('\0') - _szOther[_uIndex];
                }

                int32_t __YYAPI CompareI(_In_ StringView _sOther) const
                {
                    const auto _uMinSize = (std::min)(GetSize(), _sOther.GetSize());

                    uint_t _uIndex = 0;
                    for (; _uIndex != _uMinSize; ++_uIndex)
                    {
                        auto _result = CharUpperAsASCII(sString[_uIndex]) - CharUpperAsASCII(_sOther[_uIndex]);
                        if (_result != 0)
                            return _result;
                    }

                    if (GetSize() > _sOther.GetSize())
                    {
                        return 1;
                    }
                    else if (GetSize() < _sOther.GetSize())
                    {
                        return -1;
                    }
                    else
                    {
                        return 0;
                    }
                }

                /// <summary>
                /// 查找指定字符在此实例中的首次出现位置。
                /// </summary>
                /// <param name="_ch">要查找的字符。</param>
                /// <returns>如果找到，返回字符在字符串中的索引；如果未找到，返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(char_t _ch) const
                {
                    const auto _sEnd = sString + GetLength();
                    for (auto _sItem = sString; _sItem != _sEnd; ++_sItem)
                    {
                        if (*_sItem == _ch)
                        {
                            return _sItem - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找子字符串在当前字符串中的首次出现位置。
                /// </summary>
                /// <param name="_sStr">要查找的子字符串视图。</param>
                /// <returns>如果找到，返回子字符串首次出现的索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(StringView _sStr) const
                {
                    if (_sStr.IsEmpty())
                        return kuInvalidIndex;

                    if (GetLength() < _sStr.GetLength())
                        return kuInvalidIndex;

                    const auto _cbStr = _sStr.GetLength() * sizeof(_sStr[0]);
                    const auto _sEnd = sString + GetLength() - _sStr.GetLength() + 1;
                    for (auto _sItem = sString; _sItem != _sEnd; ++_sItem)
                    {
                        if (memcmp(_sItem, _sStr.GetConstString(), _cbStr) == 0)
                        {
                            return _sItem - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找字符串中任意指定字符集首次出现的位置。注意：字符集中的任意字符匹配即可。
                /// </summary>
                /// <param name="_sAnyOfChar">包含要查找的字符集的 StringView 对象。</param>
                /// <returns>返回第一个匹配字符的索引，如果未找到则返回无效索引（kuInvalidIndex）。</returns>
                size_t __YYAPI IndexOfAny(StringView _sAnyOfChar) const
                {
                    if (_sAnyOfChar.IsEmpty())
                        return kuInvalidIndex;

                    const auto _sEnd = sString + GetLength();
                    for (auto _sItem = sString; _sItem != _sEnd; ++_sItem)
                    {
                        if (_sAnyOfChar.IndexOf(*_sItem) != kuInvalidIndex)
                        {
                            return _sItem - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找字符在字符串中最后一次出现的位置。
                /// </summary>
                /// <param name="_ch">要查找的字符。</param>
                /// <returns>如果找到，返回字符最后一次出现的索引；如果未找到，返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOf(char_t _ch) const
                {
                    auto _sEnd = sString + GetLength();
                    while (_sEnd != sString)
                    {
                        --_sEnd;
                        if (*_sEnd == _ch)
                        {
                            return _sEnd - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找指定子字符串在当前字符串中最后一次出现的位置。
                /// </summary>
                /// <param name="_sStr">要查找的子字符串。</param>
                /// <returns>如果找到，返回子字符串最后一次出现的起始索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOf(StringView _sStr) const
                {
                    if (_sStr.IsEmpty())
                        return kuInvalidIndex;

                    if (GetLength() < _sStr.GetLength())
                        return kuInvalidIndex;

                    const auto _cbStr = _sStr.GetLength() * sizeof(_sStr[0]);
                    auto _sEnd = sString + GetLength() - _sStr.GetLength() + 1;
                    for (; _sEnd != sString; )
                    {
                        --_sEnd;

                        if (memcmp(_sEnd, _sStr.GetConstString(), _cbStr) == 0)
                        {
                            return _sEnd - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找字符串中任意指定字符最后一次出现的位置。注意：字符集中的任意字符匹配即可。
                /// </summary>
                /// <param name="_sAnyOfChar">包含要查找的任意字符集。</param>
                /// <returns>返回最后一次出现的字符的索引，如果未找到则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOfAny(StringView _sAnyOfChar) const
                {
                    if (_sAnyOfChar.IsEmpty())
                        return kuInvalidIndex;

                    auto _sEnd = sString + GetLength();
                    while (_sEnd != sString)
                    {
                        --_sEnd;
                        if (_sAnyOfChar.IndexOf(*_sEnd) != kuInvalidIndex)
                        {
                            return _sEnd - sString;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找指定字符在此实例中的首次出现位置。 搜索从指定字符位置开始，并检查指定数量的字符位置。
                /// </summary>
                /// <param name="_ch">要查找的字符。</param>
                /// <param name="_uIndex">搜索的起始索引位置。</param>
                /// <param name="_uCount">搜索的最大字符数。</param>
                /// <returns>如果找到字符，则返回其索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(char_t _ch, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).IndexOf(_ch);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找指定字符串在此实例中的首次出现位置。 搜索从指定字符位置开始，并检查指定数量的字符位置。
                /// </summary>
                /// <param name="_sStr">要查找的字符串。</param>
                /// <param name="_uIndex">搜索的起始索引，默认为 0。</param>
                /// <param name="_uCount">搜索的最大范围。</param>
                /// <returns>如果找到子串，则返回其在字符串中的索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(StringView _sStr, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (_sStr.IsEmpty())
                        return kuInvalidIndex;

                    if (IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).IndexOf(_sStr);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找字符串中任意指定字符集首次出现的位置。注意：字符集中的任意字符匹配即可。
                /// </summary>
                /// <param name="_sAnyOfChar">包含要查找的任意字符的字符串视图。</param>
                /// <param name="_uIndex">搜索的起始索引。</param>
                /// <param name="_uCount">要搜索的字符数，默认为字符串的最大长度。</param>
                /// <returns>返回第一个匹配字符的索引，如果未找到则返回无效索引（kuInvalidIndex）。</returns>
                size_t __YYAPI IndexOfAny(StringView _sAnyOfChar, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (IsEmpty())
                        return kuInvalidIndex;

                    if (_sAnyOfChar.IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).IndexOfAny(_sAnyOfChar);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找指定字符在字符串中最后一次出现的位置。
                /// </summary>
                /// <param name="_ch">要查找的字符。</param>
                /// <param name="_uIndex">开始向左查找的起始索引，默认为字符串末尾。</param>
                /// <param name="_uCount">要查找的最大字符数，默认为整个字符串。</param>
                /// <returns>如果找到，返回字符最后一次出现的索引；否则返回无效索引（kuInvalidIndex）。</returns>
                size_t __YYAPI LastIndexOf(char_t _ch, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).LastIndexOf(_ch);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找指定子字符串在当前字符串中最后一次出现的位置。
                /// </summary>
                /// <param name="_sStr">要查找的子字符串。</param>
                /// <param name="_uIndex">搜索的起始索引位置。</param>
                /// <param name="_uCount">要搜索的字符数。</param>
                /// <returns>如果找到，则返回子字符串最后一次出现的索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOf(StringView _sStr, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (IsEmpty())
                        return kuInvalidIndex;

                    if (_sStr.IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).LastIndexOf(_sStr);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找字符串中指定字符集最后一次出现的位置。注意：字符集中的任意字符匹配即可。
                /// </summary>
                /// <param name="_sAnyOfChar">要查找的字符集。</param>
                /// <param name="_uIndex">开始向前查找的起始索引。</param>
                /// <param name="_uCount">要查找的字符数。</param>
                /// <returns>返回最后一次出现指定字符集中的任意字符的索引；如果未找到，则返回无效索引（kuInvalidIndex）。</returns>
                size_t __YYAPI LastIndexOfAny(StringView _sAnyOfChar, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (_sAnyOfChar.IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Substring(_uIndex, _uCount).LastIndexOfAny(_sAnyOfChar);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                inline size_t __YYAPI Find(char_t _ch, size_t _uIndex = 0) const
                {
                    return IndexOf(_ch, _uIndex);
                }

                inline size_t __YYAPI Find(StringView _sStr, size_t _uIndex = 0) const
                {
                    return IndexOf(_sStr, _uIndex);
                }

                StringView& __YYAPI Slice(size_t _uRemoveStart, size_t _uRemoveEnd = 0u) noexcept
                {
                    if (_uRemoveStart + _uRemoveEnd >= cchString)
                    {
                        cchString = 0;
                    }
                    else
                    {
                        sString += _uRemoveStart;
                        cchString -= _uRemoveStart;
                        cchString -= _uRemoveEnd;
                    }

                    return *this;
                }

                StringView __YYAPI Substring(size_t _uStartIndex) const noexcept
                {
                    if (_uStartIndex < cchString)
                    {
                        return StringView(sString + _uStartIndex, cchString - _uStartIndex, GetEncoding());
                    }
                    else
                    {
                        return StringView(sString + cchString, 0, GetEncoding());
                    }
                }

                StringView __YYAPI Substring(size_t _uStartIndex, size_t _cchLength) const noexcept
                {
                    if (_uStartIndex < cchString)
                    {
                        return StringView(sString + _uStartIndex, (std::min)(cchString - _uStartIndex, _cchLength), GetEncoding());
                    }
                    else
                    {
                        return StringView(sString + cchString, 0, GetEncoding());
                    }
                }

                StringView& __YYAPI TrimStart(StringView _sTrimChars)
                {
                    if (_sTrimChars.IsEmpty())
                        return *this;

                    while (cchString)
                    {
                        if (_sTrimChars.IndexOf(sString[0]) != kuInvalidIndex)
                        {
                            ++sString;
                            --cchString;
                        }
                        else
                        {
                            break;
                        }
                    }

                    return *this;
                }

                StringView& __YYAPI TrimEnd(StringView _sTrimChars)
                {
                    if (_sTrimChars.IsEmpty())
                        return *this;

                    while (cchString)
                    {
                        if (_sTrimChars.IndexOf(sString[cchString - 1]) != kuInvalidIndex)
                        {
                            --cchString;
                        }
                        else
                        {
                            break;
                        }
                    }
                    return *this;
                }

                StringView& __YYAPI Trim(StringView _sTrimChars)
                {
                    TrimStart(_sTrimChars);
                    TrimEnd(_sTrimChars);
                    return *this;
                }

                /// <summary>
                /// 按指定分隔符拆分字符串，并返回第一个子串。
                /// </summary>
                /// <param name="_chSplit">用于分割字符串的字符。</param>
                /// <param name="_uSplitIndex">开始查找分隔符的位置（默认为0）。</param>
                /// <param name="_puNextSplitIndex">可选参数，用于接收下一个分隔符的位置索引。</param>
                /// <returns>分割后第一个子串的 StringView。如果未找到分隔符，则返回整个字符串。</returns>
                StringView __YYAPI SplitAndTakeFirst(_In_ char_t _chSplit, _In_ size_t _uSplitIndex = 0, _Out_opt_ size_t* _puNextSplitIndex = nullptr) const
                {
                    const auto _uIndex = IndexOf(_chSplit, _uSplitIndex);
                    if (_uIndex == kuInvalidIndex)
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = GetLength();
                        }

                        return Substring(_uSplitIndex);
                    }
                    else
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = _uIndex + 1;
                        }

                        return Substring(_uSplitIndex, _uIndex - _uSplitIndex);
                    }
                }

                /// <summary>
                /// 根据指定分隔符，从字符串中分割并返回第一个子串。
                /// </summary>
                /// <param name="_sSplit">用于分割的字符串分隔符。</param>
                /// <param name="_uSplitIndex">开始查找分隔符的位置（默认为0）。</param>
                /// <param name="_puNextSplitIndex">可选参数，用于接收下一个分割点的索引。</param>
                /// <returns>分割后第一个子串。如果未找到分隔符，则返回原字符串。</returns>
                StringView __YYAPI SplitAndTakeFirst(_In_ StringView _sSplit, _In_ size_t _uSplitIndex = 0, _Out_opt_ size_t* _puNextSplitIndex = nullptr) const
                {
                    const auto _uIndex = IndexOf(_sSplit, _uSplitIndex);
                    if (_uIndex == kuInvalidIndex)
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = GetLength();
                        }

                        return Substring(_uSplitIndex);
                    }
                    else
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = _uIndex + _sSplit.GetLength();
                        }

                        return Substring(_uSplitIndex, _uIndex - _uSplitIndex);
                    }
                }

                /// <summary>
                /// 将字符串按指定分隔符拆分，并返回第一个子串。
                /// </summary>
                /// <param name="_chSplit">用于拆分字符串的分隔字符。</param>
                /// <param name="_psRemaining">（可选）指向接收剩余字符串的指针。如果提供，将被设置为分隔符后的剩余部分。</param>
                /// <returns>分隔符前的第一个子串。</returns>
                StringView __YYAPI SplitAndTakeFirst(_In_ char_t _chSplit, _Out_opt_ StringView* _psRemaining = nullptr) const
                {
                    size_t _uNextSplitIndex;
                    auto _sResult = SplitAndTakeFirst(_chSplit, 0, &_uNextSplitIndex);
                    if (_psRemaining)
                    {
                        *_psRemaining = Substring(_uNextSplitIndex);
                    }

                    return _sResult;
                }

                /// <summary>
                /// 分割字符串并返回第一个子串。
                /// </summary>
                /// <param name="_sSplit">用于分割的字符串视图。</param>
                /// <param name="_psRemaining">（可选）指向接收剩余字符串视图的指针。</param>
                /// <returns>分割后得到的第一个子串的字符串视图。</returns>
                StringView __YYAPI SplitAndTakeFirst(_In_ StringView _sSplit, _Out_opt_ StringView* _psRemaining = nullptr) const
                {
                    size_t _uNextSplitIndex;
                    auto _sResult = SplitAndTakeFirst(_sSplit, 0, &_uNextSplitIndex);
                    if (_psRemaining)
                    {
                        *_psRemaining = Substring(_uNextSplitIndex);
                    }
                    return _sResult;
                }

                /// <summary>
                /// 判断字符是否包含在对象中。
                /// </summary>
                /// <param name="_ch">要查找的字符。</param>
                /// <returns>如果字符存在则返回 true，否则返回 false。</returns>
                inline bool __YYAPI Contains(char_t _ch) const
                {
                    return IndexOf(_ch) != kuInvalidIndex;
                }

                /// <summary>
                /// 判断字符串视图是否包含指定的子字符串。
                /// </summary>
                /// <param name="_szStr">要查找的子字符串视图。</param>
                /// <returns>如果包含指定子字符串，则返回 true；否则返回 false。</returns>
                inline bool __YYAPI Contains(StringView _szStr) const
                {
                    return IndexOf(_szStr) != kuInvalidIndex;
                }

                /// <summary>
                /// 判断字符串是否包含指定字符集中的任意字符。
                /// </summary>
                /// <param name="_sAnyOfChar">要查找的字符集，类型为 StringView。</param>
                /// <returns>如果字符串包含字符集中的任意字符，则返回 true；否则返回 false。</returns>
                inline bool __YYAPI ContainsAny(StringView _sAnyOfChar) const
                {
                    return IndexOfAny(_sAnyOfChar) != kuInvalidIndex;
                }

                /// <summary>
                /// 判断当前字符串是否以指定字符串视图开头。
                /// </summary>
                /// <param name="_sStr">要检查的字符串视图。</param>
                /// <returns>如果当前字符串以 _sStr 开头，则返回 true；否则返回 false。</returns>
                bool __YYAPI StartsWith(StringView _sStr) const
                {
                    if (GetLength() < _sStr.GetLength())
                        return false;

                    return Substring(0, _sStr.GetLength()) == _sStr;
                }

                /// <summary>
                /// 判断当前字符串是否以指定字符串结尾。
                /// </summary>
                /// <param name="_sStr">要检查的结尾字符串视图。</param>
                /// <returns>如果当前字符串以 _sStr 结尾，则返回 true；否则返回 false。</returns>
                bool __YYAPI EndsWith(StringView _sStr) const
                {
                    if (GetLength() < _sStr.GetLength())
                        return false;

                    return Substring(GetLength() - _sStr.GetLength(), _sStr.GetLength()) == _sStr;
                }
            };

            typedef StringView<YY::Base::achar_t, Encoding::ANSI> aStringView;
            typedef StringView<YY::Base::u8char_t, Encoding::UTF8> u8StringView;
            typedef StringView<YY::Base::u16char_t, Encoding::UTF16LE> u16StringLEView;
            typedef StringView<YY::Base::u16char_t, Encoding::UTF16BE> u16StringBEView;
            typedef StringView<YY::Base::u32char_t, Encoding::UTF32LE> u32StringLEView;
            typedef StringView<YY::Base::u32char_t, Encoding::UTF32BE> u32StringBEView;
            typedef StringView<YY::Base::uchar_t, Encoding::UTFN> uStringView;

            typedef StringView<YY::Base::u16char_t, Encoding::UTF16> u16StringView;
            typedef StringView<YY::Base::u32char_t, Encoding::UTF32> u32StringView;

        } // namespace Strings
    } // namespace Base

    using namespace YY::Base::Strings;
} // namespace YY

#pragma pack(pop)
