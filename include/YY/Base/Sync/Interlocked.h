﻿#pragma once

#ifdef _WIN32
#include <intrin.h>
#include <Windows.h>
#include <winnt.h>
#endif

#include <YY/Base/YY.h>

namespace YY
{
    namespace Base
    {
        namespace Sync
        {
            enum class ProducerType
            {
                // 单生产者，不允许多个生产者同时调用
                Single,
                Multi,
            };

            enum class ConsumerType
            {
                // 单消费者，不允许多个消费者同时调用
                Single,
                Multi,
            };

            inline int32_t __YYAPI Increment(volatile int32_t* _pAddend)
            {
#ifdef _MSC_VER
                return (int32_t)_InterlockedIncrement(reinterpret_cast<long volatile*>(_pAddend));
#else
                return __sync_add_and_fetch(_pAddend, 1);
#endif
            }

            inline int32_t __YYAPI Increment(int32_t* _pAddend)
            {
                return (int32_t)Increment(const_cast<volatile int32_t*>(_pAddend));
            }

            inline int64_t __YYAPI Increment(volatile int64_t* _pAddend)
            {
#ifdef _MSC_VER
                return (int64_t)_interlockedincrement64(_pAddend);
#else
                return __sync_add_and_fetch(_pAddend, 1);
#endif
            }

            inline int64_t __YYAPI Increment(int64_t* _pAddend)
            {
                return Increment(const_cast<volatile int64_t*>(_pAddend));
            }

            inline uint32_t __YYAPI Increment(volatile uint32_t* _pAddend)
            {
                return (uint32_t)Increment(reinterpret_cast<volatile int32_t*>(_pAddend));
            }

            inline uint32_t __YYAPI Increment(uint32_t* _pAddend)
            {
                return Increment(const_cast<volatile uint32_t*>(_pAddend));
            }

            inline uint64_t __YYAPI Increment(volatile uint64_t* _pAddend)
            {
                return (uint64_t)Increment(reinterpret_cast<volatile int64_t*>(_pAddend));
            }

            inline uint64_t __YYAPI Increment(uint64_t* _pAddend)
            {
                return Increment(const_cast<volatile uint64_t*>(_pAddend));
            }

            inline int32_t __YYAPI Decrement(volatile int32_t* _pAddend)
            {
#ifdef _MSC_VER
                return (int32_t)_InterlockedDecrement(reinterpret_cast<long volatile*>(_pAddend));
#else
                return __sync_sub_and_fetch(_pAddend, 1);
#endif
            }

            inline int32_t __YYAPI Decrement(int32_t* _pAddend)
            {
                return Decrement(const_cast<volatile int32_t*>(_pAddend));
            }

            inline int64_t __YYAPI Decrement(volatile int64_t* _pAddend)
            {
#ifdef _MSC_VER
                return (int64_t)_interlockeddecrement64(reinterpret_cast<long long volatile*>(_pAddend));
#else
                return __sync_sub_and_fetch(_pAddend, 1);
#endif
            }

            inline int64_t __YYAPI Decrement(int64_t* _pAddend)
            {
                return Decrement(const_cast<volatile int64_t*>(_pAddend));
            }

            inline uint32_t __YYAPI Decrement(volatile uint32_t* _pAddend)
            {
                return (uint32_t)Decrement(reinterpret_cast<volatile int32_t*>(_pAddend));
            }

            inline uint32_t __YYAPI Decrement(uint32_t* _pAddend)
            {
                return Decrement(reinterpret_cast<volatile uint32_t*>(_pAddend));
            }

            inline uint64_t __YYAPI Decrement(volatile uint64_t* _pAddend)
            {
                return (uint64_t)Decrement(reinterpret_cast<volatile int64_t*>(_pAddend));
            }

            inline uint64_t __YYAPI Decrement(uint64_t* _pAddend)
            {
                return Decrement(reinterpret_cast<volatile uint64_t*>(_pAddend));
            }

            inline int32_t __YYAPI Add(volatile int32_t* _pAddend, int32_t _iAdd)
            {
#ifdef _MSC_VER
                return InterlockedAdd((volatile LONG*)_pAddend, _iAdd);
#else
                return __sync_add_and_fetch(_pAddend, _iAdd);
#endif
            }

            inline int32_t __YYAPI Add(int32_t* _pAddend, int32_t _iAdd)
            {
                return Add(const_cast<volatile int32_t*>(_pAddend), _iAdd);
            }

