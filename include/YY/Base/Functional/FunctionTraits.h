#pragma once
#include <tuple>
#include <type_traits>

#pragma push_macro("__APPLY_CLASS_FUNCTION_TRAITS")
#undef __APPLY_CLASS_FUNCTION_TRAITS

#pragma push_macro("__APPLY_CLASS_FUNCTION_TRAITS2")
#undef __APPLY_CLASS_FUNCTION_TRAITS2

#pragma push_macro("__APPLY_CLASS_FUNCTION_TRAITS3")
#undef __APPLY_CLASS_FUNCTION_TRAITS3

#pragma push_macro("__APPLY_FUNCTION_TRAITS")
#undef __APPLY_FUNCTION_TRAITS

#pragma push_macro("__DEFAULT")
#undef __DEFAULT
#define __DEFAULT

namespace YY
{
    namespace Base
    {
        namespace Functional
        {
            template <typename T>
            struct FunctionTraits;

            template <typename _ReturnType, typename... _Parameters>
            struct FunctionTraits<_ReturnType(_Parameters...)>
            {
                using ReturnType = _ReturnType;
                using ParametersTuple = std::tuple<_Parameters...>;
                static constexpr std::size_t kParameterCount = sizeof...(_Parameters);

                template <std::size_t kIndex>
                using ParameterTypeAt = typename std::tuple_element<kIndex, ParametersTuple>::type;
            };

            template <typename _ReturnType, typename... _Parameters>
            struct FunctionTraits<_ReturnType(_Parameters...) noexcept> : public FunctionTraits<_ReturnType(_Parameters...)>
            {
            };

#if defined(_MSC_VER) && defined(_M_IX86)
            // 函数
#define __APPLY_FUNCTION_TRAITS(CALL_CONV_SPEC)                                                                                                 \
            template <typename _ReturnType, typename... _Parameters>                                                                            \
            struct FunctionTraits<_ReturnType(CALL_CONV_SPEC*)(_Parameters...)> : public FunctionTraits<_ReturnType(_Parameters...)>            \
            {                                                                                                                                   \
            };                                                                                                                                  \
                                                                                                                                                \
            template <typename _ReturnType, typename... _Parameters>                                                                            \
            struct FunctionTraits<_ReturnType(CALL_CONV_SPEC*)(_Parameters...) noexcept> : public FunctionTraits<_ReturnType(_Parameters...)>   \
            {                                                                                                                                   \
            }

            __APPLY_FUNCTION_TRAITS(__cdecl);
            __APPLY_FUNCTION_TRAITS(__stdcall);
            __APPLY_FUNCTION_TRAITS(__fastcall);
            __APPLY_FUNCTION_TRAITS(__vectorcall);

            // 类成员函数
#define __APPLY_CLASS_FUNCTION_TRAITS3(CALL_CONV_SPEC, CV_QUALIFIERS, REF_QUALIFIER)                                                                                                   \
            template <typename _ClassType, typename _ReturnType, typename... _Parameters>                                                                                              \
            struct FunctionTraits<_ReturnType(CALL_CONV_SPEC _ClassType::*)(_Parameters...) CV_QUALIFIERS REF_QUALIFIER> : FunctionTraits<_ReturnType(_Parameters...)>                 \
            {                                                                                                                                                                          \
                using ClassType = _ClassType;                                                                                                                                          \
                using ThisObject = CV_QUALIFIERS _ClassType;                                                                                                                           \
            };                                                                                                                                                                         \
                                                                                                                                                                                       \
            template <typename _ClassType, typename _ReturnType, typename... _Parameters>                                                                                              \
            struct FunctionTraits<_ReturnType(CALL_CONV_SPEC _ClassType::*)(_Parameters...) CV_QUALIFIERS REF_QUALIFIER noexcept> : FunctionTraits<_ReturnType(_Parameters...)>        \
            {                                                                                                                                                                          \
                using ClassType = _ClassType;                                                                                                                                          \
                using ThisObject = CV_QUALIFIERS _ClassType;                                                                                                                           \
            };

#define __APPLY_CLASS_FUNCTION_TRAITS2(CV_QUALIFIERS, REF_QUALIFIER)                    \
            __APPLY_CLASS_FUNCTION_TRAITS3(__thiscall, CV_QUALIFIERS, REF_QUALIFIER)    \
            __APPLY_CLASS_FUNCTION_TRAITS3(__cdecl, CV_QUALIFIERS, REF_QUALIFIER)       \
            __APPLY_CLASS_FUNCTION_TRAITS3(__stdcall, CV_QUALIFIERS, REF_QUALIFIER)     \
            __APPLY_CLASS_FUNCTION_TRAITS3(__fastcall, CV_QUALIFIERS, REF_QUALIFIER)    \
            __APPLY_CLASS_FUNCTION_TRAITS3(__vectorcall, CV_QUALIFIERS, REF_QUALIFIER)                                                                                                                                    

#else // defined(_MSC_VER) && defined(_M_IX86)
            template <typename _ReturnType, typename... _Parameters>
            struct FunctionTraits<_ReturnType(*)(_Parameters...)> : public FunctionTraits<_ReturnType(_Parameters...)>
            {
            };

