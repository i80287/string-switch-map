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
#elif defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)
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

static void run_bench() {
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

int main() {
    {
        static constexpr auto sw = StringMatch<"abc", "def", "ghij", "foo", "bar", "baz", "qux",
                                               "abacaba", "ring", "ideal", "GLn(F)">();
        static_assert(sw("abc") == 0);
        static_assert(sw("def") == 1);
        static_assert(sw("ghij") == 2);
        static_assert(sw("foo") == 3);
        static_assert(sw("bar") == 4);
        static_assert(sw("baz") == 5);
        static_assert(sw("qux") == 6);
        static_assert(sw("abacaba") == 7);
        static_assert(sw("ring") == 8);
        static_assert(sw("ideal") == 9);
        static_assert(sw("GLn(F)") == 10);
        static_assert(sw.kDefaultValue == sw("GLn(F)") + 1);
        static_assert(sw.kDefaultValue == 11);
        static_assert(sw("not_in") == sw.kDefaultValue);
        static_assert(sw("") == sw.kDefaultValue);
        static_assert(sw("a") == sw.kDefaultValue);
        static_assert(sw("A") == sw.kDefaultValue);
        static_assert(sw("bc") == sw.kDefaultValue);
        static_assert(sw("de") == sw.kDefaultValue);
        constexpr const unsigned char kUString[] = "abc";
        static_assert(sw(kUString, std::size(kUString) - 1) == sw("abc"));

        assert(sw("abc") == 0);
        assert(sw("def") == 1);
        assert(sw("ghij") == 2);
        assert(sw("foo") == 3);
        assert(sw("bar") == 4);
        assert(sw("baz") == 5);
        assert(sw("qux") == 6);
        assert(sw("abacaba") == 7);
        assert(sw("ring") == 8);
        assert(sw("ideal") == 9);
        assert(sw("GLn(F)") == 10);
        assert(sw.kDefaultValue == sw("GLn(F)") + 1);
        assert(sw.kDefaultValue == 11);
        assert(sw("not_in") == sw.kDefaultValue);
        assert(sw("") == sw.kDefaultValue);
        assert(sw("a") == sw.kDefaultValue);
        assert(sw("A") == sw.kDefaultValue);
        assert(sw("bc") == sw.kDefaultValue);
        assert(sw("de") == sw.kDefaultValue);
        assert(sw(kUString, std::size(kUString) - 1) == sw("abc"));
    }
    {
        static constexpr auto match = StringMatch<"text1", "text2", "text3", "text4">();
        static_assert(match("text1") == 0);
        static_assert(match("text2") == 1);
        static_assert(match("text3") == 2);
        static_assert(match("text4") == 3);
        static_assert(match("not in") == match.kDefaultValue);
        static_assert(match.kDefaultValue == 4);

        assert(match("text1") == 0);
        assert(match("text2") == 1);
        assert(match("text3") == 2);
        assert(match("text4") == 3);
        assert(match("not in") == match.kDefaultValue);
        assert(match.kDefaultValue == 4);
    }
    {
        enum class SomeEnum {
            kText1,
            kText2,
            kText3,
            kText4,
            kNone,
        };
        using enum SomeEnum;
        static constexpr auto map =
            StringMap<std::array{kText1, kText2, kText3, kText4, kText1, kText3},
                      /* DefaultMapValue = */ kNone, "text1", "text2", "text3", "text4", "Text1",
                      "Text3">();

        static_assert(map("text1") == kText1);
        static_assert(map("text2") == kText2);
        static_assert(map("text3") == kText3);
        static_assert(map("text4") == kText4);
        static_assert(map("Text1") == kText1);
        static_assert(map("Text3") == kText3);
        static_assert(map("something else") == kNone);
        static_assert(map.kDefaultValue == kNone);

        assert(map("text1") == kText1);
        assert(map("text2") == kText2);
        assert(map("text3") == kText3);
        assert(map("text4") == kText4);
        assert(map("Text1") == kText1);
        assert(map("Text3") == kText3);
        assert(map("something else") == kNone);
        assert(map.kDefaultValue == kNone);
    }
    {
        constexpr std::string_view kMyConstants[] = {"abc", "def", "ghi", "sneaky input"};

        struct MyTrivialType {
            std::array<int, 2> field1{};
            int field2{};

            constexpr MyTrivialType(int arg1, int arg2, int arg3) noexcept
                : field1{arg1, arg2}, field2(arg3) {}
            constexpr bool operator==(const MyTrivialType&) const noexcept = default;
        };

        static constexpr auto map =
            StringMap<std::array{MyTrivialType(1, 2, 3), MyTrivialType(4, 5, 6),
                                 MyTrivialType(7, 8, 9)},
                      /* DefaultMapValue = */ MyTrivialType(0, 0, 0), kMyConstants[0],
                      kMyConstants[1], kMyConstants[2]>();

        static_assert(map(kMyConstants[0]) == MyTrivialType(1, 2, 3));
        static_assert(map(kMyConstants[1]) == MyTrivialType(4, 5, 6));
        static_assert(map(kMyConstants[2]) == MyTrivialType(7, 8, 9));
        static_assert(map(kMyConstants[3]) == MyTrivialType(0, 0, 0));
        static_assert(map.kDefaultValue == MyTrivialType(0, 0, 0));

        assert(map(kMyConstants[0]) == MyTrivialType(1, 2, 3));
        assert(map(kMyConstants[1]) == MyTrivialType(4, 5, 6));
        assert(map(kMyConstants[2]) == MyTrivialType(7, 8, 9));
        assert(map(kMyConstants[3]) == MyTrivialType(0, 0, 0));
        assert(map.kDefaultValue == MyTrivialType(0, 0, 0));
    }

    run_bench();
    return 0;
}
