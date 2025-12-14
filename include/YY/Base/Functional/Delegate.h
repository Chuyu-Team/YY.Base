#pragma once
#include <YY/Base/YY.h>
#include <YY/Base/Containers/DoublyLinkedList.h>

#pragma pack(push, __YY_PACKING)

namespace YY
{
    namespace Base
    {
        namespace Functional
        {
            template<typename DelegateHandle>
            class Delegate
            {
            protected:
                YY::DoublyLinkedList<DelegateHandle> oDelegateHandleList;

            public:
                constexpr Delegate() = default;

                constexpr Delegate(Delegate&& _oOther) noexcept
                    : oDelegateHandleList(_oOther.oDelegateHandleList.Flush())
                {
                }

                Delegate(const Delegate&) = delete;
                Delegate& operator=(const Delegate&) = delete;

                bool __YYAPI AddHandler(DelegateHandle* _pHandle)
                {
                    if (!_pHandle)
                    {
                        return false;
                    }

                    if (_pHandle->pPrior != nullptr || _pHandle->pNext != nullptr)
                    {
                        // 已经在链表中！！！
                        return false;
                    }

                    oDelegateHandleList.PushBack(_pHandle);
                    return true;
                }

                bool __YYAPI RemoveHandler(DelegateHandle* _pHandle)
                {
                    if (!_pHandle)
                        return false;

                    if (_pHandle->pPrior == nullptr)
                    {
                        // 头节点
                        if (oDelegateHandleList.GetFirst() == _pHandle)
                        {
                            oDelegateHandleList.Remove(_pHandle);
                            return true;
                        }
                    }
                    else if (_pHandle->pNext == nullptr)
                    {
                        // 尾节点
                        if (oDelegateHandleList.GetLast() == _pHandle)
                        {
                            oDelegateHandleList.Remove(_pHandle);
                            return true;
                        }
                    }
                    else
                    {
                        // 中间节点，检查是否在链表中
                        for (auto _pEntry = oDelegateHandleList.GetFirst(); _pEntry != nullptr; _pEntry = _pEntry->pNext)
                        {
                            if (_pEntry == _pHandle)
                            {
                                oDelegateHandleList.Remove(_pHandle);
                                return true;
                            }
                        }
                    }

                    return false;
                }

                Delegate& __YYAPI operator+=(Delegate&& _oOther)
                {
                    oDelegateHandleList.PushBack(_oOther.oDelegateHandleList.Flush());
                    return *this;
                }

                Delegate& __YYAPI operator+=(DelegateHandle* _pHandle)
                {
                    AddHandler(_pHandle);
                    return *this;
                }

                Delegate& __YYAPI operator-=(DelegateHandle* _pHandle)
                {
                    RemoveHandler(_pHandle);
                    return *this;
                }

                template<typename InvokeCallback>
                void __YYAPI Invoke(InvokeCallback&& _pfnInvoke)
                {
                    while (auto _pEntry = oDelegateHandleList.PopFront())
                    {
                        _pfnInvoke(_pEntry);
                    }
                }
            };
        }
    }

    using namespace YY::Base::Functional;
}

#pragma pack(pop)
