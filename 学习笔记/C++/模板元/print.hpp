#pragma once
#include <type_traits>
#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <optional>
#include <variant>

// 基于SFINAE的实现
namespace _print_details {
    // 模板原型一   is_default_print是true
    template <class T, class = void>
    struct _printer {
        static void print(T const &t) {
            std::cout << t;
        }

        using is_default_print = std::true_type;
    };


    // 模板原型二   默认只有一个type成员是false
    template <class T, class = void>
    struct _is_default_printable : std::false_type {
    };

    // std::declval<>() 可以在不实例化的条件下获取对象
    // 偏例化一  用std::void_t<std::pair<typename _printer<T>::is_default_print, 
    // decltype(std::declval<std::ostream &>() << std::declval<T const &>())>>
    // 约束T满足 _printer<T>::is_default_print 存在 并且 ostream& << T
    // 即可以直接print
    template <class T> 
    struct _is_default_printable<T, std::void_t<std::pair<typename _printer<T>::is_default_print, decltype(std::declval<std::ostream &>() << std::declval<T const &>())>>> : std::true_type {
    };

    template <class T, class = void>
    struct _is_printer_printable : std::true_type {
    };

    template <class T>
    struct _is_printer_printable<T, std::void_t<typename _printer<T>::is_default_print>> : std::false_type {
    };

    // disjunction 相当于或
    // 所以is_printable 代表default printable 或者 手动实现可以print
    template <class T, class = void>
    struct is_printable : std::disjunction<_is_default_printable<T>, _is_printer_printable<T>> {
    };

    // remove const volatile reference 
    template <class T>
    using _rmcvref_t = std::remove_cv_t<std::remove_reference_t<T>>;


    template <class T, class U = void, class = void>
    struct _enable_if_tuple {
        using not_type = U;
    };

    // tuple_size<T>::value 获取大小(需要T支持静态获取大小) 通过decltype获取的是size_t
    // 
    template <class T, class U>
    struct _enable_if_tuple<T, U, std::void_t<decltype(std::tuple_size<T>::value)>> {
        using type = U;
    };

    template <class T, class U = void, class = void>
    struct _enable_if_map {
        using not_type = U;
    };

    // 比较T::value_type 和 std::pair<T::key_type const, T::mapped_type>
    // std::enable_if_t<std::is_same_v<typename T::value_type, 
    // std::pair<typename T::key_type const, typename T::mapped_type> >>
    template <class T, class U>
    struct _enable_if_map<T, U, std::enable_if_t<std::is_same_v<typename T::value_type, std::pair<typename T::key_type const, typename T::mapped_type>>>> {
        using type = U;
    };

    template <class T, class U = void, class = void>
    struct _enable_if_iterable {
        using not_type = U;
    };

    // T 可以获取iterable 
    template <class T, class U>
    struct _enable_if_iterable<T, U, std::void_t<typename std::iterator_traits<decltype(std::begin(std::declval<T const &>()))>::value_type>> {
        using type = U;
    };

    template <class T, class U = void, class = void>
    struct _enable_if_has_print {
        using not_type = U;
    };

    // T实现了do_print方法
    template <class T, class U>
    struct _enable_if_has_print<T, U, std::void_t<decltype(std::declval<T const &>().do_print())>> {
        using type = U;
    };

    /* template <class T, class U> */
    /* struct _enable_if_iterable<T, U, std::void_t<typename std::iterator_traits<typename T::const_iterator>::value_type>> { */
    /*     using type = U; */
    /* }; */

    template <class T>
    struct _is_char : std::false_type {
    };

    template <>
    struct _is_char<char> : std::true_type {
    };

    template <>
    struct _is_char<wchar_t> : std::true_type {
    };

    // 默认模板
    template <class T, class U = void, class = void>
    struct _enable_if_char {
        using not_type = U;
    };
    // enable_if_t<condtion,Tp_>如果condition == true 则获取Tp_类型 默认Tp_是void
    // T是char或者wchar则实例化这个
    template <class T, class U>
    struct _enable_if_char<T, U, std::enable_if_t<_is_char<T>::value>> {
        using type = U;
    };

    template <class T, class U = void, class = void>
    struct _enable_if_string {
        using not_type = U;
    };

    // string
    template <class T, class Alloc, class Traits, class U>
    struct _enable_if_string<std::basic_string<T, Alloc, Traits>, U, typename _enable_if_char<T>::type> {
        using type = U;
    };
    // string_view
    template <class T, class Traits, class U>
    struct _enable_if_string<std::basic_string_view<T, Traits>, U, typename _enable_if_char<T>::type> {
        using type = U;
    };

    template <class T, class U = void, class = void>
    struct _enable_if_c_str {
        using not_type = U;
    };

