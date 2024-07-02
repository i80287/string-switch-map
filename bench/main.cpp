#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <ctime>
#include <random>
#include <ranges>

#include "../StringMap.hpp"

/*
 * Taken from https://github.com/tilir/c-graduate/blob/master/floating/simple-bench.h
 */
// do not optimize x away
#if defined(_MSC_VER)
#include <intrin.h>  // for _ReadWriteBarrier
static void __declspec(noinline) UseCharPointer(char const volatile* n) {}
static inline void noopt(int value) {
    UseCharPointer((char const volatile*)&value);
    _ReadWriteBarrier();
}
#define NOOPT(x) noopt(x)
#elif defined(__GNU__) || defined(__clang__)
// things are much simpler for GCC and LLVM
#define NOOPT(x) asm("" ::"r,i"(x))
#else
#error "Compiler is not supported"
#endif

// clang-format off
inline constexpr std::string_view kStrings[] = {
    "abcdefghijklmnopqrstuvwxyz",
    "bcdefghijklmnopqrstuvwxyz",
    "cdefghijklmnopqrstuvwxyz",
    "defghijklmnopqrstuvwxyz",
    "efghijklmnopqrstuvwxyz",
    "fghijklmnopqrstuvwxyz",
    "ghijklmnopqrstuvwxyz",
    "hijklmnopqrstuvwxyz",
    "ijklmnopqrstuvwxyz",
    "jklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz",
};
// clang-format on

constexpr uint64_t operator-(const timespec& t2, const timespec& t1) noexcept {
    const auto sec_passed        = static_cast<uint64_t>(t2.tv_sec - t1.tv_sec);
    auto nanoseconds_passed      = sec_passed * 1'000'000'000;
    using unsigned_nanoseconds_t = std::make_unsigned_t<decltype(timespec::tv_nsec)>;
    nanoseconds_passed += static_cast<unsigned_nanoseconds_t>(t2.tv_nsec);
    nanoseconds_passed -= static_cast<unsigned_nanoseconds_t>(t1.tv_nsec);
    return nanoseconds_passed;
}

int main() {
    constexpr auto kMeasureLimit = 10000u;

    static constexpr auto sw = StringMatch<
        kStrings[0], kStrings[1], kStrings[2], kStrings[3], kStrings[4], kStrings[5], kStrings[6],
        kStrings[7], kStrings[8], kStrings[9], kStrings[10], kStrings[11], kStrings[12],
        kStrings[13], kStrings[14], kStrings[15], kStrings[16], kStrings[17], kStrings[18],
        kStrings[19], kStrings[20], kStrings[21], kStrings[22], kStrings[23], kStrings[24],
        kStrings[25], kStrings[26], kStrings[27], kStrings[28], kStrings[29], kStrings[30],
        kStrings[31], kStrings[32], kStrings[33], kStrings[34], kStrings[35], kStrings[36],
        kStrings[37], kStrings[38], kStrings[39], kStrings[40], kStrings[41], kStrings[42],
        kStrings[43], kStrings[44], kStrings[45], kStrings[46], kStrings[47], kStrings[48],
        kStrings[49], kStrings[50], kStrings[51], kStrings[52], kStrings[53], kStrings[54],
        kStrings[55], kStrings[56], kStrings[57], kStrings[58], kStrings[59]>();

    std::array<std::size_t, kMeasureLimit> indexes{};
    {
        std::mt19937 rnd;
        std::generate_n(indexes.begin(), kMeasureLimit,
                        [&]() noexcept { return rnd() % std::size(kStrings); });
    }

    for (auto ind : indexes) {
        auto ans = sw(kStrings[ind]);
        assert(ans == ind);
    }

    timespec t1{};
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (auto ind : indexes) {
        auto ans = sw(kStrings[ind]);
        NOOPT(ans);
    }
    timespec t2{};
    clock_gettime(CLOCK_MONOTONIC, &t2);

    printf("%" PRIu64 " nanoseconds on average\n", (t2 - t1) / kMeasureLimit);
}
