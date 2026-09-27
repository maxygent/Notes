#ifndef __MPRINT_HPP
#define __MPRINT_HPP
#include <type_traits>
#include <iostream>
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <cstdint>
#include <iomanip>
#include <ranges>

// 使用concept简化了部分类型萃取  更加可读
namespace _print_detials{

    template<class T>
    struct _printer{
        static void print(T const& t)
        {
            std::cout << t;
        }
        using is_default_print = std::true_type;
    };

    template<class T>
    concept _enable_if_tuple = requires(){
        std::tuple_size<T>::value;
    };

    template<class T>
    concept _enable_if_map = requires {
            typename T::key_type;
            typename T::mapped_type;
            typename T::value_type; } &&
            std::same_as < typename T::value_type,
            std::pair < typename T::key_type const,
                        typename T::mapped_type>>;

    template<class T>
    concept _enable_if_has_print = requires(T const& t){
        t.do_print();
    };

    template<class T>
    concept _enable_if_char = 
        std::same_as<T,char> ||
        std::same_as<T,wchar_t>;

    template<class T>
    struct is_optional : std::false_type {};

    template<class U>
    struct is_optional<std::optional<U>> : std::true_type {};

    template<class T>
    concept _enable_if_optional = is_optional<T>::value;

    template<class T>
    struct is_variant : std::false_type {};

    template<class ...Ts>
    struct is_variant<std::variant<Ts...>> : std::true_type {};

    template<class T>
    concept _enable_if_variant = is_variant<T>::value;

    template<class T>
    struct is_string : std::false_type {};

    // std::string = std::basic_string<char,std::char_traits<char>,std::allocator<char>>
    // 所以is_string<std::string>会匹配到这个
    template<class C, class Traits, class Alloc>
    struct is_string<std::basic_string<C, Traits, Alloc>>: std::true_type {};

    template<class C, class Traits>
    struct is_string<std::basic_string_view<C, Traits>>: std::true_type {};

    template<class T>
    concept _enable_if_string = is_string<T>::value;

    template<class T>
    concept _enable_if_c_str = 
        (std::is_pointer_v<T> && 
        _enable_if_char<std::remove_cv_t<std::remove_pointer_t<T>>>) ||
        (std::is_array_v<T> && 
        _enable_if_char<std::remove_cv_t<std::remove_extent_t<T>>>);

    template<class T>
    concept _enable_if_iterable = 
        std::ranges::range<T>   &&
        !_enable_if_string<T>   &&
        !_enable_if_map<T>      &&
        !_enable_if_optional<T> &&
        !_enable_if_has_print<T>&&
        !_enable_if_c_str<T>    &&
        !_enable_if_variant<T>;

    template<class T>
        requires _enable_if_has_print<T>
    struct _printer<T>{
        static void print(T const &t) {
            t.do_print();
        }
    };

    template<class T>
        requires _enable_if_iterable<T>
    struct _printer<T>{
        static void print(T const &t) {
            std::cout << "{";
            bool once = false;
            for (auto const &v: t) {
                if (once) {
                    std::cout << ", ";
                } else {
                    once = true;
                }
                _printer<std::remove_cvref_t<decltype(v)>>::print(v);
            }
            std::cout << "}";
        }
    };

    template<class T>
        requires _enable_if_tuple<T>
    struct _printer<T>{
        template <std::size_t ...Is>
        static void _unrolled_print(T const &t, std::index_sequence<Is...>) {
            std::cout << "{";
            ((_printer<std::remove_cvref_t<std::tuple_element_t<Is, T>>>::print(std::get<Is>(t)), std::cout << ", "), ...);
            if constexpr (sizeof...(Is) != 0) _printer<std::remove_cvref_t<std::tuple_element_t<sizeof...(Is), T>>>::print(std::get<sizeof...(Is)>(t));
            std::cout << "}";
        }
        static void print(T const &t) {
            _unrolled_print(t, std::make_index_sequence<std::max(static_cast<std::size_t>(1), std::tuple_size_v<T>) - 1>{});
        }
    };

    template<class T>
        requires _enable_if_map<T>
    struct _printer<T>{
        static void print(T const &t) {
            std::cout << "{";
            bool once = false;
            for (auto const &[k, v]: t) {
                if (once) {
                    std::cout << ", ";
                } else {
                    once = true;
                }
                _printer<std::remove_cvref_t<decltype(k)>>::print(k);
                std::cout << ": ";
                _printer<std::remove_cvref_t<decltype(v)>>::print(v);
            }
            std::cout << "}";
        }
    };

    template<class T>
        requires _enable_if_char<T>
    struct _printer<T> {
        static void print(T const& t) {
            T s[2] = {t, T('\0')};
            std::cout << std::quoted(s, T('\''));
        }
    };

    template<class T>
        requires _enable_if_string<T>
    struct _printer<T>{
        static void print(T const &t) {
            std::cout << std::quoted(t);
        }
    };

    template<class T>
        requires _enable_if_c_str<T>
    struct _printer<T>{
        static void print(T const &t) {
            std::cout << std::quoted(t);
        }
    };

    template <class T>
        requires _enable_if_optional<T>
    struct _printer<T> {
        static void print(T const &t) {
            if (t.has_value()) {
                _printer<typename T::value_type>::print(*t);
            } else {
                std::cout << "nullopt";
            }
        }
    };

    template<>
    struct _printer<std::monostate> {
        static void print(std::monostate const&) {
            std::cout << "monostate";
        }
    };

    template <class T>
        requires _enable_if_variant<T>
    struct _printer<T> {
        static void print(T const &t) {
            std::visit([] (auto const &v) {
                _printer<std::remove_cvref_t<decltype(v)>>::print(v);
            }, t);
        }
    };
    template <class T>
        requires std::is_same_v<T,bool>
    struct _printer<T> {
        static void print(bool const &t) {
            if (t) {
                std::cout << "true";
            } else {
                std::cout << "false";
            }
        }
    };

    template <class T0, class ...Ts>
    void print(T0 const &t0, Ts const &...ts) {
        _printer<std::remove_cvref_t<T0>>::print(t0);
        ((std::cout << " ", _printer<std::remove_cvref_t<Ts>>::print(ts)), ...);
        std::cout << "\n";
    }

    template <class T0, class ...Ts>
    void printnl(T0 const &t0, Ts const &...ts) {
        _printer<std::remove_cvref_t<T0>>::print(t0);
        ((std::cout << " ", _printer<std::remove_cvref_t<Ts>>::print(ts)), ...);
    }

    template<class T>
    concept _is_struct_printable = requires(std::ostream& os, T const& t) {
        os << t;
    };

    template <class T>
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
        requires _is_struct_printable<T>
    class print_adaptor<T> {
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

using _print_detials::print;
using _print_detials::printnl;
using _print_detials::print_adaptor;
#endif



