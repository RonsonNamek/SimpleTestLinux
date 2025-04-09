#include <cstdlib>
#include <iostream>
#include <set>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "simpletest/nameofenum.h"
#include "version.h"

namespace {
enum class Color : uint16_t {
    White,
    Red,
    Green,
    Blue,
};
}

#define LOG_TO_STDERR_ONLY true

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);

    // Initialize Google's logging library.
#if LOG_TO_STDERR_ONLY
    FLAGS_colorlogtostderr = true;
    FLAGS_logtostderr = true;
#else
    FLAGS_alsologtostderr = true;
    FLAGS_log_dir = "./";
#endif

    google::EnableLogCleaner(std::chrono::minutes(10080)); // 7 days
    google::InitGoogleLogging(argv[0]);

    LOG(INFO) << argv[0] << " version:" << simpletest::version::Major << "."
              << simpletest::version::Minor << "." << simpletest::version::Patch
              << "." << simpletest::version::Git;

    std::set<uint32_t, std::greater<uint32_t>> testSet { 1, 3, 5, 7 };
    auto it1 = testSet.lower_bound(0);
    auto it2 = testSet.lower_bound(2);
    auto it3 = testSet.lower_bound(3);
    auto it4 = testSet.lower_bound(8);
    std::cout << "lower 0:" << *it1 << ", lower 2:" << *it2
              << ", lower 3:" << *it3 << ", lower 8:" << *it4 << std::endl;
    std::cout << "lower 0 end?:" << (it1 == testSet.cend())
              << ", lower 8 end?:" << (it4 == testSet.cend())
              << ", *end:" << *(testSet.cend()) << std::endl;

    constexpr uint32_t oneweek = 7 * 24 * 3600;
    // constexpr size_t szConst = std::strlen("test");
    constexpr size_t szConst2 = std::char_traits<char>::length("test2");
    constexpr std::string_view testName = ReflectionEnum<Color, Color::Blue>().valueName;
    std::cout << "1:" << oneweek << ", 2:" << szConst2 << ", 3:" << testName
              << std::endl;

    google::ShutdownGoogleLogging();
    return EXIT_SUCCESS;
}