    // c_str需要T是char * 
    template <class T, class U>
    struct _enable_if_c_str<T, U, std::enable_if_t<std::is_pointer_v<std::decay_t<T>> && _is_char<std::remove_const_t<std::remove_pointer_t<std::decay_t<T>>>>::value>> {
        using type = U;
    };

    template <class T, class U = void, class = void>
    struct _enable_if_optional {
        using not_type = U;
    };

    template <class T, class U>
    struct _enable_if_optional<std::optional<T>, U, void> {
        using type = U;
    };

    template <class T, class U = void, class = void>
    struct _enable_if_variant {
        using not_type = U;
    };

    template <class ...Ts, class U>
    struct _enable_if_variant<std::variant<Ts...>, U, void> {
        using type = U;
    };
    // 对于实现了do_print的类型
    template <class T>
    struct _printer<T, typename _enable_if_has_print<T>::type> {
        static void print(T const &t) {
            t.do_print();
        }
    };
    // 没有实现do_print
    // template <class T, class U = void, class = void>
    // struct _enable_if_has_print {
    //     using not_type = U;
    // };
    // 多层嵌套  typename _enable_if_map<T>::not_type  ->  _enable_if_string<T, typename _enable_if_map<T>::not_type>::not_type>::not_type
    // ->  _enable_if_iterable<T, ...>::type   ->  _enable_if_has_print<T, ...>::not_type
    // not type 说明匹配失败
    // 仅仅 iterable 即实现了begin 和 end
    // typename _enable_if_has_print<T, typename _enable_if_iterable<T, 
    // typename _enable_if_c_str<T, typename _enable_if_string<T, 
    // typename _enable_if_map<T>::not_type>::not_type>::not_type>::type>::not_type
    template <class T>
    struct _printer<T, typename _enable_if_has_print<T, typename _enable_if_iterable<T, typename _enable_if_c_str<T, typename _enable_if_string<T, typename _enable_if_map<T>::not_type>::not_type>::not_type>::type>::not_type> {
        static void print(T const &t) {
            std::cout << "{";
            bool once = false;
            for (auto const &v: t) {
                if (once) {
                    std::cout << ", ";
                } else {
                    once = true;
                }
                _printer<_rmcvref_t<decltype(v)>>::print(v);
            }
            std::cout << "}";
        }
    };
    // _enable_if_tuple<T, typename _enable_if_iterable<T>::not_type>::type
    // 对于tuple对象
    template <class T>
    struct _printer<T, typename _enable_if_has_print<T, typename _enable_if_tuple<T, typename _enable_if_iterable<T>::not_type>::type>::not_type> {
        template <std::size_t ...Is>
        static void _unrolled_print(T const &t, std::index_sequence<Is...>) {
            std::cout << "{";
            //(_printer<_rmcvref_t<std::tuple_element_t<Is, T>>>::print(std::get<Is>(t)), std::cout << ", "), ...);
            //,... 右展开     tuple_element_t获取T中第Is个元素的类型
            // 所以就是对于T(tuple like)中的每一个元素获取类型实例化_printer<>模板并调用print其中的元素
            // 并且如果其中元素不能直接cout会递归从模板实例化🐮
            // 最后打印大小
            ((_printer<_rmcvref_t<std::tuple_element_t<Is, T>>>::print(std::get<Is>(t)), std::cout << ", "), ...);
            if constexpr (sizeof...(Is) != 0) _printer<_rmcvref_t<std::tuple_element_t<sizeof...(Is), T>>>::print(std::get<sizeof...(Is)>(t));
            std::cout << "}";
        }

        static void print(T const &t) {
            // 获取constexpr的integer序列 上限是max(1,tuple_size_v<T>) - 1
            // tuple是可以编译器元素数量的类型  例如using Type = std::tuple<int,double>;
            // std::tuple_size_v<Type> == 2 
            _unrolled_print(t, std::make_index_sequence<std::max(static_cast<std::size_t>(1), std::tuple_size_v<T>) - 1>{});
        }
    };
    // 对于map对象
    // { key1: value1, key2: value2, ...}
    template <class T>
    struct _printer<T, typename _enable_if_has_print<T, typename _enable_if_map<T>::type>::not_type> {
        static void print(T const &t) {
            std::cout << "{";
            bool once = false;
            for (auto const &[k, v]: t) {
                if (once) {
                    std::cout << ", ";
                } else {
                    once = true;
                }
                _printer<_rmcvref_t<decltype(k)>>::print(k);
                std::cout << ": ";
                _printer<_rmcvref_t<decltype(v)>>::print(v);
            }
            std::cout << "}";
        }
    };
    // string like
    template <class T>
    struct _printer<T, typename _enable_if_has_print<T, typename _enable_if_string<T>::type>::not_type> {
        static void print(T const &t) {
            std::cout << std::quoted(t);
        }
    };
    
