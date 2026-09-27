#pragma once

#include <chrono>
#include <format>
#include <iostream>
#include <map>
#include <set>
#include <string_view>
#include <vector>
//用法  {Timer _("Tag");  func();}创建局部对象,出作用域自动析构记录
//最后调用 printTimer();  显示结果

class Timer {
   public:
    using ClockType = std::chrono::high_resolution_clock;

    struct Record {
        const char *tag;
        int us;
    };

   private:
    inline thread_local static std::vector<Record> records;

    ClockType::time_point beg;
    ClockType::time_point end;
    const char *tag;

    inline Timer(const char *tag, ClockType::time_point beg);
    inline void onDestroy(ClockType::time_point end);

   public:
    Timer(const char *tag_) : Timer(tag_, ClockType::now()) {}
    ~Timer() { onDestroy(ClockType::now()); }

    static std::vector<Record> const &getRecords() { return records; }
    inline static void printLog(std::ostream &out = std::cout);
};

Timer::Timer(const char *tag_,
                             Timer::ClockType::time_point beg_)
    : beg(beg_), tag(tag_) {}

void Timer::onDestroy(Timer::ClockType::time_point end) {
    auto diff = end - beg;
    int us =
        std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
    records.push_back({tag, us});
}

void Timer::printLog(std::ostream &out) {
    if (records.size() == 0) {
        return;
    }

    struct Statistic {
        int max_us = 0;
        int min_us = 0;
        int total_us = 0;
        int count_rec = 0;
        const char *tag = nullptr;
    };
    std::map<std::string_view, Statistic> stats;
    for (auto const &[tag, us] : records) {
        auto &stat = stats[tag];
        stat.total_us += us;
        stat.max_us = std::max(stat.max_us, us);
        stat.min_us = !stat.count_rec ? us : std::min(stat.min_us, us);
        stat.count_rec++;
        stat.tag = tag;
    }

    struct StatisticCompare {
        using value_type = std::pair<std::string_view, Statistic>;
        bool operator()(value_type const &lhs, value_type const &rhs) const {
            return lhs.second.total_us > rhs.second.total_us;
        }
    };

    std::multiset<std::pair<std::string_view, Statistic>, StatisticCompare>
        sortstats(stats.begin(), stats.end());

    auto dump = [&out](int val, int w) {
        auto str = std::format("{:^{}}| ", val, w);
        out << str;
    };
    // auto dump = [&out] (int val, int w) {
    //     auto tpwv = 1;
    //     for (int i = 0; i < w - 1; i++) tpwv *= 10;
    //     if (val > tpwv) {
    //         if (val / 1000 > tpwv / 10) {
    //             out << std::setw(w - 1) << val / 1000000 << 'M';
    //         } else {
    //             out << std::setw(w - 1) << val / 1000 << 'k';
    //         }
    //     } else {
    //         out << std::setw(w) << val;
    //     }
    // };

    out << "   avg   |   min    |    max   |   total  | cnt |  tag\n";
    for (auto const &[tag, stat] : sortstats) {
        dump(stat.total_us / stat.count_rec, 9);
        dump(stat.min_us, 9);
        dump(stat.max_us, 9);
        dump(stat.total_us, 9);
        dump(stat.count_rec, 4);
        out << ' ' << tag << '\n';
    }
}

#if defined(__GNUC__) || defined(__clang__)
#define DefTimer Timer _Timer(__PRETTY_FUNCTION__);
#elif defined(_MSC_VER)
#define DefTimer Timer _Timer(__FUNCSIG__);
#else
#define DefTimer Timer _Timer(__func__);
#endif

template <class T>
static
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((noinline))
#elif defined(_MSC_VER)
    __declspec(noinline)
#endif
    void
    doNotOptimize(T volatile const &t) {
}

static void printTimer(std::ostream &out = std::cout) {
    Timer::printLog(out);
}