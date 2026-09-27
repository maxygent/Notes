

### 日志模块

- **appender**:决定日志输出位置的对象，比如文件，stdout等

- **logger**:日志器核心，管理若干appender对象

- **formatter**:用于日志格式化

- **日志宏**:方便用户使用

- **loglevel**:日志等级

- **日志缓冲区(可自行实现)**:如果使用stringstream则默认全都在堆上分配内存，自定义缓冲区streambuf可以实现部分在栈上分配

- **异步输出**:设置额外线程和日志缓冲区，不阻塞主线程的同时输出日志

appender实现日志的具体输出，logger根据日志等级将格式化的日志信息传递给各个appender，用户最终通过宏来使用日志模块

### formatter具体实现：

formatter内嵌formatItem，根据格式化字符串决定各个部分的输出方式和输出顺序

```cpp
class Formatter final
{
public:
    class FormatItem
    {
    public:
        virtual ~FormatItem() = default;
        virtual void format(const LogRecordView& record, FormattedRecordBuffer& output) const = 0;
    };
    
    explicit Formatter(std::string_view pattern);
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const;

    [[nodiscard]] const std::string& getPattern() const noexcept { return m_pattern; }

private:
    int parse();
    void addLiteral(std::string& literal);

private:
    std::string m_pattern;
    std::vector<std::unique_ptr<FormatItem>> m_items;
};




constexpr std::string_view DEFAULT_DATE_FORMAT = "%Y-%m-%d %H:%M:%S";

Formatter::Formatter(std::string_view pattern)
    : m_pattern(pattern)
{
    if(parse() != 0)
    {
        throw std::invalid_argument("invalid logger format pattern");
    }
}

void Formatter::format(const LogRecordView& record, FormattedRecordBuffer& output) const
{
    for(const auto& item : m_items)
    {
        item->format(record, output);
    }
}

template <typename T>
void AppendInteger(FormattedRecordBuffer& output, T value)
{
    std::array<char, 24> buffer{};   //64位数据加上负号最多可以到20个字符
    const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value); // 基本上不会失败，转失败了也没关系
    ASSERT_RETNONE2(result.ec == std::errc(), "trans " << value << " to chars failed.");
    output.append(buffer.data(), static_cast<std::size_t>(result.ptr - buffer.data()));
}

class LiteralFormatItem final : public Formatter::FormatItem
{
public:
    explicit LiteralFormatItem(std::string value)
        : m_value(std::move(value))
    {}
    void format(const LogRecordView&, FormattedRecordBuffer& output) const override
    {
        output.append(m_value);
    }

private:
    std::string m_value;
};   //固定文字不改变 直接用最开始记录的就行

class MessageFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        output.append(record.m_message);
    }
};

class LevelFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        output.append(LogLevel::ToString(record.m_level));
    }
};

class ElapsedFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        AppendInteger(output, std::chrono::duration_cast<std::chrono::milliseconds>(record.m_elapsed).count());
    }
};

class LoggerNameFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        output.append(record.m_loggerName);
    }
};

class ThreadIdFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        AppendInteger(output, record.m_threadId);
    }
};

class NewLineFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView&, FormattedRecordBuffer& output) const override
    {
        output.append('\n');
    }
};

class DateTimeFormatItem final : public Formatter::FormatItem
{
public:
    explicit DateTimeFormatItem(std::string_view format)
        : m_format(format.empty() ? DEFAULT_DATE_FORMAT : format)
    {}
    
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        // Warning: 同一个进程中的datetime format要一致，否则会串
        static thread_local time_t last_second = 0;
        static thread_local char cached_date_time[20] = {'\0'};
        
        const auto duration = record.m_timestamp.time_since_epoch();
        const time_t current_second = static_cast<time_t>(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
        if(current_second != last_second)
        {
            std::tm buffer{};
#if defined(_WIN32)
            const errno_t result = localtime_s(&buffer, &current_second);
            ASSERT_RETNONE2(result == 0, "failed to convert log time to local time");
#else
            /**
             * @brief localtime_r是线程安全的，但是消耗也大：
             * 1. 全局有锁可能排队
             * 2. 需要进行复杂的时区计算(尤其是有过夏令时变更历史的时区)
             * 3. 如果系统没有加载时区信息或者要求响应时区变更, 每次调用这个接口还要去发起磁盘IO，读系统文件/etc/localtime
             */
            const std::tm* result = localtime_r(&current_second, &buffer);
            ASSERT_RETNONE2(result != nullptr, "failed to convert log time to local time");
#endif
            const std::size_t size = std::strftime(cached_date_time, sizeof(cached_date_time), m_format.c_str(), &buffer);
            ASSERT_RETNONE2(size != 0, "failed to format log time");
            last_second = current_second;
        }

        output.append(cached_date_time);
    }

private:
    std::string m_format;
};

class MicrosecondsFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        const auto total_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(record.m_timestamp.time_since_epoch()).count();
        s64 microseconds = total_microseconds % 1'000'000;
        if(microseconds < 0)
        {
            microseconds += 1'000'000;
        }

        // 秒内微秒必须补齐成固定六位，否则 12:00:00.1234 这样的时间戳无法解析，也无法按字典序排序。
        std::array<char, MICROSECONDS_WIDTH> digits{};
        auto remaining = static_cast<u32>(microseconds);
        for(std::size_t index = digits.size(); index-- > 0;)
        {
            digits[index] = static_cast<char>('0' + remaining % 10);
            remaining /= 10;
        }
        output.append(digits.data(), digits.size());
    }

private:
    static constexpr std::size_t MICROSECONDS_WIDTH = 6;
};

class FileNameFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        output.append(record.m_fileName);
    }
};

class LineFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        AppendInteger(output, record.m_line);
    }
};

class TabFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView&, FormattedRecordBuffer& output) const override
    {
        output.append('\t');
    }
};

class FiberIdFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        AppendInteger(output, record.m_fiberId);
    }
};

class ThreadNameFormatItem final : public Formatter::FormatItem
{
public:
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const override
    {
        output.append(record.m_threadName);
    }
};

using FormatItemFactory = std::unique_ptr<Formatter::FormatItem> (*)(std::string_view);

template <typename Item>
[[nodiscard]] std::unique_ptr<Formatter::FormatItem> CreateSimpleFormatItem(std::string_view)
{
    return std::make_unique<Item>();
}

template <typename Item>
[[nodiscard]] std::unique_ptr<Formatter::FormatItem> CreateConfiguredFormatItem(std::string_view format)
{
    return std::make_unique<Item>(format);
}

[[nodiscard]] const std::unordered_map<char, FormatItemFactory>& GetFormatItemFactories()
{
    static const std::unordered_map<char, FormatItemFactory> FORMAT_ITEM_FACTORIES{
        {'m', &CreateSimpleFormatItem<MessageFormatItem>},          // 日志内容
        {'p', &CreateSimpleFormatItem<LevelFormatItem>},            // LogLevel
        {'r', &CreateSimpleFormatItem<ElapsedFormatItem>},          // 起服已过时间
        {'c', &CreateSimpleFormatItem<LoggerNameFormatItem>},       // 日志器名称
        {'t', &CreateSimpleFormatItem<ThreadIdFormatItem>},         // 线程id
        {'n', &CreateSimpleFormatItem<NewLineFormatItem>},          // 换行
        {'d', &CreateConfiguredFormatItem<DateTimeFormatItem>},     // datetime
        {'u', &CreateSimpleFormatItem<MicrosecondsFormatItem>},     // 毫秒数
        {'f', &CreateSimpleFormatItem<FileNameFormatItem>},         // 文件名
        {'l', &CreateSimpleFormatItem<LineFormatItem>},             // 行号
        {'T', &CreateSimpleFormatItem<TabFormatItem>},              // 制表符
        {'F', &CreateSimpleFormatItem<FiberIdFormatItem>},          // 协程ID
        {'N', &CreateSimpleFormatItem<ThreadNameFormatItem>},       // 大N线程名
    };
    return FORMAT_ITEM_FACTORIES;
}

[[nodiscard]] std::unique_ptr<Formatter::FormatItem> CreateFormatItem(char directive, std::string_view format)
{
    const auto& factories = GetFormatItemFactories();
    const auto it = factories.find(directive);
    ASSERT_RETVAL2(it != factories.end(), nullptr, "unknown logger format directive: " << directive);
    return it->second(format);
}

void Formatter::addLiteral(std::string& literal)
{
    if(literal.empty())
    {
        return;
    }
    m_items.push_back(std::make_unique<LiteralFormatItem>(std::move(literal)));
    literal.clear();
}

// %d{%Y-%m-%d %H:%M:%S}.%u%Tthread=%t%Tfiber=%F%T[%p]%T%f:%l%T%m%n
int Formatter::parse()
{
    std::string literal;
    for(std::size_t index = 0; index < m_pattern.size(); ++index)
    {
        if(m_pattern[index] != '%')
        {
            literal.push_back(m_pattern[index]);
            continue;
        }
        ASSERT_RETVAL2(index + 1 < m_pattern.size(), -1, "logger format pattern ends with an incomplete directive");
        const char directive = m_pattern[++index];
        if(directive == '%')
        {
            literal.push_back('%');
            continue;
        }
        addLiteral(literal);
        std::string_view item_format;
        if(index + 1 < m_pattern.size() && m_pattern[index + 1] == '{')
        {
            const std::size_t closing_brace = m_pattern.find('}', index + 2);
            ASSERT_RETVAL2(closing_brace != std::string::npos && closing_brace != (index + 2), -2, "missing a closing brace or empty");
            item_format = std::string_view(m_pattern).substr(index + 2, closing_brace - index - 2);
            index = closing_brace;
        }
        m_items.push_back(CreateFormatItem(directive, item_format));
    }
    
    addLiteral(literal);
    
    for(const auto& item : m_items)
    {
        ASSERT_RETVAL2(item != nullptr, -3, "logger formatter contains a null format item");
    }

    return 0;
}


```

