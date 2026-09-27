# C++ 语法与特性总览（C++98 → C++20）

> 按标准版本划分，每个特性附简短示例说明作用与用法。
> 编译建议：`g++ -std=c++20 main.cpp`（向下兼容各版本特性）

---

## 目录

1. [C++98 / C++03 —— 语言基石](#一c98--c03--语言基石)
2. [C++11 —— 现代C++的开端](#二c11--现代c的开端)
3. [C++14 —— 渐进式改进](#三c14--渐进式改进)
4. [C++17 —— 实用主义升级](#四c17--实用主义升级)
5. [C++20 —— 四大新特性与全面进化](#五c20--四大新特性与全面进化)

---

## 一、C++98 / C++03 —— 语言基石

C++98 确立了语言核心，C++03 主要是缺陷修复（如 `auto_ptr` 语义微调）。

### 1.1 类与封装

```cpp
class Point {
private:
    int x, y;                       // 私有成员，隐藏实现细节
public:
    Point(int x_, int y_) : x(x_), y(y_) {}  // 构造函数 + 初始化列表

    int getX() const { return x; }  // const 成员函数：不修改成员
    void move(int dx, int dy) { x += dx; y += dy; }
};

Point p(1, 2);
p.move(3, 0);
```

### 1.2 构造/析构与 RAII

RAII：资源获取即初始化，对象析构时自动释放资源——C++ 管理资源的核心思想。

```cpp
class FileHandle {
public:
    FileHandle(const char* path) { f_ = std::fopen(path, "r"); }
    ~FileHandle() { if (f_) std::fclose(f_); }   // 析构时自动释放
    // 禁止拷贝，防止双重释放
private:
    std::FILE* f_;
};
// 离开作用域时自动关闭文件，即使抛异常也能释放
```

### 1.3 继承与多态（虚函数）

```cpp
class Animal {
public:
    virtual void speak() const { std::cout << "..."; }
    virtual ~Animal() = default;    // 基类析构必须为虚！
};
class Dog : public Animal {
public:
    void speak() const override { std::cout << "Woof\n"; }
};

Animal* a = new Dog();
a->speak();   // 输出 Woof —— 运行时根据实际类型调用（动态绑定）
delete a;
```

### 1.4 运算符重载

```cpp
class Vec2 {
public:
    float x, y;
    Vec2(float x_, float y_) : x(x_), y(y_) {}
    Vec2 operator+(const Vec2& rhs) const {
        return Vec2(x + rhs.x, y + rhs.y);
    }
};

Vec2 v = Vec2(1, 2) + Vec2(3, 4);   // v = (4, 6)
```

### 1.5 模板（泛型编程）

```cpp
template <typename T>
T maxOf(T a, T b) {
    return a > b ? a : b;
}

maxOf(3, 7);        // T = int，编译器自动推导
maxOf(1.5, 2.5);    // T = double
```

### 1.6 类模板与 STL 容器

```cpp
#include <vector>
#include <map>
#include <string>

std::vector<int> nums;                       // 动态数组
nums.push_back(10);
nums.push_back(20);

std::map<std::string, int> ages;             // 有序键值对（红黑树）
ages["alice"] = 30;

std::string s = "hello";
s += " world";
```

### 1.7 STL 算法与迭代器

```cpp
#include <algorithm>

std::vector<int> v{5, 3, 8, 1};
std::sort(v.begin(), v.end());               // 排序：1 3 5 8
auto it = std::find(v.begin(), v.end(), 3);  // 查找
int total = std::accumulate(v.begin(), v.end(), 0);  // 求和（<numeric>）
```

### 1.8 异常处理

```cpp
try {
    throw std::runtime_error("出错了");
} catch (const std::exception& e) {   // 按引用捕获
    std::cout << e.what() << '\n';
}
```

### 1.9 命名空间

```cpp
namespace physics {
    constexpr double g = 9.8;
    void fall() { /* ... */ }
}

physics::fall();          // 作用域解析运算符 ::
using physics::g;         // 引入单个名字
```

### 1.10 引用

```cpp
int a = 10;
int& ref = a;       // 左值引用：ref 是 a 的别名
ref = 20;           // a 变为 20

void incr(int& x) { ++x; }   // 传引用避免拷贝、可修改实参
incr(a);
```

### 1.11 函数重载与默认参数

```cpp
void log(const std::string& msg);                  // 重载 1
void log(const std::string& msg, int level);       // 重载 2
void print(int x, int base = 10);                  // 默认参数
print(255);        // 十进制
print(255, 16);    // 十六进制
```

### 1.12 auto_ptr（C++03，已被废弃）

```cpp
// 旧式智能指针，拷贝时会转移所有权，语义易出错
std::auto_ptr<int> p(new int(5));   // C++11 起废弃，改用 unique_ptr
```

---

## 二、C++11 —— 现代C++的开端

C++11 是一次革命性更新，通常被视为"现代 C++"的起点。

### 2.1 auto 类型推导

```cpp
auto i = 42;               // int
auto d = 3.14;             // double
auto it = nums.begin();    // 替代冗长的 std::vector<int>::iterator
```

### 2.2 范围 for 循环

```cpp
std::vector<int> v{1, 2, 3};
for (int x : v)            // 值拷贝
    std::cout << x << ' ';
for (auto& x : v)          // 引用，可修改元素
    x *= 2;
for (const auto& x : v)    // 只读引用，避免拷贝（最常用）
    std::cout << x << ' ';
```

### 2.3 nullptr

```cpp
int* p = nullptr;          // 类型安全的空指针
// int x = nullptr;        // 编译错误！而 NULL（即 0）可以隐式转 int，是隐患
void f(int);
void f(int*);
f(nullptr);                // 明确调用 f(int*)，NULL 会有歧义
```

### 2.4 Lambda 表达式

```cpp
// [捕获列表](参数) -> 返回类型 { 函数体 }
auto add = [](int a, int b) { return a + b; };
add(1, 2);

int factor = 3;
auto mul = [factor](int x) { return x * factor; };   // 按值捕获
auto mul2 = [&factor](int x) { return x * factor; }; // 按引用捕获
auto all = [=](int x) { return x * factor; };        // 捕获全部（值）
auto allRef = [&](int x) { return x * factor; };     // 捕获全部（引用）

// 典型用法：作为算法谓词
std::sort(v.begin(), v.end(), [](int a, int b) { return a > b; });
```

### 2.5 右值引用与移动语义

```cpp
std::string a = "hello, a very long string...";
std::string b = std::move(a);   // 移动：直接"偷走"a的内部缓冲区，O(1)
// 此后 a 处于有效但未定义状态，不要再使用它的值

void process(std::string s);
process(std::move(b));          // 避免深拷贝
```

### 2.6 智能指针（memory 头文件）

```cpp
#include <memory>

auto up = std::make_unique<int>(42);   // 独占所有权（C++14 起才有 make_unique）
std::unique_ptr<int> up2(new int(42));

auto sp = std::make_shared<int>(10);   // 共享所有权，引用计数
std::shared_ptr<int> sp2 = sp;         // 计数 +1

std::weak_ptr<int> wp = sp;            // 弱引用，不增加计数，解决循环引用
if (auto locked = wp.lock())           // 使用前需 lock() 提升为 shared_ptr
    std::cout << *locked << '\n';
```

### 2.7 初始化列表与统一初始化

```cpp
std::vector<int> v{1, 2, 3};           // 花括号初始化
int arr[]{1, 2, 3};
std::map<std::string, int> m{{"a", 1}, {"b", 2}};

class Widget {
public:
    Widget(std::initializer_list<int> list) { /* ... */ }
};
Widget w{1, 2, 3};                     // 自定义类型也能接收任意个初始化值
```

### 2.8 enum class（强类型枚举）

```cpp
enum class Color { Red, Green, Blue };
Color c = Color::Red;
// int x = c;           // 编译错误！不会隐式转换为整数（旧 enum 会）
int y = static_cast<int>(c);   // 必须显式转换
enum class Status : uint8_t { Ok, Error };   // 可指定底层类型
```

### 2.9 constexpr（编译期常量/计算）

```cpp
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}
constexpr int f5 = factorial(5);    // 编译期算出 120，存入只读段
int arr[factorial(4)];              // 可用作数组大小
```

### 2.10 static_assert（编译期断言）

```cpp
static_assert(sizeof(int) == 4, "需要32位int");
static_assert(factorial(3) == 6);   // C++17 起消息可省略
```

### 2.11 override 与 final

```cpp
struct Base { virtual void run() = 0; virtual ~Base() = default; };
struct Derived : Base {
    void run() override;    // 显式标记重写，签名不匹配会直接报错
};
struct Sealed final {};     // final：禁止被继承
// struct X : Sealed {};    // 编译错误
```

### 2.12 变参模板

```cpp
// 递归展开任意数量的模板参数
void print() {}                        // 终止条件
template <typename T, typename... Args>
void print(T first, Args... rest) {
    std::cout << first << ' ';
    print(rest...);                    // 递归展开剩余参数
}
print(1, "hello", 3.14, 'c');
```

### 2.13 default 与 delete

```cpp
class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;             // 禁止拷贝
    NonCopyable& operator=(const NonCopyable&) = delete;
};
// NonCopyable a, b = a;  // 编译错误
```

### 2.14 委托构造函数

```cpp
class Rect {
public:
    Rect() : Rect(0, 0) {}              // 委托给另一个构造函数
    Rect(int w, int h) : w_(w), h_(h) {}
private:
    int w_, h_;
};
```

### 2.15 类型工具：decltype / alias

```cpp
int x = 0;
decltype(x) y = 1;                     // y 的类型与 x 相同（int）

using IntVec = std::vector<int>;       // 类型别名（替代 typedef，支持模板）
template <typename T>
using Vec = std::vector<T>;            // 模板别名，typedef 做不到
Vec<double> dv;
```

### 2.16 noexcept

```cpp
void safeFunc() noexcept { }           // 承诺不抛异常
// 若 noexcept 函数内抛出异常，程序直接 terminate
void oldSwap(int* a, int* b) noexcept { std::swap(*a, *b); }
```

### 2.17 用户自定义字面量

```cpp
constexpr long double operator""_km(long double x) { return x * 1000.0; }
auto meters = 1.5_km;                  // 1500.0 米
using namespace std::literals;
auto s = "abc"s;                       // std::string
auto t = 10min;                        // std::chrono::minutes
```

### 2.18 新标准库：线程、chrono、tuple 等

```cpp
#include <thread>
#include <chrono>
#include <tuple>
#include <atomic>
#include <mutex>
#include <future>

// 线程
std::thread t([]{ std::cout << "并发执行\n"; });
t.join();

// 原子变量（无锁线程安全）
std::atomic<int> counter{0};
++counter;

// 互斥锁 + RAII 锁管理
std::mutex m;
{
    std::lock_guard<std::mutex> lock(m);   // 构造加锁，析构解锁
}

// future / async 异步任务
auto fut = std::async(std::launch::async, []{ return 42; });
int result = fut.get();

// chrono 时间库
using namespace std::chrono;
auto start = steady_clock::now();
std::this_thread::sleep_for(100ms);
auto dur = duration_cast<milliseconds>(steady_clock::now() - start);

// tuple 元组
std::tuple<int, std::string, double> item{1, "apple", 3.5};
auto [id, name, price] = item;            // C++17 结构化绑定更方便
```

### 2.19 其他容器与工具

```cpp
#include <array>
#include <unordered_map>
#include <functional>

std::array<int, 3> arr{1, 2, 3};          // 定长数组，栈上，带 STL 接口
std::unordered_map<std::string, int> um;  // 哈希表，O(1) 查找
um["key"] = 1;

std::function<int(int)> f = [](int x) { return x * 2; };  // 通用可调用包装
auto bound = std::bind(f, std::placeholders::_1);
```

### 2.20 其他语言改进速览

| 特性 | 示例 | 说明 |
|---|---|---|
| `char16_t` / `char32_t` | `char16_t c = u'你';` | UTF-16/32 字符 |
| 原始字符串 | `R"(C:\path\no_escape)"` | 反斜杠不转义 |
| `>>` 嵌套模板闭合 | `vector<vector<int>>` | 不再需要 `> >` 空格 |
| 成员初始化 | `int x = 0;`（类内） | 声明处直接给默认值 |
| `using` 引入基类成员 | `using Base::f;` | 继承重载集 |
| 对齐 | `alignas(16) char buf[64];` | 控制对齐 |
| `long long` | `long long x = 10LL;` | 至少 64 位整型 |

---

## 三、C++14 —— 渐进式改进

### 3.1 泛型 Lambda（auto 参数）

```cpp
auto add = [](auto a, auto b) { return a + b; };  // 参数类型自动推导
add(1, 2);        // int
add(1.5, 2.5);    // double
```

### 3.2 返回类型推导

```cpp
auto sum(int a, int b) {          // 函数返回类型自动推导
    return a + b;                 // int
}
```

### 3.3 放宽的 constexpr（允许循环和分支）

```cpp
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i)  // C++11 不允许循环，C++14 可以
        result *= i;
    return result;
}
constexpr int f = factorial(6);   // 编译期求值 720
```

### 3.4 变量模板

```cpp
template <typename T>
constexpr T pi = T(3.1415926535897932385);

pi<float>;       // float 精度
pi<double>;      // double 精度
double r = pi<double> * 2.0;
```

### 3.5 二进制字面量与数字分隔符

```cpp
int mask = 0b1010'0101;     // 二进制字面量
long big = 1'000'000'000;   // 单引号分隔，提高可读性
```

### 3.6 std::make_unique

```cpp
auto p = std::make_unique<std::vector<int>>(3, 0);  // 安全构造 unique_ptr
// 避免裸 new，且异常安全
```

### 3.7 [[deprecated]] 属性

```cpp
[[deprecated("请改用 newFunc()")]]
void oldFunc() {}

// 调用处会产生编译警告
```

### 3.8 其他

```cpp
// std::exchange：换出新值，返回旧值
int x = 5;
int old = std::exchange(x, 10);   // old=5, x=10

// constexpr 函数中可修改局部变量、lambda 可在 constexpr 中使用
// 数字分隔符见 3.5
```

---

## 四、C++17 —— 实用主义升级

### 4.1 结构化绑定

```cpp
std::map<std::string, int> ages{{"alice", 30}, {"bob", 25}};

for (const auto& [name, age] : ages)     // 一行拆出 key 和 value
    std::cout << name << ": " << age << '\n';

std::pair<int, std::string> p{1, "one"};
auto [num, str] = p;                     // num=1, str="one"

int arr[]{1, 2, 3};
auto [a, b, c] = arr;                    // 数组也能拆
```

### 4.2 if constexpr（编译期分支）

```cpp
template <typename T>
auto getValue(T t) {
    if constexpr (std::is_pointer_v<T>)   // 编译期裁剪，未选中的分支不实例化
        return *t;
    else
        return t;
}
// 模板元编程的利器，替代大量 SFINAE
```

### 4.3 if / switch 带初始化语句

```cpp
if (auto it = m.find("alice"); it != m.end()) {   // 变量作用域仅限于 if
    std::cout << it->second << '\n';
}
// it 在此处不可见，命名空间更干净

switch (int code = getStatusCode(); code) {
    case 200: /* ... */ break;
    case 404: /* ... */ break;
}
```

### 4.4 折叠表达式

```cpp
template <typename... Args>
auto sum(Args... args) {
    return (args + ...);              // 一元右折叠：a + (b + (c + d))
}
sum(1, 2, 3, 4);                      // 10

template <typename... Args>
void printAll(Args... args) {
    ((std::cout << args << ' '), ...);   // 逗号折叠，逐个打印
}
```

### 4.5 CTAD（类模板实参推导）

```cpp
std::pair p{1, 2.5};                  // 自动推导为 pair<int, double>
std::vector v{1, 2, 3};               // vector<int>，不用手写 <int>
std::lock_guard lock(mtx);            // 推导出 lock_guard<mutex>
```

### 4.6 内联变量 inline variable

```cpp
// 头文件中定义全局变量，多个翻译单元包含也不会链接冲突
struct Config {
    static inline int maxRetry = 3;   // C++17 之前只能声明+类外定义
};
inline constexpr double kG = 9.8;     // 头文件里的全局常量
```

### 4.7 std::optional —— 可能没有值

```cpp
#include <optional>

std::optional<int> findIndex(const std::vector<int>& v, int target) {
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i] == target) return static_cast<int>(i);
    return std::nullopt;              // 明确表达"无值"
}

if (auto idx = findIndex(v, 5); idx)  // 有值时为 true
    std::cout << *idx << '\n';        // 解引用取值
else
    std::cout << "未找到\n";
// 比"返回 -1 表示失败"或输出参数更安全清晰
```

### 4.8 std::variant —— 类型安全的联合

```cpp
#include <variant>

std::variant<int, std::string> data;
data = 42;
data = "hello";                       // 当前持有 string

if (auto p = std::get_if<std::string>(&data))       // 类型检查后访问
    std::cout << *p << '\n';

std::visit([](auto&& v) { std::cout << v << '\n'; }, data);  // 模式访问
// 替代 C 风格 union，类型安全，不会读错成员
```

### 4.9 std::any —— 任意类型容器

```cpp
#include <any>

std::any a = 42;
a = std::string("hello");
if (a.type() == typeid(std::string)) {
    auto& s = std::any_cast<std::string&>(a);   // 取回需精确匹配类型
}
```

### 4.10 std::string_view —— 轻量字符串视图

```cpp
#include <string_view>

// 不拥有字符串，只是"指针+长度"，零拷贝传参
int count(std::string_view sv, char c) {        // 接受 string/字面量都不拷贝
    return static_cast<int>(std::count(sv.begin(), sv.end(), c));
}
count("hello world", 'l');

std::string_view sv = "abcdef";
sv.substr(2, 3);                       // 切片无拷贝
// 注意：不要让 string_view 指向临时字符串！
```

### 4.11 文件系统库 filesystem

```cpp
#include <filesystem>
namespace fs = std::filesystem;

fs::path dir = "logs";
fs::create_directories(dir);           // 递归建目录
for (const auto& entry : fs::directory_iterator(dir))
    std::cout << entry.path() << '\n'; // 遍历目录
auto size = fs::file_size("a.txt");
bool exists = fs::exists("a.txt");
fs::copy("src.txt", "dst.txt");
```

### 4.12 并行算法

```cpp
#include <execution>

std::vector<int> big(10'000'000);
// 多线程并行排序/求和等，只需加一个执行策略参数
std::sort(std::execution::par, big.begin(), big.end());
std::for_each(std::execution::par_unseq, big.begin(), big.end(),
              [](int& x) { x *= 2; });
```

### 4.13 保证的拷贝消除

```cpp
struct Widget { Widget() = default; Widget(const Widget&) = delete; };
Widget makeWidget() { return Widget{}; }   // C++17：返回纯右值不再调用拷贝/移动构造
Widget w = makeWidget();                    // 即使拷贝构造是 delete 也能编译
```

### 4.14 其他 C++17 特性速览

```cpp
// std::byte —— 明确的字节类型
std::byte b{0xFF};

// [[nodiscard]] —— 返回值不可忽略
[[nodiscard]] int compute();     // 调用时写 compute(); 会警告

// [[maybe_unused]] —— 抑制未使用警告
[[maybe_unused]] int debugVar = 0;

// [[fallthrough]] —— 明确的 switch 贯穿
switch (x) {
    case 1:
        prepare();
        [[fallthrough]];         // 告诉编译器这是故意的
    case 2:
        run();
        break;
}

// 嵌套命名空间
namespace app::core::net { }     // 等价于三层嵌套

// utf-8 字面量
auto u8s = u8"你好";

// if constexpr 见 4.2；constexpr lambda 也已支持
```

---

## 五、C++20 —— 四大新特性与全面进化

C++20 四大特性：**概念（Concepts）、范围（Ranges）、协程（Coroutines）、模块（Modules）**。

### 5.1 Concepts —— 对模板参数的约束

```cpp
#include <concepts>

// 定义概念：约束"必须是数字类型"
template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// requires 子句约束模板
template <Numeric T>
T twice(T x) { return x * 2; }

// requires 表达式写法
template <typename T>
requires requires(T a, T b) { a + b; }     // 要求 a+b 合法
T add(T a, T b) { return a + b; }

// 标准 concepts（<concepts>）
template <typename T>
concept Container = requires(T c) {
    c.begin();          // 必须有 begin()
    c.end();            // 必须有 end()
    typename T::value_type;
};

static_assert(Numeric<int>);        // 编译期检查类型是否满足
// twice("str");                    // 编译错误，且报错信息清晰指出约束不满足
```

### 5.2 Ranges —— 声明式管道操作

```cpp
#include <ranges>
#include <vector>

namespace rv = std::views;
namespace rg = std::ranges;

std::vector<int> nums{1, 2, 3, 4, 5, 6, 7, 8};

// 视图：惰性求值，零拷贝，管道语法
auto result = nums
    | rv::filter([](int n) { return n % 2 == 0; })   // 留偶数
    | rv::transform([](int n) { return n * n; })     // 平方
    | rv::take(3);                                    // 取前3个
// 结果：4 16 36 —— 不产生中间容器

for (int x : result)
    std::cout << x << ' ';

rg::sort(nums);                        // 直接对容器排序，不用传迭代器
rg::sort(nums, rg::greater{});         // 降序
auto it = rg::find(nums, 4);           // 容器版算法
std::cout << rg::count_if(nums, [](int n){ return n > 3; });

// iota：生成无限序列
for (int x : rv::iota(1) | rv::take(5))
    std::cout << x << ' ';             // 1 2 3 4 5
```

### 5.3 协程（Coroutines）

```cpp
#include <coroutine>

// 一个最小的生成器：函数可以"暂停"并逐个产出值
template <typename T>
struct Generator {
    struct promise_type {
        T current_;
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T v) { current_ = v; return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h_;
    explicit Generator(std::coroutine_handle<promise_type> h) : h_(h) {}
    ~Generator() { if (h_) h_.destroy(); }

    bool next() { h_.resume(); return !h_.done(); }
    T value() { return h_.promise().current_; }
};

Generator<int> counter(int start, int end) {
    for (int i = start; i < end; ++i)
        co_yield i;                    // 暂停点：产出一个值并挂起
    co_return;                          // 结束协程
}

int main() {
    auto gen = counter(0, 3);
    while (gen.next())
        std::cout << gen.value() << ' ';   // 输出 0 1 2
}
// 三个关键字：co_await（等待）、co_yield（产出）、co_return（返回）
```

### 5.4 模块（Modules）

```cpp
// math.cppm —— 模块文件
export module math;                 // 声明模块名

export int add(int a, int b) {      // export：对外可见
    return a + b;
}

int internalHelper() {              // 未 export：模块私有
    return 0;
}

// main.cpp —— 使用模块
import math;                        // 替代 #include
int main() { return add(1, 2); }
// 编译：g++ -std=c++20 -c math.cppm   （需较新的编译器支持）
```

### 5.5 三路比较运算符 <=>

```cpp
#include <compare>

struct Version {
    int major, minor, patch;

    auto operator<=>(const Version&) const = default;   // 一行搞定全部比较
    // 自动生成 == != < <= > >= 六个运算符
};

Version a{1, 2, 0}, b{1, 3, 0};
bool newer = a > b;                 // false

// 也可以手动实现
std::strong_ordering r = 10 <=> 20; // r < 0
```

### 5.6 指定初始化器（Designated Initializers）

```cpp
struct WindowConfig {
    int width = 800;
    int height = 600;
    bool fullscreen = false;
    std::string title = "App";
};

WindowConfig cfg{
    .width = 1280,                  // 指名道姓地初始化，其余用默认值
    .fullscreen = true,
};
// 注意：必须按声明顺序填写，C++ 比 C 更严格
```

### 5.7 std::format —— 现代格式化

```cpp
#include <format>

std::string s = std::format("姓名: {}, 年龄: {}", "小明", 18);
std::string t = std::format("{:.2f} / {:>8}", 3.14159, 42);   // "3.14 /      42"
std::string u = std::format("{1} {0} {1}", "A", "B");         // 位置参数："B A B"

// C++23 起才有 std::print，C++20 用 format + cout
std::cout << std::format("x = {}\n", 10);
```

### 5.8 constexpr / consteval / constinit 大扩展

```cpp
// consteval：立即函数，只能编译期调用
consteval int square(int x) { return x * x; }
constexpr int s = square(5);        // OK，编译期求值
// int y = square(runtimeValue);    // 编译错误！禁止运行时调用

// constinit：保证编译期初始化，但值可运行时修改
constinit int counter = 0;
void tick() { ++counter; }          // OK：不是 const，可以改

// constexpr 能用 try/catch、虚函数、动态分配（C++20 大幅放宽）
constexpr auto makeVec() {
    std::vector<int> v{1, 2, 3};    // constexpr vector！
    v.push_back(4);
    return v;
}
```

### 5.9 std::span —— 连续内存的视图

```cpp
#include <span>

// 不拷贝地"借用"任何连续内存，统一接口
int process(std::span<const int> data) {        // 接受 vector/array/裸数组
    int sum = 0;
    for (int x : data) sum += x;
    return sum;
}

std::vector<int> v{1, 2, 3};
int arr[]{4, 5, 6};
process(v);                        // OK
process(arr);                      // OK
process({arr, 3});                 // 指针+长度也行
```

### 5.10 jthread 与线程协作工具

```cpp
#include <thread>
#include <stop_token>
#include <latch>
#include <barrier>
#include <semaphore>

// jthread：析构自动请求停止并 join，不泄漏
std::jthread worker([](std::stop_token st) {
    while (!st.stop_requested()) {       // 响应取消请求
        std::this_thread::sleep_for(100ms);
    }
});
worker.request_stop();                   // 请求停止

// latch：一次性倒计数门闩
std::latch done{3};
// 三个线程各调用 done.count_down();
done.wait();                             // 等三个都完成

// barrier：可重复使用的同步屏障（多轮迭代）
std::barrier sync{3};

// semaphore：信号量，限制并发数
std::counting_semaphore<4> slots{4};     // 最多4个同时进入
slots.acquire();                         // P 操作
slots.release();                         // V 操作
```

### 5.11 原子等待与 chrono 增强

```cpp
#include <atomic>

std::atomic<int> flag{0};
// 等待者：阻塞直到值变化（比忙等高效，比条件变量轻量）
flag.wait(0);                            // 值仍为 0 则休眠
// 另一个线程：
flag.store(1);
flag.notify_all();                       // 唤醒等待者

// 日历与时区
#include <chrono>
using namespace std::chrono;
auto today = year{2026} / August / 30;               // 年月日
auto tp = sys_days{today} + 12h + 30min;             // 具体时间点
std::cout << tp << '\n';                             // 直接输出时间
auto now = zoned_time{current_zone(), system_clock::now()};  // 时区支持
```

### 5.12 其他 C++20 特性速览

```cpp
// char8_t —— 独立的 UTF-8 字符类型
const char8_t* u8str = u8"你好";

// lambda 模板参数
auto getSize = []<typename C>(const C& c) { return c.size(); };
getSize(std::vector{1,2,3});

// 位操作库 <bit>
#include <bit>
std::cout << std::popcount(0b1011u);       // 3（1的个数）
std::cout << std::bit_width(0b1000u);      // 4（所需位数）
float f = std::bit_cast<float>(0x4048F5C3u); // 位级重解释，比 memcpy 干净

// starts_with / ends_with
std::string s = "config.json";
s.starts_with("config");                   // true
s.ends_with(".json");                      // true

// 数学工具
#include <numeric>
std::midpoint(1, 9);                       // 5，中点（防溢出）
std::lerp(0.0, 10.0, 0.25);                // 2.5，线性插值

// 数组/向量也可以比较大小（三路比较的延伸）
std::vector<int> a{1,2}, b{1,3};
bool smaller = a < b;                      // true，字典序

// source_location —— 获取调用位置
#include <source_location>
void log(const char* msg,
         const std::source_location& loc = std::source_location::current()) {
    std::cout << loc.file_name() << ":" << loc.line() << " " << msg << '\n';
}

// synchronized ostream（多线程输出不再交错）
#include <syncstream>
std::osyncstream out{std::cout};
out << "线程安全输出\n";
```

---

## 附：版本特性对照表

| 版本 | 年份 | 核心关键词 |
|---|---|---|
| C++98/03 | 1998/2003 | 类、模板、STL、异常、RAII |
| C++11 | 2011 | auto、lambda、智能指针、移动语义、constexpr、线程库 |
| C++14 | 2014 | 泛型 lambda、放宽 constexpr、make_unique |
| C++17 | 2017 | 结构化绑定、if constexpr、optional/variant、string_view、filesystem、并行算法 |
| C++20 | 2020 | Concepts、Ranges、协程、模块、`<=>`、format、span、jthread |

## 附：编译命令参考

```bash
g++ -std=c++20 main.cpp          # GCC 10+（完整特性建议 GCC 12+）
clang++ -std=c++20 main.cpp      # Clang 14+（模块建议 Clang 16+）
cl /std:c++20 main.cpp           # MSVC 2019 16.8+，对 C++20 支持最完整
```
