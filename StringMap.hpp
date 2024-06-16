/**
 * Copyright 2024 https://github.com/i80287
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace string_map_detail {
namespace {

inline constexpr std::size_t kMaxStringViewSize = 200;

template <std::size_t N = kMaxStringViewSize>
struct [[nodiscard]] CompileTimeStringLiteral {
    static_assert(N > 0);
    consteval CompileTimeStringLiteral(std::string_view str) noexcept : length(str.size()) {
        // Change kMaxStringViewSize if you are using
        //  very long strings in the StringMatch / StringMap.
        [[maybe_unused]] const auto string_view_size_check = 0 / (str.size() <= std::size(value));
        std::char_traits<char>::copy(value.data(), str.data(), str.size());
    }
    consteval CompileTimeStringLiteral(const char (&str)[N]) noexcept
        : length(str[N - 1] == '\0' ? N - 1 : N) {
        std::char_traits<char>::copy(value.data(), str, size());
    }
    [[nodiscard]] consteval std::size_t size() const noexcept {
        return length;
    }
    [[nodiscard]] consteval char operator[](std::size_t index) const noexcept {
        return value[index];
    }
    [[nodiscard]] consteval char& operator[](std::size_t index) noexcept {
        return value[index];
    }

    std::array<char, N> value{};
    const std::size_t length{};
};

namespace trie_tools {

struct TrieParamsType final {
    static constexpr std::uint32_t kRootNodeIndex = 0;
    std::uint32_t min_char;
    std::uint32_t max_char;
    std::size_t trie_alphabet_size = max_char - min_char + 1;
    std::size_t nodes_size{};

    [[nodiscard]] constexpr std::size_t CharToNodeIndex(unsigned char chr) const noexcept {
        return static_cast<std::size_t>(chr) - min_char;
    }
    [[nodiscard]] constexpr std::size_t CharToNodeIndex(signed char chr) const noexcept {
        return CharToNodeIndex(static_cast<unsigned char>(chr));
    }
    [[nodiscard]] constexpr std::size_t CharToNodeIndex(char chr) const noexcept {
        return CharToNodeIndex(static_cast<unsigned char>(chr));
    }
};

};  // namespace trie_tools

namespace counting_tools {

struct MinMaxCharsType {
    std::uint32_t min_char;
    std::uint32_t max_char;
};

template <string_map_detail::CompileTimeStringLiteral FirstString,
          string_map_detail::CompileTimeStringLiteral... Strings>
consteval MinMaxCharsType FindMinMaxChars() noexcept {
    // std::min and std::max are not used in order not to
    // include additional header (<algorithm>) just for min/max.
    constexpr auto min_fn = [](std::uint32_t a, std::uint32_t b) constexpr noexcept {
        return a <= b ? a : b;
    };
    constexpr auto max_fn = [](std::uint32_t a, std::uint32_t b) constexpr noexcept {
        return a >= b ? a : b;
    };

    // Not FirstString.size() because it can be 0.
    constexpr auto kMaxUINT8  = std::numeric_limits<std::uint8_t>::max() + 1;
    std::uint32_t min_char    = kMaxUINT8;
    std::uint32_t max_char    = 0;
    constexpr std::size_t len = FirstString.size();
    for (std::size_t i = 0; i < len; i++) {
        const std::uint32_t chr = static_cast<std::uint8_t>(FirstString[i]);
        min_char                = min_fn(min_char, chr);
        max_char                = max_fn(max_char, chr);
    }
    if constexpr (sizeof...(Strings) > 0) {
        constexpr MinMaxCharsType mn_mx_chars = FindMinMaxChars<Strings...>();
        min_char                              = min_fn(min_char, mn_mx_chars.min_char);
        max_char                              = max_fn(max_char, mn_mx_chars.max_char);
    }

    if (min_char == kMaxUINT8) {
        // All string are empty.
        min_char = 0;
    }

    return {
        .min_char = min_char,
        .max_char = max_char,
    };
}

template <std::size_t AlphabetSize>
struct CountingNode final {
    std::array<std::uint32_t, AlphabetSize> edges{};
};

template <trie_tools::TrieParamsType TrieParams,
          string_map_detail::CompileTimeStringLiteral FirstString,
          string_map_detail::CompileTimeStringLiteral... Strings>
consteval std::size_t CountNodesImpl(
    std::vector<CountingNode<TrieParams.trie_alphabet_size>>& nodes) {
    std::size_t current_node_index = 0;
    constexpr std::size_t len      = FirstString.size();
    for (std::size_t i = 0; i < len; i++) {
        std::size_t index           = TrieParams.CharToNodeIndex(FirstString[i]);
        std::size_t next_node_index = nodes[current_node_index].edges[index];
        if (next_node_index == 0) {
            std::size_t new_node_index = nodes.size();
            nodes.emplace_back();
            nodes[current_node_index].edges[index] = std::uint32_t(new_node_index);
            next_node_index                        = new_node_index;
        }
        current_node_index = next_node_index;
    }

    if constexpr (sizeof...(Strings) > 0) {
        return CountNodesImpl<TrieParams, Strings...>(nodes);
    } else {
        return nodes.size();
    }
}

template <trie_tools::TrieParamsType TrieParams,
          string_map_detail::CompileTimeStringLiteral... Strings>
consteval std::size_t CountNodes() {
    std::vector<CountingNode<TrieParams.trie_alphabet_size>> nodes(std::size_t(1));
    return CountNodesImpl<TrieParams, Strings...>(nodes);
}

template <string_map_detail::CompileTimeStringLiteral... Strings>
consteval trie_tools::TrieParamsType TrieParams() {
    constexpr MinMaxCharsType kMinMaxChars           = FindMinMaxChars<Strings...>();
    constexpr trie_tools::TrieParamsType kTrieParams = {
        .min_char = kMinMaxChars.min_char,
        .max_char = kMinMaxChars.max_char,
    };
    return {
        .min_char   = kTrieParams.min_char,
        .max_char   = kTrieParams.max_char,
        .nodes_size = CountNodes<kTrieParams, Strings...>(),
    };
}

}  // namespace counting_tools

namespace string_map_impl {

template <trie_tools::TrieParamsType TrieParams, std::array MappedValues,
          decltype(MappedValues)::value_type DefaultMapValue,
          string_map_detail::CompileTimeStringLiteral... Strings>
class [[nodiscard]] StringMapImpl final {
    static_assert(0 < TrieParams.min_char && TrieParams.min_char <= TrieParams.max_char &&
                      TrieParams.max_char <= std::numeric_limits<std::uint8_t>::max(),
                  "Empty string was passed in StringMatch / StringMap");

public:
    using MappedType = decltype(MappedValues)::value_type;
    static_assert(std::is_copy_assignable_v<MappedType>);

    static constexpr MappedType kDefaultValue = DefaultMapValue;
    static constexpr char kMinChar            = static_cast<char>(TrieParams.min_char);
    static constexpr char kMaxChar            = static_cast<char>(TrieParams.max_char);

    consteval StringMapImpl() noexcept {
        AddPattern<0, Strings...>(kRootNodeIndex + 1);
    }

    [[nodiscard]] consteval MappedType operator()(std::nullptr_t) const noexcept = delete;
    [[nodiscard]] constexpr MappedType operator()(std::string_view str) const noexcept {
        return operator()(str.data());
    }
    [[nodiscard]] constexpr MappedType operator()(const std::string& str) const noexcept {
        return operator()(str.c_str());
    }
    [[nodiscard]] constexpr MappedType operator()(const char* str) const noexcept {
        if (str == nullptr) [[unlikely]] {
            return kDefaultValue;
        }

        std::size_t current_node_index = kRootNodeIndex;
        if (std::is_constant_evaluated()) {
            const std::size_t str_size = std::char_traits<char>::length(str);
            for (std::size_t i = 0; i < str_size; i++) {
                std::size_t index = TrieParams.CharToNodeIndex(str[i]);
                if (index >= kTrieAlphabetSize) {
                    return kDefaultValue;
                }

                std::size_t next_node_index = nodes_[current_node_index].edges[index];
                if (next_node_index != 0) {
                    current_node_index = next_node_index;
                } else {
                    return kDefaultValue;
                }
            }
        } else {
            const auto* ustr = std::bit_cast<const unsigned char*>(str);
            for (unsigned char c{}; (c = *ustr) != '\0'; ++ustr) {
                std::size_t index = TrieParams.CharToNodeIndex(c);
                if (index >= kTrieAlphabetSize) {
                    return kDefaultValue;
                }

                std::size_t next_node_index = nodes_[current_node_index].edges[index];
                if (next_node_index != 0) {
                    current_node_index = next_node_index;
                } else {
                    return kDefaultValue;
                }
            }
        }

        return nodes_[current_node_index].node_value;
    }

private:
    using NodeIndex = std::uint32_t;

    static constexpr NodeIndex kRootNodeIndex      = TrieParams.kRootNodeIndex;
    static constexpr std::size_t kTrieAlphabetSize = TrieParams.trie_alphabet_size;
    static constexpr std::size_t kNodesSize        = TrieParams.nodes_size;

    struct TrieNodeImpl final {
        std::array<NodeIndex, kTrieAlphabetSize> edges{};
        MappedType node_value = kDefaultValue;
    };
    std::array<TrieNodeImpl, kNodesSize> nodes_{};

    template <std::size_t CurrentPackIndex, string_map_detail::CompileTimeStringLiteral String,
              string_map_detail::CompileTimeStringLiteral... AddStrings>
    consteval void AddPattern(std::size_t first_free_node_index) noexcept {
        std::size_t current_node_index = 0;
        constexpr std::size_t len      = String.size();
        for (std::size_t i = 0; i < len; i++) {
            std::size_t symbol_index    = TrieParams.CharToNodeIndex(String[i]);
            std::size_t next_node_index = nodes_[current_node_index].edges[symbol_index];
            if (next_node_index == 0) {
                nodes_[current_node_index].edges[symbol_index] =
                    static_cast<NodeIndex>(first_free_node_index);
                next_node_index = first_free_node_index;
                first_free_node_index++;
            }
            current_node_index = next_node_index;
        }

        // Division by zero will occur in compile time if
        //  the same patterns are added to the trie.
        [[maybe_unused]] const auto duplicate_strings_check =
            0 / (nodes_[current_node_index].node_value == kDefaultValue);
        static_assert(CurrentPackIndex < MappedValues.size(), "impl error");
        nodes_[current_node_index].node_value = MappedValues[CurrentPackIndex];
        if constexpr (sizeof...(AddStrings) > 0) {
            AddPattern<CurrentPackIndex + 1, AddStrings...>(first_free_node_index);
        }
    }
};

}  // namespace string_map_impl

template <std::size_t N>
consteval std::array<std::size_t, N> make_index_array() noexcept {
    std::array<std::size_t, N> index_array{};
    std::iota(index_array.begin(), index_array.end(), 0);
    return index_array;
}

}  // namespace
}  // namespace string_map_detail

template <std::array MappedValues, decltype(MappedValues)::value_type DefaultMapValue,
          string_map_detail::CompileTimeStringLiteral... Strings>
    requires(sizeof...(Strings) == MappedValues.size())
using StringMap = typename string_map_detail::string_map_impl::StringMapImpl<
    string_map_detail::counting_tools::TrieParams<Strings...>(), MappedValues, DefaultMapValue,
    Strings...>;

template <string_map_detail::CompileTimeStringLiteral... Strings>
using StringMatch = StringMap<string_map_detail::make_index_array<sizeof...(Strings)>(),
                              sizeof...(Strings), Strings...>;
