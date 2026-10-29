## 通过继承Mutex实现灵活的线程安全
使用`concept(c++ 20)`或者`type traits` ，通过模板创建一个Mutex，并将可能需要加锁的api进行包装
```cpp
template <Lockable Mutex> class basicSink : public sink {
public:
  basicSink();
  explicit basicSink(std::unique_ptr<details::formatter>);

  basicSink(const basicSink &) = delete;
  basicSink(basicSink &&) = delete;
  basicSink &operator=(const basicSink &) = delete;
  basicSink &operator=(basicSink &&) = delete;

  void log(const details::logmsg &) final override;
  void flush() final override;
  void sync() final override;
  void setPattern(const std::string &) final override;
  void setFormatter(std::unique_ptr<details::formatter>) final override;

  ~basicSink() {}

protected:
  virtual void flush_() = 0;
  virtual void sync_() = 0;
  virtual void setPattern_(const std::string &);
  virtual void setFormatter_(std::unique_ptr<details::formatter>);

  std::unique_ptr<details::formatter> m_formatter;
  Mutex m_mtx;
};
```

实现一个无效锁类，可以通过模板实现控制锁是否有效，需要加锁可以直接使用`std::mutex`
```cpp
struct NonMutex{
    void lock(){}
    void unlock(){}
    bool try_lock(){
      return false;
    }
};

template <class Mutex>
concept Lockable = requires(Mutex &mtx) {
  { mtx.lock() } -> std::same_as<void>;
  { mtx.unlock() } -> std::same_as<void>;
  { mtx.try_lock() } -> std::convertible_to<bool>;
};
```

再将api包装起来 ，从而实现灵活的单/多线程版本   
```cpp

template <Lockable Mutex>
void basicSink<Mutex>::log(const details::logmsg &msg) {
  std::lock_guard<Mutex> lock(m_mtx);
  sinkIt(msg);
}

template <Lockable Mutex> void basicSink<Mutex>::flush() {
  std::lock_guard<Mutex> lock(m_mtx);
  flush_();
}

template <Lockable Mutex> void basicSink<Mutex>::sync() {
  std::lock_guard<Mutex> lock(m_mtx);
  sync_();
}

template <Lockable Mutex>
void basicSink<Mutex>::setFormatter(
    std::unique_ptr<details::formatter> formatter) {
  std::lock_guard<Mutex> lock(m_mtx);
  setFormatter_(std::move(formatter));
}
```

填充对齐(Padding)也可以通过这种方式，灵活控制对齐与否
```cpp

class scopedPadder {
   public:
    scopedPadder(size_t wrapped_size, const paddingInfo &padinfo,
                 formatterBuf &dest)
        : m_padinfo(padinfo), m_dest(dest),m_truncate(dest.size()) {
        // 对齐大小 - 内容大小
        m_remainedPadding = static_cast<long>(padinfo.m_width) -
                            static_cast<long>(wrapped_size);
        if (m_remainedPadding <= 0) {
            return;
        }

        if (m_padinfo.m_side == paddingInfo::padSide::left) {
            pad_it(m_remainedPadding);
            m_remainedPadding = 0;
        } else if (m_padinfo.m_side == paddingInfo::padSide::center) {
            auto half_pad = ( m_remainedPadding + 1 )/ 2;  // 考虑可能截断
            auto reminder = m_remainedPadding & 1;  // 补上一个保证总宽度一致
            pad_it(half_pad);
            m_remainedPadding = half_pad + reminder;  // for the right side
        }
    }

    ~scopedPadder() {
        if (m_remainedPadding >= 0) {
            pad_it(m_remainedPadding);
        } else if (m_padinfo.m_truncate) {
            // 强制截断   remaining_pad是负数 强制截断
            long new_size = m_padinfo.m_width + m_truncate;
            if (new_size < 0) {
                new_size = 0;
            }
            m_dest.erase(new_size);
        }
    }

   private:
    void pad_it(long count) { m_dest.append(m_spaces.data(), count); }
    size_t m_truncate;
    const paddingInfo &m_padinfo;
    formatterBuf &m_dest;
    int m_remainedPadding;
    std::string_view m_spaces{
        "                                                                ", 64};
};

struct nullScopedPadder {
    nullScopedPadder(size_t /*wrapped_size*/, const paddingInfo & /*padinfo*/,
                     formatterBuf & /*dest*/) {}

    template <class T>
    static unsigned int count_digits(T /* number */) {
        return 0;
    }
};









template <class scopedPadder>
class millisecondsFormatter final : public flagFormatter {
   public:
    explicit millisecondsFormatter(paddingInfo padinfo)
        : flagFormatter(padinfo) {}
    void format(const logmsg &msg, formatterBuf &dest) override {
        // scopedPadder
        uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          msg.m_timeStamp.time_since_epoch())
                          .count();
        ms %= 1000;
        scopedPadder _(4, m_padInfo, dest);
        auto s = std::to_string(ms);
        dest.append('.' + std::string(3-s.size(),'0') + s);
    }
};
```