            template <typename _ReturnType, typename... _Parameters>
            struct FunctionTraits<_ReturnType(*)(_Parameters...) noexcept> : public FunctionTraits<_ReturnType(_Parameters...)>
            {
            };

            // 类成员函数
#define __APPLY_CLASS_FUNCTION_TRAITS2(CV_QUALIFIERS, REF_QUALIFIER)                                                                                                        \
            template <typename _ClassType, typename _ReturnType, typename... _Parameters>                                                                                   \
            struct FunctionTraits<_ReturnType(_ClassType::*)(_Parameters...) CV_QUALIFIERS REF_QUALIFIER> : FunctionTraits<_ReturnType(_Parameters...)>                     \
            {                                                                                                                                                               \
                using ClassType = _ClassType;                                                                                                                               \
                using ThisObject = CV_QUALIFIERS _ClassType;                                                                                                                \
            };                                                                                                                                                              \
                                                                                                                                                                            \
            template <typename _ClassType, typename _ReturnType, typename... _Parameters>                                                                                   \
            struct FunctionTraits<_ReturnType(_ClassType::*)(_Parameters...) CV_QUALIFIERS REF_QUALIFIER noexcept> : FunctionTraits<_ReturnType(_Parameters...)>            \
            {                                                                                                                                                               \
                using ClassType = _ClassType;                                                                                                                               \
                using ThisObject = CV_QUALIFIERS _ClassType;                                                                                                                \
            };

#endif // defined(_MSC_VER) && defined(_M_IX86)


#define __APPLY_CLASS_FUNCTION_TRAITS(CV_QUALIFIERS)                  \
            __APPLY_CLASS_FUNCTION_TRAITS2(CV_QUALIFIERS, __DEFAULT)  \
            __APPLY_CLASS_FUNCTION_TRAITS2(CV_QUALIFIERS, &)          \
            __APPLY_CLASS_FUNCTION_TRAITS2(CV_QUALIFIERS, &&)


            __APPLY_CLASS_FUNCTION_TRAITS(__DEFAULT)
            __APPLY_CLASS_FUNCTION_TRAITS(const)
            __APPLY_CLASS_FUNCTION_TRAITS(volatile)
            __APPLY_CLASS_FUNCTION_TRAITS(const volatile)
        } // namespace Functional
    } // namespace Base

    using namespace YY::Base::Functional;
} // namespace YY

#undef __DEFAULT
#pragma pop_macro("__DEFAULT")

#undef __APPLY_FUNCTION_TRAITS
#pragma pop_macro("__APPLY_FUNCTION_TRAITS")

#undef __APPLY_CLASS_FUNCTION_TRAITS3
#pragma pop_macro("__APPLY_CLASS_FUNCTION_TRAITS3")

#undef __APPLY_CLASS_FUNCTION_TRAITS2
#pragma pop_macro("__APPLY_CLASS_FUNCTION_TRAITS2")

#undef __APPLY_CLASS_FUNCTION_TRAITS
#pragma pop_macro("__APPLY_CLASS_FUNCTION_TRAITS")
