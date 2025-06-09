#pragma once
#include <YY/Base/Memory/RefPtr.h>

#pragma pack(push, __YY_PACKING)

/*

ObserverPtr 它是一个轻量级的智能指针，主要用于观察对象的生命周期，当对象销毁时，如果尝试获取指针则返回 nullptr。这个有点类似于 std::weak_ptr，但它不需要对象存在引用计数。

注意：ObserverPtr因为没有引用计数，这导致它无法在多线程场景下安全使用。因为A线程持有指针时，无法保证其他线程不会在此时销毁对象。请务必将操作排队后使用。

使用方法：
```cpp
// 必须派生自 —— ObserverPtrFactory
class MyClass : public YY::Base::Memory::ObserverPtrFactory
{
};

MyClass _Object1;

ObserverPtr<MyClass> _pObject1Observer = _Object1;

// 获取指针，如果对象已销毁则返回 nullptr
auto _pObject1 = ptr.Get();

```
*/

namespace YY
{
    namespace Base
    {
        namespace Memory
        {
            class ObserverPtrFactory
            {
            public:
                class ObserverData
                {
                    friend ObserverPtrFactory;
                private:
                    ObserverPtrFactory* pBase = nullptr;
                    volatile uint32_t uRef = 1;
                    bool bDestroyed = true;

                public:
                    constexpr ObserverData() noexcept = default;

                    constexpr ObserverData(ObserverPtrFactory* _pBase) noexcept
                        : pBase(_pBase)
                        , bDestroyed(false)
                    {
                    }

                    ObserverPtrFactory* __YYAPI GetBasePtr() const noexcept
                    {
                        return pBase;
                    }

                    uint32_t __YYAPI AddRef() noexcept
                    {
                        return Sync::Increment(&uRef);
                    }

                    uint32_t __YYAPI Release() noexcept
                    {
                        auto _uNewRef = Sync::Decrement(&uRef);
                        if (_uNewRef == 0)
                        {
                            delete this;
                        }

                        return _uNewRef;
                    }

                    bool IsDestroyed() const noexcept
                    {
                        return bDestroyed;
                    }
                };

            private:
                RefPtr<ObserverData> pObserverData;

            public:
                constexpr ObserverPtrFactory() noexcept = default;

                constexpr ObserverPtrFactory(const ObserverPtrFactory&) noexcept = default;

                ~ObserverPtrFactory()
                {
                    if (pObserverData)
                    {
                        pObserverData->bDestroyed = true;
                    }
                }

                ObserverPtrFactory& __YYAPI operator=(const ObserverPtrFactory&) noexcept
                {
                    return *this;
                }

                RefPtr<ObserverData> __YYAPI GetObserverData() noexcept
                {
                    if (!pObserverData)
                    {
                        pObserverData = RefPtr<ObserverData>::Create(this);
                    }

                    return pObserverData;
                }
            };

            template<typename _Type>
            class ObserverPtr
            {
            private:
                RefPtr<ObserverPtrFactory::ObserverData> pObserverData;

            public:
                constexpr ObserverPtr() noexcept = default;

                ObserverPtr(_In_opt_ _Type* _pOther) noexcept
                {
                    Reset(_pOther);
                }

                ObserverPtr(_In_ const ObserverPtr& _pOther) noexcept
                    : pObserverData(_pOther.pObserverData)
                {
                }

                ObserverPtr(_In_ ObserverPtr&& _pOther) noexcept
                    : pObserverData(std::move(_pOther.pObserverData))
                {
                }

                ~ObserverPtr()
                {
                    Reset();
                }

                _Ret_maybenull_ _Type* __YYAPI Get() const noexcept
                {
                    if(pObserverData == nullptr || pObserverData->IsDestroyed())
                    {
                        return nullptr;
                    }

                    return static_cast<_Type*>(pObserverData->GetBasePtr());
                }

                void __YYAPI Reset() noexcept
                {
                    pObserverData.Reset();
                }

                void __YYAPI Reset(_In_opt_ _Type* _pOther) noexcept
                {
                    if (_pOther)
                    {
                        pObserverData = _pOther->GetObserverData();
                    }
                    else
                    {
                        pObserverData.Reset();
                    }
                }

                bool __YYAPI operator==(_In_opt_ const _Type* _pOther) const noexcept
                {
                    return GetRawPtr() == _pOther;
                }

                bool __YYAPI operator==(_In_opt_ const ObserverPtr& _pOther) const noexcept
                {
                    return GetRawPtr() == _pOther.GetRawPtr();
                }

                bool __YYAPI operator!=(_In_opt_ const _Type* _pOther) const noexcept
                {
                    return GetRawPtr() != _pOther;
                }

                bool __YYAPI operator!=(_In_opt_ const ObserverPtr& _pOther) const noexcept
                {
                    return GetRawPtr() != _pOther.GetRawPtr();
                }

                ObserverPtr& __YYAPI operator=(std::nullptr_t) noexcept
                {
                    Reset();
                    return *this;
                }

                ObserverPtr& __YYAPI operator=(_In_opt_ _Type* _pOther) noexcept
                {
                    Reset(_pOther);
                    return *this;
                }

                ObserverPtr& __YYAPI operator=(const ObserverPtr& _pOther) noexcept
                {
                    pObserverData = _pOther.pObserverData;
                    return *this;
                }

                ObserverPtr& __YYAPI operator=(ObserverPtr&& _pOther) noexcept
                {
                    pObserverData = std::move(_pOther.pObserverData);
                    return *this;
                }
                
            private:
                _Ret_maybenull_ _Type* __YYAPI GetRawPtr() const noexcept
                {
                    if (pObserverData == nullptr)
                    {
                        return nullptr;
                    }

                    return static_cast<_Type*>(pObserverData->GetBasePtr());
                }
            };
        } // namespace Memory
    } // namespace Base
} // namespace YY

namespace YY
{
    using namespace YY::Base::Memory;
}
#pragma pack(pop)
