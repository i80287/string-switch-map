#include <array>
#include <iostream>
#include <string>
#include <string_view>

#include "StringMap.hpp"

void StringSwitchExample() {
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

    std::cout << "Input string to search:\n> ";
    std::string input;
    std::cin >> input;
    std::string_view ans;
    switch (sw(input)) {
        case sw("abc"):
            ans = "found string \"abc\"";
            break;
        case sw("def"):
            ans = "found string \"def\"";
            break;
        case sw("ghij"):
            ans = "found string \"ghij\"";
            break;
        case sw("foo"):
            ans = "found string \"foo\"";
            break;
        case sw("bar"):
            ans = "found string \"bar\"";
            break;
        case sw("baz"):
            ans = "found string \"baz\"";
            break;
        case sw("qux"):
            ans = "found string \"qux\"";
            break;
        case sw("abacaba"):
            ans = "found string \"abacaba\"";
            break;
        case sw("ring"):
            ans = "found string \"ring\"";
            break;
        case sw("ideal"):
            ans = "found string \"ideal\"";
            break;
        case sw("GLn(F)"):
            ans = "found string \"GLn(F)\"";
            break;
        case sw.kDefaultValue:
        default:
            ans = "not in the switch!";
            break;
    }

    std::cout << ans << '\n';
}

void ComileTimeStringMapExample1() {
    // Map from string to integers, just pass N string literals,
    //  and they will be mapped to the intergers 0...N-1 respectively
    // Default integer value in the matcher will be N
    static constexpr auto match = StringMatch<"text1", "text2", "text3", "text4">();
    static_assert(match("text1") == 0);
    static_assert(match("text2") == 1);
    static_assert(match("text3") == 2);
    static_assert(match("text4") == 3);
    static_assert(match("not in") == match.kDefaultValue);
    static_assert(match.kDefaultValue == 4);

    std::cout << "Max char amongst strings added to the data structure: '" << match.kMaxChar
              << "'\n"
              << "Min char amongst strings added to the data structure: '" << match.kMinChar
              << "'\n"
              << "Default mapped value in the data structure: " << match.kDefaultValue << '\n';

    // Map from string to enum
    enum class SomeEnum {
        kText1,
        kText2,
        kText3,
        kText4,
        kNone,
    };
    using enum SomeEnum;

    // First, pass N values in the std::array
    // Then, pass default value
    // Then, pass N string literals
    static constexpr auto map =
        StringMap<std::array{kText1, kText2, kText3, kText4, kText1, kText3}, kNone, "text1",
                  "text2", "text3", "text4", "Text1", "Text3">();

    static_assert(map("text1") == kText1);
    static_assert(map("text2") == kText2);
    static_assert(map("text3") == kText3);
    static_assert(map("text4") == kText4);
    static_assert(map("Text1") == kText1);
    static_assert(map("Text3") == kText3);
    static_assert(map("something else") == kNone);
    static_assert(map.kDefaultValue == kNone);
}

void ComileTimeStringMapExample2() {
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
                  MyTrivialType(0, 0, 0), kMyConstants[0], kMyConstants[1], kMyConstants[2]>();

    static_assert(map(kMyConstants[0]) == MyTrivialType(1, 2, 3));
    static_assert(map(kMyConstants[1]) == MyTrivialType(4, 5, 6));
    static_assert(map(kMyConstants[2]) == MyTrivialType(7, 8, 9));
    static_assert(map(kMyConstants[3]) == MyTrivialType(0, 0, 0));
    static_assert(map.kDefaultValue == MyTrivialType(0, 0, 0));
}

int main() {
    StringSwitchExample();
    ComileTimeStringMapExample1();
    ComileTimeStringMapExample2();
}
