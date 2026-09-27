#include <source_location>
#include <string_view>
// 类型名提取

namespace{

template<class T>
constexpr std::string_view OriginSignature(){
    return std::source_location::current().function_name();
}

template<class T>
consteval std::string_view extractTypeName(){
    constexpr std::string_view signature = OriginSignature<T>();
    // void func() [with C = int]
    constexpr std::string_view marker = "T = ";
    constexpr size_t marker_pos = signature.find(marker);
    constexpr size_t begin = marker_pos + marker.size();
    constexpr size_t end = std::min(signature.find(';'),signature.find(']'));
    if constexpr(begin == std::string_view::npos || begin > end){
        return signature;
    }
    else return signature.substr(begin,end - begin);
}

template<class T>
inline constexpr std::string_view typeName = extractTypeName<T>();
}


template<class T>
consteval std::string_view TypeName(){
    return typeName<T>;
}

