# string-switch-map
### Compile-time initialized map from string to any trivial constexpr type with linear time lookup (implementation - trie)
### Time complexity: `O(min(|S|, |S'|))`, where |S| is the length of the string and |S'| is the max length amongst the strings added to the data structure

#### Required language standard: C++20 (tested with g++ 13.2.0 and clang++ 16.0.5)

### Example of usage as mapper from `N` strings to integers `0`...`N-1` in the switch statement:
```c++
{
    static constexpr auto sw = StringMatch<"foo", "bar", "baz", "qux">();

    switch(sw(my_string)) {
        case sw("foo"):
            // my_string == "foo"
            break;
        case sw("bar"):
            // my_string == "bar"
            break;
        case sw("baz"):
            // my_string == "baz"
            break;
        case sw("qux"):
            // my_string == "qux"
            break;
        case sw.kDefaultValue:
        default:
            break;
    }

    static_assert(sw("foo") == 0);
    static_assert(sw("bar") == 1);
    static_assert(sw("baz") == 2);
    static_assert(sw("qux") == 3);
    static_assert(sw("not_in") == sw.kDefaultValue);
    static_assert(sw.kDefaultValue == 4);
}
```
### Example of usage as mapper from strings to enum values:
```c++
{
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
    static constexpr auto map = StringMap<
        std::array{kText1, kText2, kText3, kText4, kText1, kText3},
        /* DefaultMapValue = */ kNone,
        "text1", "text2", "text3", "text4", "Text1", "Text3">();

    static_assert(map("text1") == kText1);
    static_assert(map("text2") == kText2);
    static_assert(map("text3") == kText3);
    static_assert(map("text4") == kText4);
    static_assert(map("Text1") == kText1);
    static_assert(map("Text3") == kText3);
    static_assert(map("something else") == kNone);
    static_assert(map.kDefaultValue == kNone);
}
```

### Example of usage as mapper from strings to struct instances:
```c++
{
    constexpr std::string_view kMyConstants[] = {"abc", "def", "ghi", "sneaky input"};

    struct MyTrivialType {
        std::array<int, 2> field1{};
        int field2{};

        constexpr MyTrivialType(int arg1, int arg2, int arg3) noexcept
            : field1{arg1, arg2}, field2(arg3) {}
        constexpr bool operator==(const MyTrivialType&) const noexcept = default;
    };

    static constexpr auto map = StringMap<
        std::array{MyTrivialType(1, 2, 3), MyTrivialType(4, 5, 6), MyTrivialType(7, 8, 9)},
        /* DefaultMapValue = */ MyTrivialType(0, 0, 0),
        kMyConstants[0], kMyConstants[1], kMyConstants[2]>();

    static_assert(map(kMyConstants[0]) == MyTrivialType(1, 2, 3));
    static_assert(map(kMyConstants[1]) == MyTrivialType(4, 5, 6));
    static_assert(map(kMyConstants[2]) == MyTrivialType(7, 8, 9));
    static_assert(map(kMyConstants[3]) == MyTrivialType(0, 0, 0));
    static_assert(map.kDefaultValue == MyTrivialType(0, 0, 0));
}
```