            inline int64_t __YYAPI Add(volatile int64_t* _pAddend, int64_t _iAdd)
            {
#ifdef _MSC_VER
                return InterlockedAdd64((volatile LONG64*)_pAddend, _iAdd);
#else
                return __sync_add_and_fetch(_pAddend, _iAdd);
#endif
            }

            inline int64_t __YYAPI Add(int64_t* _pAddend, int64_t _iAdd)
            {
                return Add(const_cast<volatile int64_t*>(_pAddend), _iAdd);
            }

            inline uint32_t __YYAPI Add(volatile uint32_t* _pAddend, uint32_t _uAdd)
            {
                return (uint32_t)Add((volatile int32_t*)_pAddend, (int32_t)_uAdd);
            }

            inline uint32_t __YYAPI Add(uint32_t* _pAddend, uint32_t _iAdd)
            {
                return Add(const_cast<volatile uint32_t*>(_pAddend), _iAdd);
            }

            inline uint64_t __YYAPI Add(volatile uint64_t* _pAddend, uint64_t _iAdd)
            {
                return (uint64_t)Add((volatile int64_t*)_pAddend, (int64_t)_iAdd);
            }

            inline uint64_t __YYAPI Add(uint64_t* _pAddend, uint64_t _iAdd)
            {
                return Add(const_cast<volatile uint64_t*>(_pAddend), _iAdd);
            }

            inline int32_t __YYAPI Subtract(volatile int32_t* _pAddend, int32_t _uAdd)
            {
                return Add(_pAddend, int32_t(~(uint32_t)_uAdd) + 1);
            }

            inline int32_t __YYAPI Subtract(int32_t* _pAddend, int32_t _uAdd)
            {
                return Add(_pAddend, int32_t(~(uint32_t)_uAdd) + 1);
            }

            inline int64_t __YYAPI Subtract(volatile int64_t* _pAddend, int64_t _uAdd)
            {
                return Add(_pAddend, int64_t(~(uint64_t)_uAdd) + 1);
            }

            inline int64_t __YYAPI Subtract(int64_t* _pAddend, int64_t _uAdd)
            {
                return Add(_pAddend, int64_t(~(uint64_t)_uAdd) + 1);
            }

            inline uint32_t __YYAPI Subtract(volatile uint32_t* _pAddend, uint32_t _uAdd)
            {
                return Add(_pAddend, uint32_t(~(uint32_t)_uAdd) + 1);
            }

            inline uint32_t __YYAPI Subtract(uint32_t* _pAddend, uint32_t _uAdd)
            {
                return Add(_pAddend, uint32_t(~(uint32_t)_uAdd) + 1);
            }

            inline uint64_t __YYAPI Subtract(volatile uint64_t* _pAddend, uint64_t _uAdd)
            {
                return Add(_pAddend, uint64_t(~(uint64_t)_uAdd) + 1);
            }

            inline uint64_t __YYAPI Subtract(uint64_t* _pAddend, uint64_t _uAdd)
            {
                return Add(_pAddend, uint64_t(~(uint64_t)_uAdd) + 1);
            }

            /// <summary>
            /// 使用原子操作，将指定位Bit位设为 1
            /// </summary>
            /// <param name="_pBase"></param>
            /// <param name="_uOffset">指定bit</param>
            /// <returns>该 _uOffset bit位，原始值</returns>
            inline bool __YYAPI BitSet(volatile uint32_t* _pBase, uint32_t _uOffset)
            {
#ifdef _MSC_VER
                return _interlockedbittestandset(reinterpret_cast<long volatile*>(_pBase), _uOffset);
#else
                const uint32_t _fAddMark = 1 << _uOffset;
                auto _uOld = __sync_fetch_and_or(_pBase, _fAddMark);
                return _uOld & _fAddMark;
#endif
            }

            inline bool __YYAPI BitSet(uint32_t* _pBase, uint32_t _uOffset)
            {
                return BitSet(const_cast<volatile uint32_t*>(_pBase), _uOffset);
            }

            inline bool __YYAPI BitSet(volatile uint64_t* _pBase, uint32_t _uOffset)
            {
#ifdef _MSC_VER
#if defined(_AMD64_) || defined(_ARM64_)
                return _interlockedbittestandset64(reinterpret_cast<LONG64 volatile*>(_pBase), _uOffset);
#else // !(defined(_AMD64_) || defined(_ARM64_))
                return _interlockedbittestandset(reinterpret_cast<long volatile*>(_pBase), _uOffset);
#endif // defined(_AMD64_) || defined(_ARM64_)
#else // !_MSC_VER
                const uint32_t _fAddMark = 1 << _uOffset;
                auto _uOld = __sync_fetch_and_or(_pBase, _fAddMark);
                return _uOld & _fAddMark;
#endif
            }

