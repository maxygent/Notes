>响应式数据（Reactive Data） 指的是：当数据源发生变化时，依赖它的计算结果能够自动重新计算并更新，而不需要手动逐个调用计算函数。


>对于复杂表达式类(通过lambda实现)，接受参数的时候保存为引用，并且可能存在多个不同的类型，因此选择使用tuple来保存所有参数的引用
```cpp
template<typename Func,typename...Args>
class expression{
	template<typename F, typename...A>
    Expression(F&& func,A&&...args):m_func(std::forward<F>(func)),m_args(std::forward<A>(args)...),Resource<ReturnType<Func,Args...>>(){
        this->updateObservers([this]{valueChanged();},std::forward<A>(args)...);
        evaluate();
    }
	Func m_func;
	std::tuple<std::reference_wrapper<Args>...> m_args;
};
```
>其中，由于接受的是引用，所以对于tuple的模板需要reference_wrapper包裹，并且再获取的时候需要额外一次get才可以得到内部的Args类型(坑)
其次，可以用std::apply来方便的对tuple对象进行操作，省去一次解包


>关于类型萃取(TypeTraits),方式通常比较死板，都是一个基础模板，然后在此基础上实现特例化版本
```cpp
template<typename T>
struct ExpressionTraits{
    using Type = T;
};


template<typename T>
struct ExpressionTraits<DataSource<T>>{
    using Type = T;
};

template<typename Func,typename...Args>
struct ExpressionTraits<DataSource<Func,Args...>>{
    using Type = std::invoke_result_t<Func,typename ExpressionTraits<Args>::Type...>;
};


template<typename T, typename...Args>
using ReturnType = typename ExpressionTraits<DataSource<T,Args...>>::Type;
```

>关于<=>运算符重载，规则如下

|   已定义的运算符    | 其他运算符是否可用（C++20） |
| :----------: | :--------------: |
|      ==      |       !=可用       |
|      !=      |      ==不可用       |
|    自定义<=>    |     ==和!=不可用     |
| <=>default实现 |     ==和!=均可以     |
>default生成的按照成员变量的声明顺序比较

>响应式数据的响应方式：对于一个依赖于其他数据的数据，比如某个表达式，在其构造的时候对于每个将本身的更改方法(通知)传递给每个依赖
```cpp
// expression类的构造
    template<typename F, typename...A>
    Expression(F&& func,A&&...args):m_func(std::forward<F>(func)),m_args(std::forward<A>(args)...),Resource<ReturnType<Func,Args...>>(){
        this->updateObservers([this]{valueChanged();},std::forward<A>(args)...);
        evaluate();
    }
// valueChanged就是计算一次表达式


// 对于每个数据源，其每次更新值的时候需要通知依赖于他的变量或表达式更新
    template <typename... Args>
    void updateObservers(const std::function<void()> &f, Args &&...args)
    {
        (..., args.addObserver(f));
        // 给每个数据源提供了更新依赖于他的变量的计算方法
    }

    void notify()
    {
        for (auto &observer : m_observers)
        {
            observer();
        }
    }


```