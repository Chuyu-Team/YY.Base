#pragma once

#include <YY/Base/YY.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Containers
        {
            template<typename _Type>
            class Span
            {
            private:
                _Type* pData = nullptr;
                size_t cData = 0;

            public:
                using ValueType = _Type;

                explicit constexpr Span() = default;

                explicit constexpr Span(_In_reads_opt_(_cData) ValueType* _pData, _In_ size_t _cData)
                    : pData(_pData)
                    , cData(_pData ? _cData : 0)
                {
                }

                template<size_t _uArrayCount>
                explicit constexpr Span(ValueType(&_Array)[_uArrayCount])
                    : pData(_Array)
                    , cData(_uArrayCount)
                {
                }

                ValueType* __YYAPI GetData()
                {
                    return pData;
                }

                const ValueType* __YYAPI GetData() const noexcept
                {
                    return pData;
                }

                size_t __YYAPI GetSize() const
                {
                    return cData;
                }

                size_t __YYAPI GetLength() const
                {
                    return cData;
                }

                bool __YYAPI IsEmpty() const
                {
                    return cData == 0;
                }

                /// <summary>
                /// 从当前 Span 中移除指定数量的起始和末尾元素，并返回修改后的 Span 引用。
                /// </summary>
                /// <param name="_uRemoveStart">要从起始位置移除的元素数量。</param>
                /// <param name="_uRemoveEnd">要从末尾移除的元素数量。</param>
                /// <returns>返回修改后的当前 Span 的引用。</returns>
                Span& __YYAPI Slice(size_t _uRemoveStart, size_t _uRemoveEnd = 0u)
                {
                    if (_uRemoveStart + _uRemoveEnd >= cData)
                    {
                        cData = 0;
                    }
                    else
                    {
                        pData += _uRemoveStart;
                        cData -= _uRemoveStart;
                        cData -= _uRemoveEnd;
                    }

                    return *this;
                }

                /// <summary>
                /// 返回从指定起始索引开始的子区间（子 Span）。
                /// </summary>
                /// <param name="_uStartIndex">子区间的起始索引。</param>
                /// <returns>一个新的 Span，表示从起始索引到原区间末尾的子区间。如果起始索引超出范围，则返回长度为 0 的 Span。</returns>
                Span __YYAPI Subspan(size_t _uStartIndex) const noexcept
                {
                    if (_uStartIndex < GetLength())
                    {
                        return Span(GetData() + _uStartIndex, GetLength() - _uStartIndex);
                    }
                    else
                    {
                        return Span(GetData() + GetLength(), 0);
                    }
                }

                /// <summary>
                /// 返回一个表示当前 Span 子区间的新 Span 对象。
                /// </summary>
                /// <param name="_uStartIndex">子区间的起始索引。</param>
                /// <param name="_cchLength">子区间的长度。</param>
                /// <returns>一个新的 Span，表示从指定起始索引开始、指定长度的子区间。如果起始索引超出范围，则返回长度为 0 的 Span。</returns>
                Span __YYAPI Subspan(size_t _uStartIndex, size_t _cchLength) const noexcept
                {
                    if (_uStartIndex < GetLength())
                    {
                        return Span(GetData() + _uStartIndex, (std::min)(GetLength() - _uStartIndex, _cchLength));
                    }
                    else
                    {
                        return Span(GetData() + GetLength(), 0);
                    }
                }

                /// <summary>
                /// 查找指定值在容器中的索引位置。
                /// </summary>
                /// <param name="_oValue">要查找的值。</param>
                /// <returns>如果找到，返回值在容器中的索引；如果未找到，返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(const ValueType& _oValue) const
                {
                    const auto _sEnd = GetData() + GetLength();
                    for (auto _sItem = GetData(); _sItem != _sEnd; ++_sItem)
                    {
                        if (*_sItem == _oValue)
                        {
                            return _sItem - GetData();
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找指定子区间在当前区间中的起始索引。
                /// </summary>
                /// <param name="_oValue">要查找的子区间（Span）。</param>
                /// <returns>如果找到，返回子区间的起始索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(Span _oValue) const
                {
                    if (_oValue.IsEmpty())
                        return kuInvalidIndex;

                    if (GetLength() < _oValue.GetLength())
                        return kuInvalidIndex;

                    const auto _pEnd = pData + GetLength() - _oValue.GetLength() + 1;
                    for (auto _pItem = pData; _pItem != _pEnd; ++_pItem)
                    {
                        if (Span(_pItem, _oValue.GetLength()) == _oValue)
                        {
                            return _pItem - GetData();
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找当前对象中第一个匹配 _oAnyOfValue 中任意元素的位置。
                /// </summary>
                /// <param name="_oAnyOfValue">包含要查找的任意元素的 Span 对象。</param>
                /// <returns>返回第一个匹配元素的索引，如果未找到则返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOfAny(Span _oAnyOfValue) const
                {
                    if (_oAnyOfValue.IsEmpty())
                        return kuInvalidIndex;

                    const auto _sEnd = pData + GetLength();
                    for (auto _sItem = pData; _sItem != _sEnd; ++_sItem)
                    {
                        if (_oAnyOfValue.IndexOf(*_sItem) != kuInvalidIndex)
                        {
                            return _sItem - pData;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找指定值在容器中的最后一次出现的索引。
                /// </summary>
                /// <param name="_oValue">要查找的值。</param>
                /// <returns>如果找到，返回值最后一次出现的索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOf(const ValueType& _oValue) const
                {
                    auto _pEnd = pData + GetLength();
                    while (_pEnd != pData)
                    {
                        --_pEnd;
                        if (*_pEnd == _oValue)
                        {
                            return _pEnd - pData;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找指定子序列在当前序列中最后一次出现的位置。
                /// </summary>
                /// <param name="_oValue">要查找的子序列。</param>
                /// <returns>如果找到，返回子序列最后一次出现的起始索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOf(Span _oValue) const
                {
                    if (_oValue.IsEmpty())
                        return kuInvalidIndex;

                    if (GetLength() < _oValue.GetLength())
                        return kuInvalidIndex;

                    auto _pEnd = pData + GetLength() - _oValue.GetLength() + 1;
                    for (; _pEnd != pData; )
                    {
                        --_pEnd;

                        if (Span(_pEnd, _oValue.GetLength()) == _oValue)
                        {
                            return _pEnd - pData;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找并返回当前对象中最后一个匹配 _sAnyOfValue 中任意元素的位置索引。
                /// </summary>
                /// <param name="_sAnyOfValue">包含要查找的元素集合的 Span 对象。</param>
                /// <returns>如果找到匹配元素，返回其在当前对象中的索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOfAny(Span _sAnyOfValue) const
                {
                    if (_sAnyOfValue.IsEmpty())
                        return kuInvalidIndex;

                    auto _pEnd = pData + GetLength();
                    while (_pEnd != pData)
                    {
                        --_pEnd;
                        if (_sAnyOfValue.IndexOf(*_pEnd) != kuInvalidIndex)
                        {
                            return _pEnd - pData;
                        }
                    }

                    return kuInvalidIndex;
                }

                /// <summary>
                /// 查找指定值在容器中的索引位置。
                /// </summary>
                /// <param name="_oValue">要查找的值。</param>
                /// <param name="_uIndex">搜索的起始索引位置。</param>
                /// <param name="_uCount">要搜索的元素数量，默认为包含整个Span。</param>
                /// <returns>如果找到，返回值的索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(const ValueType& _oValue, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if(IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Subspan(_uIndex, _uCount).IndexOf(_oValue);
                    if(_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找指定子序列在当前序列中的索引位置。
                /// </summary>
                /// <param name="_oValue">要查找的子序列。</param>
                /// <param name="_uIndex">开始查找的起始索引位置。</param>
                /// <param name="_uCount">要查找的元素数量，默认为包含整个Span。</param>
                /// <returns>如果找到，则返回子序列的起始索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI IndexOf(Span _oValue, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (_oValue.IsEmpty())
                        return kuInvalidIndex;

                    if (IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Subspan(_uIndex, _uCount).IndexOf(_oValue);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找指定字符集合中任意字符在当前范围内首次出现的位置。
                /// </summary>
                /// <param name="_sAnyOfChar">要查找的字符集合。</param>
                /// <param name="_uIndex">搜索的起始索引位置。</param>
                /// <param name="_uCount">要搜索的字符数量，默认为包含整个Span。</param>
                /// <returns>如果找到，返回首次出现的索引；否则返回无效索引（kuInvalidIndex）。</returns>
                size_t __YYAPI IndexOfAny(Span _sAnyOfChar, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (IsEmpty())
                        return kuInvalidIndex;

                    if (_sAnyOfChar.IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Subspan(_uIndex, _uCount).IndexOfAny(_sAnyOfChar);
                    if(_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找指定值在子区间中的最后一次出现的索引。
                /// </summary>
                /// <param name="_oValue">要查找的值。</param>
                /// <param name="_uIndex">子区间的起始索引。</param>
                /// <param name="_uCount">要搜索的元素数量，默认为包含整个Span。</param>
                /// <returns>如果找到，返回值在子区间中的最后一次出现的全局索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOf(const ValueType& _oValue, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Subspan(_uIndex, _uCount).LastIndexOf(_oValue);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找指定子序列在当前序列中最后一次出现的位置。
                /// </summary>
                /// <param name="_oValue">要查找的子序列。</param>
                /// <param name="_uIndex">搜索的起始索引位置。</param>
                /// <param name="_uCount">要搜索的元素数量，默认为包含整个Span。</param>
                /// <returns>如果找到，则返回子序列最后一次出现的索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOf(Span _oValue, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (IsEmpty())
                        return kuInvalidIndex;

                    if (_oValue.IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Subspan(_uIndex, _uCount).LastIndexOf(_oValue);
                    if (_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 查找指定范围内最后一个匹配任意给定值的元素的索引。
                /// </summary>
                /// <param name="_sAnyOfValue">包含要匹配的任意值的 Span。</param>
                /// <param name="_uIndex">搜索的起始索引位置。</param>
                /// <param name="_uCount">要搜索的元素数量，默认为包含整个Span。</param>
                /// <returns>如果找到，返回最后一个匹配元素的索引；否则返回 kuInvalidIndex。</returns>
                size_t __YYAPI LastIndexOfAny(Span _sAnyOfValue, size_t _uIndex, size_t _uCount = (std::numeric_limits<size_t>::max)()) const
                {
                    if (_sAnyOfValue.IsEmpty())
                        return kuInvalidIndex;

                    const auto _uSubIndex = Subspan(_uIndex, _uCount).LastIndexOfAny(_sAnyOfValue);
                    if(_uSubIndex == kuInvalidIndex)
                        return kuInvalidIndex;

                    return _uIndex + _uSubIndex;
                }

                /// <summary>
                /// 移除当前 Span 开头处所有在指定值集合中的字符。
                /// </summary>
                /// <param name="_oTrimValues">包含要从开头移除的字符的 Span。</param>
                /// <returns>移除指定字符后的当前 Span 的引用。</returns>
                Span& __YYAPI TrimStart(Span _oTrimValues)
                {
                    if (_oTrimValues.IsEmpty())
                        return *this;

                    while (cData)
                    {
                        if (_oTrimValues.IndexOf(pData[0]) != kuInvalidIndex)
                        {
                            ++pData;
                            --cData;
                        }
                        else
                        {
                            break;
                        }
                    }

                    return *this;
                }

                /// <summary>
                /// 移除当前 Span 末尾所有在指定集合中的字符。
                /// </summary>
                /// <param name="_oTrimValues">包含要从末尾移除的字符的 Span。</param>
                /// <returns>移除指定字符后的当前 Span 的引用。</returns>
                Span& __YYAPI TrimEnd(Span _oTrimValues)
                {
                    if (_oTrimValues.IsEmpty())
                        return *this;

                    while (cData)
                    {
                        if (_oTrimValues.IndexOf(pData[cData - 1]) != kuInvalidIndex)
                        {
                            --cData;
                        }
                        else
                        {
                            break;
                        }
                    }
                    return *this;
                }

                /// <summary>
                /// 移除当前 Span 对象两端的指定字符。
                /// </summary>
                /// <param name="_sTrimChars">要从当前 Span 两端移除的字符集合。</param>
                /// <returns>移除指定字符后的当前 Span 的引用。</returns>
                Span& __YYAPI Trim(Span _sTrimChars)
                {
                    TrimStart(_sTrimChars);
                    TrimEnd(_sTrimChars);
                    return *this;
                }

                /// <summary>
                /// 根据指定分隔符，从当前索引开始分割，并返回第一个子区段。
                /// </summary>
                /// <param name="_chSplit">用于分割的分隔符值。</param>
                /// <param name="_uSplitIndex">开始查找分隔符的索引，默认为0。</param>
                /// <param name="_puNextSplitIndex">可选参数，用于输出下一个分割索引的位置指针。</param>
                /// <returns>返回从起始索引到分隔符（不包括分隔符）的子区段。如果未找到分隔符，则返回从起始索引到末尾的子区段。</returns>
                Span __YYAPI SplitAndTakeFirst(_In_ const ValueType& _chSplit, _In_ size_t _uSplitIndex = 0, _Out_opt_ size_t* _puNextSplitIndex = nullptr) const
                {
                    const auto _uIndex = IndexOf(_chSplit, _uSplitIndex);
                    if (_uIndex == kuInvalidIndex)
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = GetLength();
                        }

                        return Subspan(_uSplitIndex);
                    }
                    else
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = _uIndex + 1;
                        }

                        return Subspan(_uSplitIndex, _uIndex - _uSplitIndex);
                    }
                }

                /// <summary>
                /// 根据指定分隔符，从当前分割索引处分割并返回第一个子区段。
                /// </summary>
                /// <param name="_sSplit">用于分割的分隔符区段。</param>
                /// <param name="_uSplitIndex">开始查找分割符的位置（默认为0）。</param>
                /// <param name="_puNextSplitIndex">可选参数，用于接收下一个分割索引的位置指针。</param>
                /// <returns>分割后第一个子区段（Span类型）。</returns>
                Span __YYAPI SplitAndTakeFirst(_In_ Span _sSplit, _In_ size_t _uSplitIndex = 0, _Out_opt_ size_t* _puNextSplitIndex = nullptr) const
                {
                    const auto _uIndex = IndexOf(_sSplit, _uSplitIndex);
                    if (_uIndex == kuInvalidIndex)
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = GetLength();
                        }

                        return Subspan(_uSplitIndex);
                    }
                    else
                    {
                        if (_puNextSplitIndex)
                        {
                            *_puNextSplitIndex = _uIndex + _sSplit.GetLength();
                        }

                        return Subspan(_uSplitIndex, _uIndex - _uSplitIndex);
                    }
                }

                /// <summary>
                /// 判断容器是否包含指定的值。
                /// </summary>
                /// <param name="_oValue">要查找的值。</param>
                /// <returns>如果容器包含该值，则返回 true；否则返回 false。</returns>
                inline bool __YYAPI Contains(const ValueType& _oValue) const
                {
                    return IndexOf(_oValue) != kuInvalidIndex;
                }

                /// <summary>
                /// 判断当前对象是否包含指定的 Span。
                /// </summary>
                /// <param name="_oValue">要查找的 Span 对象。</param>
                /// <returns>如果包含指定的 Span，则返回 true；否则返回 false。</returns>
                inline bool __YYAPI Contains(Span _oValue) const
                {
                    return IndexOf(_oValue) != kuInvalidIndex;
                }

                /// <summary>
                /// 判断当前对象是否包含 _oAnyOfValue 中的任意元素。
                /// </summary>
                /// <param name="_oAnyOfValue">要查找的元素集合，类型为 Span。</param>
                /// <returns>如果包含集合中的任意元素，则返回 true；否则返回 false。</returns>
                inline bool __YYAPI ContainsAny(Span _oAnyOfValue) const
                {
                    return IndexOfAny(_oAnyOfValue) != kuInvalidIndex;
                }

                /// <summary>
                /// 判断当前对象是否以指定的 Span 开头。
                /// </summary>
                /// <param name="_oValue">要检查的前缀 Span。</param>
                /// <returns>如果当前对象以 _oValue 开头，则返回 true；否则返回 false。</returns>
                bool __YYAPI StartsWith(Span _oValue) const
                {
                    if (GetLength() < _oValue.GetLength())
                        return false;

                    return Subspan(0, _oValue.GetLength()) == _oValue;
                }

                /// <summary>
                /// 判断当前对象的结尾是否与指定的 Span 匹配。
                /// </summary>
                /// <param name="_oValue">要进行结尾比较的 Span 对象。</param>
                /// <returns>如果当前对象的结尾与 _oValue 完全相同，则返回 true；否则返回 false。</returns>
                bool __YYAPI EndsWith(Span _oValue) const
                {
                    if (GetLength() < _oValue.GetLength())
                        return false;

                    return Subspan(GetLength() - _oValue.GetLength(), _oValue.GetLength()) == _oValue;
                }

                _Type& __YYAPI operator[](_In_ size_t _uIndex)
                {
                    return pData[_uIndex];
                }

                const _Type& __YYAPI operator[](_In_ size_t _uIndex) const
                {
                    return pData[_uIndex];
                }

                bool __YYAPI operator==(_In_ const Span& _Other) const
                {
                    if (GetLength() != _Other.GetLength())
                        return false;

                    for (size_t i = 0; i < GetLength(); ++i)
                    {
                        if (pData[i] != _Other.pData[i])
                            return false;
                    }

                    return true;
                }

                bool __YYAPI operator!=(_In_ const Span& _Other) const
                {
                    if (GetLength() != _Other.GetLength())
                        return true;

                    for (size_t i = 0; i < GetLength(); ++i)
                    {
                        if (pData[i] == _Other.pData[i])
                            return true;
                    }

                    return false;
                }

                _Type* __YYAPI begin() noexcept
                {
                    return pData;
                }

                _Type* __YYAPI end() noexcept
                {
                    return pData + GetSize();
                }

                const _Type* __YYAPI begin() const noexcept
                {
                    return pData;
                }

                const _Type* __YYAPI end() const noexcept
                {
                    return pData + GetSize();
                }

                _Type* __YYAPI _Unchecked_begin() noexcept
                {
                    return pData;
                }

                _Type* __YYAPI _Unchecked_end() noexcept
                {
                    return pData + GetSize();
                }

                const _Type* __YYAPI _Unchecked_begin() const noexcept
                {
                    return pData;
                }

                const _Type* __YYAPI _Unchecked_end() const noexcept
                {
                    return pData + GetSize();
                }
            };
        } // namespace Containers
    } // namespace Base

    using namespace YY::Base::Containers;
} // namespace YY

#pragma pack(pop)