            inline bool __YYAPI BitSet(uint64_t* _pBase, uint32_t _uOffset)
            {
                return BitSet(const_cast<volatile uint64_t*>(_pBase), _uOffset);
            }

            inline bool __YYAPI BitSet(volatile int32_t* _pBase, uint32_t _uOffset)
            {
                return BitSet(reinterpret_cast<volatile uint32_t*>(_pBase), _uOffset);
            }

            inline bool __YYAPI BitSet(int32_t* _pBase, uint32_t _uOffset)
            {
                return BitSet(const_cast<volatile int32_t*>(_pBase), _uOffset);
            }

            inline bool __YYAPI BitSet(volatile int64_t* _pBase, uint32_t _uOffset)
            {
                return BitSet(reinterpret_cast<volatile uint64_t*>(_pBase), _uOffset);
            }

            inline bool __YYAPI BitSet(int64_t* _pBase, uint32_t _uOffset)
            {
                return BitSet(const_cast<volatile int64_t*>(_pBase), _uOffset);
            }

            /// <summary>
            /// 使用原子操作，将指定位Bit位设为 0
            /// </summary>
            /// <param name="_pBase"></param>
            /// <param name="_uOffset">指定bit</param>
            /// <returns>该 _uOffset bit位，原始值</returns>
            inline bool __YYAPI BitReset(volatile uint32_t* _pBase, uint32_t _uOffset)
            {
#ifdef _MSC_VER
                return _interlockedbittestandreset(reinterpret_cast<long volatile*>(_pBase), _uOffset);
#else
                const uint32_t _fRemoveMark = 1 << _uOffset;
                auto _uOld = __sync_fetch_and_and(_pBase, ~_fRemoveMark);
                return _uOld & _fRemoveMark;
#endif
            }

            inline bool __YYAPI BitReset(uint32_t* _pBase, uint32_t _uOffset)
            {
                return BitReset(const_cast<volatile uint32_t*>(_pBase), _uOffset);
            }

            inline bool __YYAPI BitReset(volatile uint64_t* _pBase, uint32_t _uOffset)
            {
#ifdef _MSC_VER
#if defined(_AMD64_) || defined(_ARM64_)
                return _interlockedbittestandreset64(reinterpret_cast<LONG64 volatile*>(_pBase), _uOffset);
#else
                return _interlockedbittestandreset(reinterpret_cast<long volatile*>(_pBase), _uOffset);
#endif

#else
                const uint32_t _fRemoveMark = 1 << _uOffset;
                auto _uOld = __sync_fetch_and_and(_pBase, ~_fRemoveMark);
                return _uOld & _fRemoveMark;
#endif
            }

            inline bool __YYAPI BitReset(uint64_t* _pBase, uint32_t _uOffset)
            {
                return BitReset(const_cast<volatile uint64_t*>(_pBase), _uOffset);
            }

            inline bool __YYAPI BitReset(volatile int32_t* _pBase, uint32_t _uOffset)
            {
                return BitReset(reinterpret_cast<volatile int32_t*>(_pBase), _uOffset);
            }

            inline bool __YYAPI BitReset(int32_t* _pBase, uint32_t _uOffset)
            {
                return BitReset(const_cast<int32_t*>(_pBase), _uOffset);
            }

            inline bool __YYAPI BitReset(volatile int64_t* _pBase, uint32_t _uOffset)
            {
                return BitReset(reinterpret_cast<volatile uint64_t*>(_pBase), _uOffset);
            }

            inline bool __YYAPI BitReset(int64_t* _pBase, uint32_t _uOffset)
            {
                return BitReset(const_cast<volatile int64_t*>(_pBase), _uOffset);
            }

            inline int32_t __YYAPI CompareExchange(volatile int32_t* _pDestination, int32_t _iExchange, int32_t _iComparand)
            {
#ifdef _MSC_VER
                return (int32_t)_InterlockedCompareExchange(reinterpret_cast<long volatile*>(_pDestination), _iExchange, _iComparand);
#else
                return __sync_val_compare_and_swap(_pDestination, _iComparand, _iExchange);
#endif
            }

            inline int32_t __YYAPI CompareExchange(int32_t* _pDestination, int32_t _iExchange, int32_t _iComparand)
            {
                return CompareExchange(const_cast<volatile int32_t*>(_pDestination), _iExchange, _iComparand);
            }

            inline uint32_t __YYAPI CompareExchange(volatile uint32_t* _pDestination, uint32_t _iExchange, uint32_t _iComparand)
            {
                return (uint32_t)CompareExchange(reinterpret_cast<volatile int32_t*>(_pDestination), (int32_t)_iExchange, (int32_t)_iComparand);
            }