### 异步日志









### 日志简化宏

利用宏构造LogLine对象并调用ostream实现<<流式输出，构造的时候需要ostream对象绑定inlineBuffer(自定义的streamBuf)

```cpp

class PANS_API LogLine final
{
public:
    LogLine(Logger& logger, LogLevel::Level level, u32 line, std::string_view file_name);
    ~LogLine() noexcept;

    LogLine(const LogLine&) = delete;
    LogLine& operator=(const LogLine&) = delete;
    LogLine(LogLine&&) = delete;
    LogLine& operator=(LogLine&&) = delete;

    [[nodiscard]] std::ostream& stream() noexcept;

private:

    struct Impl;
    static constexpr std::size_t LOG_LINE_IMPL_SIZE = 1024;
    alignas(std::max_align_t) std::byte m_implStorage[LOG_LINE_IMPL_SIZE];

    [[nodiscard]] Impl& getImpl() noexcept;
};

PANS_API void LogPrintf(Logger& logger, LogLevel::Level level, u32 line, std::string_view file_name, const char* format, ...);

} // namespace pans::detail

#define PANS_LOG_LEVEL(logger, level) \
    if (auto pans_log_logger = (logger); !pans_log_logger) {} \
    else if (const auto pans_log_level = (level); !pans_log_logger->shouldLog(pans_log_level)) {} \
    else pans::detail::LogLine(*pans_log_logger, pans_log_level, __LINE__, __FILE__).stream()

#define PANS_LOG_DEBUG(logger) PANS_LOG_LEVEL((logger), pans::LogLevel::Level::LOG_LV_DEBUG)
#define PANS_LOG_INFO(logger) PANS_LOG_LEVEL((logger), pans::LogLevel::Level::LOG_LV_INFO)
#define PANS_LOG_WARN(logger) PANS_LOG_LEVEL((logger), pans::LogLevel::Level::LOG_LV_WARN)
#define PANS_LOG_ERROR(logger) PANS_LOG_LEVEL((logger), pans::LogLevel::Level::LOG_LV_ERROR)
#define PANS_LOG_FATAL(logger) PANS_LOG_LEVEL((logger), pans::LogLevel::Level::LOG_LV_FATAL)

#define PANS_LOG_FMT_LEVEL(logger, level, format, ...) \
    if (auto pans_log_logger = (logger); !pans_log_logger) {} \
    else if (const auto pans_log_level = (level); !pans_log_logger->shouldLog(pans_log_level)) {} \
    else pans::detail::LogPrintf(*pans_log_logger, pans_log_level, __LINE__, __FILE__, (format) __VA_OPT__(,) __VA_ARGS__)

#define PANS_LOG_FMT_DEBUG(logger, format, ...) \
    PANS_LOG_FMT_LEVEL((logger), pans::LogLevel::Level::LOG_LV_DEBUG, (format) __VA_OPT__(,) __VA_ARGS__)
#define PANS_LOG_FMT_INFO(logger, format, ...) \
    PANS_LOG_FMT_LEVEL((logger), pans::LogLevel::Level::LOG_LV_INFO, (format) __VA_OPT__(,) __VA_ARGS__)
#define PANS_LOG_FMT_WARN(logger, format, ...) \
    PANS_LOG_FMT_LEVEL((logger), pans::LogLevel::Level::LOG_LV_WARN, (format) __VA_OPT__(,) __VA_ARGS__)
#define PANS_LOG_FMT_ERROR(logger, format, ...) \
    PANS_LOG_FMT_LEVEL((logger), pans::LogLevel::Level::LOG_LV_ERROR, (format) __VA_OPT__(,) __VA_ARGS__)
#define PANS_LOG_FMT_FATAL(logger, format, ...) \
    PANS_LOG_FMT_LEVEL((logger), pans::LogLevel::Level::LOG_LV_FATAL, (format) __VA_OPT__(,) __VA_ARGS__)

#define PANS_LOG_ROOT() pans::GetRootLogger()
#define PANS_LOG_NAME(name) pans::GetLogger((name))

#define LOG_DEBUG PANS_LOG_DEBUG(g_logger)
#define LOG_INFO PANS_LOG_INFO(g_logger)
#define LOG_WARN PANS_LOG_WARN(g_logger)
#define LOG_ERROR PANS_LOG_ERROR(g_logger)
#define LOG_FATAL PANS_LOG_FATAL(g_logger)






struct LogLine::Impl final
{
    Impl(Logger& logger, LogLevel::Level level, u32 line, std::string_view file_name)
        : m_logger(logger)
        , m_level(level)
        , m_line(line)
        , m_fileName(file_name)
        , m_timestamp(std::chrono::system_clock::now())
        , m_elapsed(GetElapsedTime())
        , m_threadId(GetThreadId())
        , m_fiberId(GetFiberId())
        , m_threadName(GetThreadName())
        , m_streamBuffer(m_inlineBuffer)
        , m_stream(&m_streamBuffer)
    {}

    [[nodiscard]] LogRecordView getRecord() const noexcept
    {
        return {
            m_level,
            m_logger.getName(),
            m_inlineBuffer.view(),
            m_timestamp,
            m_elapsed,
            m_threadId,
            m_fiberId,
            m_threadName,
            m_fileName,
            m_line,
        };
    }

    Logger& m_logger;
    LogLevel::Level m_level;
    u32 m_line = 0;
    std::string_view m_fileName;
    std::chrono::system_clock::time_point m_timestamp;
    std::chrono::steady_clock::duration m_elapsed;
    u64 m_threadId = 0;
    u64 m_fiberId = 0;
    std::string_view m_threadName;
    InlineBuffer<LOG_MESSAGE_INLINE_CAPACITY> m_inlineBuffer;
    SmallStreamBuffer<LOG_MESSAGE_INLINE_CAPACITY> m_streamBuffer;
    std::ostream m_stream; 
};

// 日志格式化
void LogPrintf(Logger& logger, LogLevel::Level level, u32 line, std::string_view file_name, const char* format, ...)
{
    LogLine log_line(logger, level, line, file_name);
    if(format == nullptr)
    {
        log_line.stream() << "<null-format>";
        return;
    }

    std::array<char, PRINTF_FORMAT_INLINE_CAPACITY> inline_buffer;

    va_list arguments;
    va_list arguments_copy; // 访问可变参数会破坏参数指针偏移量，为了能再次访问可变参数包，必须拷贝一份
    va_start(arguments, format);
    va_copy(arguments_copy, arguments);
    const int required_size = std::vsnprintf(inline_buffer.data(), inline_buffer.size(), format, arguments);
    va_end(arguments);

    if(required_size < 0)
    {
        va_end(arguments_copy);
        log_line.stream() << "<format-error>";
        return;
    }
    
    const std::size_t message_size = static_cast<std::size_t>(required_size);
    if(message_size < inline_buffer.size())
    {
        va_end(arguments_copy);
        log_line.stream().write(inline_buffer.data(), static_cast<std::streamsize>(message_size));
        return;
    }
    
    std::vector<char> overflow_buffer(message_size + 1);
    const int second_result = std::vsnprintf(overflow_buffer.data(), overflow_buffer.size(), format, arguments_copy);
    va_end(arguments_copy);
    if(second_result < 0)
    {
        log_line.stream() << "<format-error>";
        return;
    }
    
    log_line.stream().write(overflow_buffer.data(), static_cast<std::streamsize>(message_size));
    // 到这里还只是在内存里格式化这条日志, 出了这个函数，log_line对象析构，才把日志从内存提交到logger的appender中去
}


```