    template <class T>
    struct _printer<T, typename _enable_if_c_str<T>::type> {
        static void print(T const &t) {
            std::cout << t;
        }
    };

    template <class T>
    struct _printer<T, typename _enable_if_char<T>::type> {
        static void print(T const &t) {
            T s[2] = {t, T('\0')};
            std::cout << std::quoted(s, T('\''));
        }
    };
    // 空指针
    template <>
    struct _printer<std::nullptr_t, void> {
        static void print(std::nullptr_t const &) {
            std::cout << "nullptr";
        }
    };
    // 空的optional
    template <>
    struct _printer<std::nullopt_t, void> {
        static void print(std::nullopt_t const &) {
            std::cout << "nullopt";
        }
    };
    // 空的 variant
    template <>
    struct _printer<std::monostate, void> {
        static void print(std::monostate const &) {
            std::cout << "monostate";
        }
    };
    // optional含有值 对std::optional<t>对象获取值*t
    template <class T>
    struct _printer<T, typename _enable_if_has_print<T, typename _enable_if_optional<T>::type>::not_type> {
        static void print(T const &t) {
            if (t.has_value()) {
                _printer<typename T::value_type>::print(*t);
            } else {
                _printer<std::nullopt_t>::print(std::nullopt);
            }
        }
    };
    // 对于variant
    template <class T>
    struct _printer<T, typename _enable_if_has_print<T, typename _enable_if_variant<T>::type>::not_type> {
        static void print(T const &t) {
            std::visit([] (auto const &v) {
                _printer<_rmcvref_t<decltype(v)>>::print(v);
            }, t);
        }
    };

    template <>
    struct _printer<bool, void> {
        static void print(bool const &t) {
            if (t) {
                std::cout << "true";
            } else {
                std::cout << "false";
            }
        }
    };
    // print实际使用
    template <class T0, class ...Ts>
    void print(T0 const &t0, Ts const &...ts) {
        _printer<_rmcvref_t<T0>>::print(t0);
        ((std::cout << " ", _printer<_rmcvref_t<Ts>>::print(ts)), ...);
        std::cout << "\n";
    }
    // 不换行
    template <class T0, class ...Ts>
    void printnl(T0 const &t0, Ts const &...ts) {
        _printer<_rmcvref_t<T0>>::print(t0);
        ((std::cout << " ", _printer<_rmcvref_t<Ts>>::print(ts)), ...);
    }
    // 更一般的对象
    template <class T, class = void>
    class print_adaptor {
        T const &t;

    public:
        explicit constexpr print_adaptor(T const &t_) : t(t_) {
        }

        friend std::ostream &operator<<(std::ostream &os, print_adaptor const &&self) {
            auto oldflags = os.flags();
            os << "[object 0x" << std::hex << reinterpret_cast<uintptr_t>(
                reinterpret_cast<void const volatile *>(std::addressof(self.t))) << ']';
            os.flags(oldflags);
            // 恢复输出格式
            return os;
        }
    };

    template <class T>
    class print_adaptor<T, std::enable_if_t<is_printable<T>::value>> {
        T const &t;

    public:
        explicit constexpr print_adaptor(T const &t_) : t(t_) {
        }

        friend std::ostream &operator<<(std::ostream &os, print_adaptor const &&self) {
            printnl(self.t);
            return os;
        }
    };

    template <class T>
    explicit print_adaptor(T const &) -> print_adaptor<T>;
}

using _print_details::print;
using _print_details::printnl;
using _print_details::print_adaptor;
using _print_details::is_printable;

// Usage:
//
// map<string, optional<int>> m = {{"hello", 42}, {"world", nullopt}};
// print(m);  // {"hello": 42, "world": nullopt}


/* // use of the macro below requires #include "ppforeach.h" */
/* #define DEF_PRINT(Class, TmplArgs, ...) \ */
/* template <TmplArgs> \ */
/* struct ::constl::_print_details::_printer<Class, void> { \ */
/*     static void print(Class const &_cls) { \ */
/*         std::cout << "{"; \ */
/*         PP_FOREACH(_PRINTER_PER_MEMBER, std::cout << ", ";, __VA_ARGS__); \ */
/*         std::cout << "}"; \ */
/*     } \ */
/* }; */
/* #define _PRINTER_PER_MEMBER(memb) \ */
/*     std::cout << #memb << ": "; \ */
/*     ::constl::_print_details::_printer<_print_details::_rmcvref_t<decltype(_cls.memb)>>::print(_cls.memb); */
/*  */
/* #define PRINT(x) print(#x " :=", x) */