            inline uint32_t __YYAPI CompareExchange(uint32_t* _pDestination, uint32_t _iExchange, uint32_t _iComparand)
            {
                return CompareExchange(const_cast<volatile uint32_t*>(_pDestination), _iExchange, _iComparand);
            }

            inline long __YYAPI CompareExchange(volatile long* _pDestination, long _iExchange, long _iComparand)
            {
                static_assert(sizeof(long) == sizeof(int32_t), "");
                return (long)CompareExchange(reinterpret_cast<volatile int32_t*>(_pDestination), (int32_t)_iExchange, (int32_t)_iComparand);
            }

            inline long __YYAPI CompareExchange(long* _pDestination, long _iExchange, long _iComparand)
            {
                return CompareExchange(const_cast<volatile long*>(_pDestination), _iExchange, _iComparand);
            }

            inline int64_t __YYAPI CompareExchange(volatile int64_t* _pDestination, int64_t _iExchange, int64_t _iComparand)
            {
#ifdef _MSC_VER
                return (int64_t)_InterlockedCompareExchange64(reinterpret_cast<long long volatile*>(_pDestination), _iExchange, _iComparand);
#else
                return __sync_val_compare_and_swap(_pDestination, _iComparand, _iExchange);
#endif
            }

            inline int64_t __YYAPI CompareExchange(int64_t* _pDestination, int64_t _iExchange, int64_t _iComparand)
            {
                return CompareExchange(const_cast<volatile int64_t*>(_pDestination), _iExchange, _iComparand);
            }

            inline uint64_t __YYAPI CompareExchange(volatile uint64_t* _pDestination, uint64_t _iExchange, uint64_t _iComparand)
            {
                return (uint64_t)CompareExchange(reinterpret_cast<volatile int64_t*>(_pDestination), (int64_t)_iExchange, (int64_t)_iComparand);
            }

            inline uint64_t __YYAPI CompareExchange( uint64_t* _pDestination, uint64_t _iExchange, uint64_t _iComparand)
            {
                return CompareExchange(const_cast<volatile uint64_t*>(_pDestination), _iExchange, _iComparand);
            }

            template<typename Type1, typename Type2, typename Type3>
            inline Type1* __YYAPI CompareExchangePoint(volatile Type1** _ppDestination, Type2 _pExchange, Type3 _pComparand)
            {
                return (Type1*)CompareExchange(reinterpret_cast<volatile intptr_t*>(_ppDestination), reinterpret_cast<intptr_t>(static_cast<Type1*>(_pExchange)), reinterpret_cast<intptr_t>(static_cast<Type1*>(_pComparand)));
            }

            template<typename Type1, typename Type2, typename Type3>
            inline Type1* __YYAPI CompareExchangePoint(Type1** _ppDestination, Type2 _pExchange, Type3 _pComparand)
            {
                return (Type1*)CompareExchange(reinterpret_cast<intptr_t*>(_ppDestination), reinterpret_cast<intptr_t>(static_cast<Type1*>(_pExchange)), reinterpret_cast<intptr_t>(static_cast<Type1*>(_pComparand)));
            }

            inline int32_t __YYAPI Exchange(volatile int32_t* _pDestination, int32_t _iExchange)
            {
#ifdef _MSC_VER
                return (int32_t)_InterlockedExchange(reinterpret_cast<long volatile*>(_pDestination), _iExchange);
#else
                return __sync_lock_test_and_set(_pDestination, _iExchange);
#endif
            }

            inline int32_t __YYAPI Exchange(int32_t* _pDestination, int32_t _iExchange)
            {
                return Exchange(const_cast<volatile int32_t*>(_pDestination), _iExchange);
            }

            inline uint32_t __YYAPI Exchange(volatile uint32_t* _pDestination, uint32_t _iExchange) noexcept
            {
                return (uint32_t)Exchange(reinterpret_cast<volatile int32_t*>(_pDestination), (int32_t)_iExchange);
            }

            inline uint32_t __YYAPI Exchange(uint32_t* _pDestination, uint32_t _iExchange)
            {
                return (uint32_t)Exchange((int32_t*)_pDestination, (int32_t)_iExchange);
            }

            inline int64_t __YYAPI Exchange(volatile int64_t* _pDestination, int64_t _iExchange)
            {
#ifdef _MSC_VER
                return (int64_t)_interlockedexchange64(reinterpret_cast<long long volatile*>(_pDestination), _iExchange);
#else
                return __sync_lock_test_and_set(_pDestination, _iExchange);
#endif
            }

