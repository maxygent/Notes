>对于简单的表达式(a + b * 3 - c),通过实现operator运算符重载来实现，具体来说，实现react\<Type\>的运算符重载，并捕获返回一个一个Expr类，可以实现链式的运算符计算，并且随着表达式的长度增加，需要嵌套的Expr会变得很深
```cpp
template <typename Op, typename L, typename R> class BinaryOpExpr {
// 表达式类的基础  捕获左右操作数和运算符，运算符通过类实现，具体
public:
  using ValueType =
      typename std::common_type_t<typename L::ValueType, typename R::ValueType>;
  template <typename Left, typename Right>
  BinaryOpExpr(Left &&l, Right &&r, Op o = Op{})
      : left(std::forward<Left>(l)), right(std::forward<Right>(r)), op(o) {}

  auto operator()() const { return calculate(); }

  operator ValueType() { return calculate(); }

private:
  auto calculate() const { return op(left(), right()); }
  L left;
  R right;
  [[no_unique_address]] Op op;
};
```

```cpp
struct AddOp {
  auto operator()(auto &&l, auto &&r) const { return l + r; }
};

struct MulOp {
  auto operator()(auto &&l, auto &&r) const { return l * r; }
};

struct SubOp {
  auto operator()(auto &&l, auto &&r) const { return l - r; }
};

struct DivOp {
  auto operator()(auto &&l, auto &&r) const { return l / r; }
};

template <typename T> struct ValueWrapper {
  using ValueType = T;
  T value;

  template <typename Type>
  ValueWrapper(Type &&t) : value(std::forward<Type>(t)) {}
  const T &operator()() const { return value; }
};

template <typename Op, typename L, typename R>
auto make_binary_expr(L &&l, R &&r) {
  return BinaryOpExpr<Op, ExprWrapper<std::decay_t<L>>,
                      ExprWrapper<std::decay_t<R>>>(std::forward<L>(l),
                                                    std::forward<R>(r));
}

template <typename L, typename R>
  requires HasCustomOp<L, R>
auto operator+(L &&l, R &&r) {
  return make_binary_expr<AddOp>(std::forward<L>(l), std::forward<R>(r));
}

template <typename L, typename R>
  requires HasCustomOp<L, R>
auto operator*(L &&l, R &&r) {
  return make_binary_expr<MulOp>(std::forward<L>(l), std::forward<R>(r));
}

template <typename L, typename R>
  requires HasCustomOp<L, R>
auto operator-(L &&l, R &&r) {
  return make_binary_expr<SubOp>(std::forward<L>(l), std::forward<R>(r));
}

template <typename L, typename R>
  requires HasCustomOp<L, R>
auto operator/(L &&l, R &&r) {
  return make_binary_expr<DivOp>(std::forward<L>(l), std::forward<R>(r));
}
```