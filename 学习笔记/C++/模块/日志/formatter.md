### 处理Padding
取自spdlog，通过局部对象自动构造和析构来实现
inlineBuffer取自pans::logger，优先构造栈上缓冲区`array`，溢出则采用堆上缓冲区`vector`
```CPP
template<size_t INLINE_CAPACITY>
class scopedPadder {
public:

    scopedPadder(size_t wrapped_size, const paddingInfo &padinfo, inlineBuffer<INLINE_CAPACITY> &dest)
        : m_padinfo(padinfo),
          m_dest(dest) {
        // 对齐大小 - 内容大小
        m_remainedPadding = static_cast<long>(padinfo.m_width) - static_cast<long>(wrapped_size);
        if (m_remainedPadding <= 0) {
            return;
        }

        if (m_padinfo.m_side == paddingInfo::padSide::left) {
            pad_it(m_remainedPadding);
            m_remainedPadding = 0;
        } else if (m_padinfo.m_side == paddingInfo::padSide::center) {
            auto half_pad = m_remainedPadding / 2;  // 考虑可能截断
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
            long new_size = static_cast<long>(m_dest.size()) + m_remainedPadding;
            if (new_size < 0) {
                new_size = 0;
            }
            m_dest.erase(new_size);
        }
    }

private:
    void pad_it(long count) {
        m_dest.append(m_padinfo,count);
    }

    const paddingInfo &m_padinfo;
    details::inlineBuffer<INLINE_CAPACITY> &m_dest;
    int m_remainedPadding;
    std::string_view m_spaces{"                                                                ", 64};
};

template<size_t INLINE_CAPACITY>
struct nullScopedPadder {
    nullScopedPadder(size_t /*wrapped_size*/,
                       const paddingInfo & /*padinfo*/,
                       inlineBuffer<INLINE_CAPACITY> & /*dest*/) {}

    template <typename T>
    static unsigned int count_digits(T /* number */) {
        return 0;
    }
};

template <typename ScopedPadder,size_t INLINE_CAPACITY>
class name_formatter final : public flagFormatter {
public:
    explicit name_formatter(paddingInfo padinfo)
        : flagFormatter(padinfo) {}

    /**
     * @brief 构造ScopedPadder 输出消息  析构ScopedPadder 实现padding
     * 
     * @param msg 
     * @param dest 
     */
    void format(const LOGMSG &msg, const std::tm &, inlineBuffer<INLINE_CAPACITY> &dest) override {
        ScopedPadder p(msg.m_loggerName.size(), m_padInfo, dest);
        dest.append();
    }
};


```