            inline uint64_t __YYAPI Exchange(volatile uint64_t* _pDestination, uint64_t _iExchange) noexcept
            {
                return (uint64_t)Exchange(reinterpret_cast<volatile int64_t*>(_pDestination), (int64_t)_iExchange);
            }

            inline int64_t __YYAPI Exchange(int64_t* _pDestination, int64_t _iExchange)
            {
                return Exchange(const_cast<volatile int64_t*>(_pDestination), _iExchange);
            }

            inline uint64_t __YYAPI Exchange(uint64_t* _pDestination, uint64_t _iExchange) noexcept
            {
                return (uint64_t)Exchange(reinterpret_cast<volatile uint64_t*>(_pDestination), _iExchange);
            }

            template<typename _Type>
            inline _Type* __YYAPI ExchangePoint(volatile _Type** _ppDestination, _Type* _pExchange)
            {
                return (_Type*)Exchange(reinterpret_cast<volatile intptr_t*>(_ppDestination), (intptr_t)_pExchange);
            }

            template<typename _Type>
            inline _Type* __YYAPI ExchangePoint(_Type** _ppDestination, _Type* _pExchange)
            {
                return (_Type*)Exchange(reinterpret_cast<intptr_t*>(_ppDestination), (intptr_t)_pExchange);
            }

            template<typename _Type>
            inline _Type* __YYAPI ExchangePoint(volatile _Type** _ppDestination, std::nullptr_t)
            {
                return (_Type*)Exchange(reinterpret_cast<volatile intptr_t*>(_ppDestination), (intptr_t)0);
            }

            template<typename _Type>
            inline _Type* __YYAPI ExchangePoint(_Type** _ppDestination, std::nullptr_t)
            {
                return (_Type*)Exchange(reinterpret_cast<intptr_t*>(_ppDestination), (intptr_t)0);
            }

            inline int32_t __YYAPI BitOr(_In_ volatile int32_t* _pDestination, int32_t _iValue) noexcept
            {
                auto _iLastVlaue = *_pDestination;

                for (;;)
                {
                    auto _iCurrentValue = CompareExchange(_pDestination, _iLastVlaue | _iValue, _iLastVlaue);
                    if (_iCurrentValue == _iLastVlaue)
                        break;

                    _iLastVlaue = _iCurrentValue;
                }

                return _iLastVlaue;
            }

            inline int32_t __YYAPI BitOr(_In_ int32_t* _pDestination, int32_t _iValue) noexcept
            {
                return BitOr(const_cast<volatile int32_t*>(_pDestination), _iValue);
            }

            inline uint32_t __YYAPI BitOr(_In_ volatile uint32_t* _pDestination, uint32_t _iValue) noexcept
            {
                return (uint32_t)BitOr((volatile int32_t*)_pDestination, (int32_t)_iValue);
            }

            inline uint32_t __YYAPI BitOr(_In_ uint32_t* _pDestination, uint32_t _iValue) noexcept
            {
                return BitOr((volatile uint32_t*)_pDestination, _iValue);
            }

            inline int32_t __YYAPI BitXor(_In_ int32_t* _pDestination, int32_t _iValue)
            {
                auto _iLastVlaue = *_pDestination;

                for (;;)
                {
                    auto _iCurrentValue = CompareExchange(_pDestination, _iLastVlaue ^ _iValue, _iLastVlaue);
                    if (_iCurrentValue == _iLastVlaue)
                        break;

                    _iLastVlaue = _iCurrentValue;
                }

                return _iLastVlaue;
            }

            inline uint32_t __YYAPI BitXor(_In_ uint32_t* _pDestination, uint32_t _iValue)
            {
                return (uint32_t)BitXor((int32_t*)_pDestination, (int32_t)_iValue);
            }

            inline int32_t __YYAPI BitAnd(_In_ int32_t* _pDestination, int32_t _iValue)
            {
                auto _iLastVlaue = *_pDestination;

                for (;;)
                {
                    auto _iCurrentValue = CompareExchange(_pDestination, _iLastVlaue & _iValue, _iLastVlaue);
                    if (_iCurrentValue == _iLastVlaue)
                        break;

                    _iLastVlaue = _iCurrentValue;
                }

                return _iLastVlaue;
            }

            inline uint32_t __YYAPI BitAnd(_In_ uint32_t* _pDestination, uint32_t _iValue)
            {
                return (uint32_t)BitAnd((int32_t*)_pDestination, (int32_t)_iValue);
            }
        }
    }
}

namespace YY
{
    using namespace YY::Base::Sync;
}
