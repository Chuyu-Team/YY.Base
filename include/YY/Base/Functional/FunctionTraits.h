#pragma once
#include <tuple>
#include <type_traits>

#pragma push_macro("__APPLY_FUNCTION_TRAITS")
#undef __APPLY_FUNCTION_TRAITS

#pragma push_macro("__APPLY_FUNCTION_TRAITS2")
#undef __APPLY_FUNCTION_TRAITS2

#pragma push_macro("__APPLY_CLASS_FUNCTION_TRAITS")
#undef __APPLY_CLASS_FUNCTION_TRAITS

#pragma push_macro("__APPLY_CLASS_FUNCTION_TRAITS2")
#undef __APPLY_CLASS_FUNCTION_TRAITS2

#pragma push_macro("__APPLY_CLASS_FUNCTION_TRAITS3")
#undef __APPLY_CLASS_FUNCTION_TRAITS3

#pragma push_macro("__APPLY_CLASS_FUNCTION_TRAITS4")
#undef __APPLY_CLASS_FUNCTION_TRAITS4

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

            // 函数
#define __APPLY_FUNCTION_TRAITS2(CALL_CONV_SPEC, NOEXCEPT_SPEC)                                                                                    \
            template <typename _ReturnType, typename... _Parameters>                                                                               \
            struct FunctionTraits<_ReturnType(CALL_CONV_SPEC*)(_Parameters...) NOEXCEPT_SPEC> : public FunctionTraits<_ReturnType(_Parameters...)> \
            {                                                                                                                                      \
            };

            // 类成员函数
#define __APPLY_CLASS_FUNCTION_TRAITS4(CALL_CONV_SPEC, CV_QUALIFIERS, REF_QUALIFIER, NOEXCEPT_SPEC)                                                                                    \
            template <typename _ClassType, typename _ReturnType, typename... _Parameters>                                                                                              \
            struct FunctionTraits<_ReturnType(CALL_CONV_SPEC _ClassType::*)(_Parameters...) CV_QUALIFIERS REF_QUALIFIER NOEXCEPT_SPEC> : FunctionTraits<_ReturnType(_Parameters...)>   \
            {                                                                                                                                                                          \
                using ClassType = _ClassType;                                                                                                                                          \
                using ThisObject = CV_QUALIFIERS _ClassType;                                                                                                                           \
            };

#define __APPLY_CLASS_FUNCTION_TRAITS3(CALL_CONV_SPEC, REF_QUALIFIER, NOEXCEPT_SPEC)                              \
            __APPLY_CLASS_FUNCTION_TRAITS4(CALL_CONV_SPEC,               , REF_QUALIFIER, NOEXCEPT_SPEC)          \
            __APPLY_CLASS_FUNCTION_TRAITS4(CALL_CONV_SPEC,          const, REF_QUALIFIER, NOEXCEPT_SPEC)          \
            __APPLY_CLASS_FUNCTION_TRAITS4(CALL_CONV_SPEC,       volatile, REF_QUALIFIER, NOEXCEPT_SPEC)          \
            __APPLY_CLASS_FUNCTION_TRAITS4(CALL_CONV_SPEC, const volatile, REF_QUALIFIER, NOEXCEPT_SPEC)

#define __APPLY_CLASS_FUNCTION_TRAITS2(CALL_CONV_SPEC, NOEXCEPT_SPEC)                  \
            __APPLY_CLASS_FUNCTION_TRAITS3(CALL_CONV_SPEC,   , NOEXCEPT_SPEC)          \
            __APPLY_CLASS_FUNCTION_TRAITS3(CALL_CONV_SPEC,  &, NOEXCEPT_SPEC)          \
            __APPLY_CLASS_FUNCTION_TRAITS3(CALL_CONV_SPEC, &&, NOEXCEPT_SPEC)

#if defined(_WIN32) && defined(_M_IX86)

#define __APPLY_FUNCTION_TRAITS(NOEXCEPT_SPEC)                     \
            __APPLY_FUNCTION_TRAITS2(     __cdecl, NOEXCEPT_SPEC)  \
            __APPLY_FUNCTION_TRAITS2(   __stdcall, NOEXCEPT_SPEC)  \
            __APPLY_FUNCTION_TRAITS2(  __fastcall, NOEXCEPT_SPEC)  \
            __APPLY_FUNCTION_TRAITS2(__vectorcall, NOEXCEPT_SPEC)

#define __APPLY_CLASS_FUNCTION_TRAITS(NOEXCEPT_SPEC)                     \
            __APPLY_CLASS_FUNCTION_TRAITS2(  __thiscall, NOEXCEPT_SPEC)  \
            __APPLY_CLASS_FUNCTION_TRAITS2(     __cdecl, NOEXCEPT_SPEC)  \
            __APPLY_CLASS_FUNCTION_TRAITS2(   __stdcall, NOEXCEPT_SPEC)  \
            __APPLY_CLASS_FUNCTION_TRAITS2(  __fastcall, NOEXCEPT_SPEC)  \
            __APPLY_CLASS_FUNCTION_TRAITS2(__vectorcall, NOEXCEPT_SPEC)                                                                                                                            

#else // !(defined(_WIN32) && defined(_M_IX86))

#define __APPLY_FUNCTION_TRAITS(NOEXCEPT_SPEC)               \
            __APPLY_FUNCTION_TRAITS2(, NOEXCEPT_SPEC)

#define __APPLY_CLASS_FUNCTION_TRAITS(NOEXCEPT_SPEC)         \
            __APPLY_CLASS_FUNCTION_TRAITS2(, NOEXCEPT_SPEC)

#endif // defined(_WIN32) && defined(_M_IX86)

            __APPLY_FUNCTION_TRAITS(__DEFAULT)
            __APPLY_CLASS_FUNCTION_TRAITS(__DEFAULT)

#if defined(__cpp_noexcept_function_type) && __cpp_noexcept_function_type >= 201510L
            // 此时 noexcept 也是函数类型的一部分，需要单独处理
            template <typename _ReturnType, typename... _Parameters>
            struct FunctionTraits<_ReturnType(_Parameters...) noexcept> : public FunctionTraits<_ReturnType(_Parameters...)>
            {
            };

            __APPLY_FUNCTION_TRAITS(noexcept)
            __APPLY_CLASS_FUNCTION_TRAITS(noexcept)
#endif // defined(__cpp_noexcept_function_type) && __cpp_noexcept_function_type >= 201510L

        } // namespace Functional
    } // namespace Base

    using namespace YY::Base::Functional;
} // namespace YY

#undef __DEFAULT
#pragma pop_macro("__DEFAULT")

#undef __APPLY_CLASS_FUNCTION_TRAITS4
#pragma pop_macro("__APPLY_CLASS_FUNCTION_TRAITS4")

#undef __APPLY_CLASS_FUNCTION_TRAITS3
#pragma pop_macro("__APPLY_CLASS_FUNCTION_TRAITS3")

#undef __APPLY_CLASS_FUNCTION_TRAITS2
#pragma pop_macro("__APPLY_CLASS_FUNCTION_TRAITS2")

#undef __APPLY_CLASS_FUNCTION_TRAITS
#pragma pop_macro("__APPLY_CLASS_FUNCTION_TRAITS")

#undef __APPLY_FUNCTION_TRAITS2
#pragma pop_macro("__APPLY_FUNCTION_TRAITS2")

#undef __APPLY_FUNCTION_TRAITS
#pragma pop_macro("__APPLY_FUNCTION_TRAITS")
