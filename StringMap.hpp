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

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#if defined(__cpp_lib_span) && __cpp_lib_span >= 202002L && defined(__has_include) && \
    __has_include(<span>)
#define STRING_MAP_USE_SPAN 1
#include <span>
#else
#define STRING_MAP_USE_SPAN 0
#endif

#if defined(__cpp_consteval) && __cpp_consteval >= 201811L
#define STRING_MAP_CONSTEVAL consteval
#else
#define STRING_MAP_CONSTEVAL constexpr
#endif

namespace string_map_detail {

inline constexpr std::size_t kMaxStringViewSize = 200;

template <std::size_t N = kMaxStringViewSize>
struct [[nodiscard]] CompileTimeStringLiteral {
    static_assert(N > 0);
    STRING_MAP_CONSTEVAL CompileTimeStringLiteral(std::string_view str) noexcept
        : length(str.size()) {
        // HINT: change kMaxStringViewSize if you are using
        //  very long strings in the StringMatch / StringMap.
        [[maybe_unused]] const auto string_view_size_check = 0 / (str.size() <= std::size(value));
        std::char_traits<char>::copy(value.data(), str.data(), str.size());
    }
    STRING_MAP_CONSTEVAL CompileTimeStringLiteral(const char (&str)[N]) noexcept
        : length(str[N - 1] == '\0' ? N - 1 : N) {
        std::char_traits<char>::copy(value.data(), str, size());
    }
    [[nodiscard]] STRING_MAP_CONSTEVAL std::size_t size() const noexcept {
        return length;
    }
    [[nodiscard]] STRING_MAP_CONSTEVAL char operator[](std::size_t index) const noexcept {
        return value[index];
    }
    [[nodiscard]] STRING_MAP_CONSTEVAL char& operator[](std::size_t index) noexcept {
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
STRING_MAP_CONSTEVAL MinMaxCharsType FindMinMaxChars() noexcept {
    static_assert(FirstString.size() > 0, "Empty string was passed in StringMatch / StringMap");

    std::uint32_t min_char    = FirstString[0];
    std::uint32_t max_char    = FirstString[0];
    constexpr std::size_t len = FirstString.size();
    for (std::size_t i = 1; i < len; i++) {
        const std::uint32_t chr = static_cast<std::uint8_t>(FirstString[i]);
        min_char                = std::min(min_char, chr);
        max_char                = std::max(max_char, chr);
    }
    if constexpr (sizeof...(Strings) > 0) {
        constexpr MinMaxCharsType mn_mx_chars = FindMinMaxChars<Strings...>();
        min_char                              = std::min(min_char, mn_mx_chars.min_char);
        max_char                              = std::max(max_char, mn_mx_chars.max_char);
    }

    return {
        .min_char = min_char,
        .max_char = max_char,
    };
}

template <std::size_t AlphabetSize>
struct CountingVector final {
    struct CountingNode final {
        std::array<std::uint32_t, AlphabetSize> edges{};
    };

    using value_type = CountingNode;

    struct CountingVectorAllocatorImpl final {
        [[nodiscard]] constexpr value_type* allocate(std::size_t size) const {
            // Can't call ::operator new(...) in the constexpr context
            return new value_type[size]();
        }
        constexpr void deallocate(value_type* ptr,
                                  [[maybe_unused]] std::size_t size) const noexcept {
            delete[] ptr;
        }
    };
    using allocator_type = CountingVectorAllocatorImpl;

    explicit constexpr CountingVector(std::size_t size)
        : size_(size), capacity_(size_ > 16 ? size_ : 16) {
        data_ = allocator_type{}.allocate(capacity_);
    }
    constexpr CountingVector(const CountingVector& other)
        : size_(other.size_), capacity_(other.capacity_) {
        data_ = allocator_type{}.allocate(capacity_);
#if defined(__cpp_lib_constexpr_algorithms) && __cpp_lib_constexpr_algorithms >= 201806L
        std::copy_n(other.data_, size_, data_);
#else
        const auto other_iter = other.data_;
        for (auto iter = data_, end = iter + size_; iter != end; ++iter, ++other_iter) {
            *iter = *other_iter;
        }
#endif
    }
    constexpr CountingVector(CountingVector&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)),
          size_(std::exchange(other.size_, nullptr)),
          capacity_(std::exchange(other.capacity_, nullptr)) {}
    constexpr CountingVector& operator=(const CountingVector& other) {
        return *this = CountingVector(other);
    }
    constexpr CountingVector& operator=(CountingVector&& other) noexcept {
        swap(*this, other);
        return *this;
    }
    friend constexpr void swap(CountingVector& lhs, CountingVector& rhs) noexcept {
        std::swap(lhs.data_, rhs.data_);
        std::swap(lhs.size_, rhs.size_);
        std::swap(lhs.capacity_, rhs.capacity_);
    }
    constexpr ~CountingVector() {
        allocator_type{}.deallocate(data_, capacity_);
        data_ = nullptr;
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return size_;
    }
    [[nodiscard]] constexpr std::size_t capacity() const noexcept {
        return capacity_;
    }
    [[nodiscard]] constexpr bool empty() const noexcept {
        return size_ == 0;
    }
    [[nodiscard]] constexpr value_type& operator[](std::size_t index) noexcept {
        return data_[index];
    }
    [[nodiscard]] constexpr const value_type& operator[](std::size_t index) const noexcept {
        return data_[index];
    }
    constexpr void emplace_back_empty_node() {
        if (size_ == capacity_) {
            growth_storage();
        }
        ++size_;
    }
    constexpr void growth_storage() {
        const auto new_capacity = capacity_ > 0 ? capacity_ * 2 : 16;
        auto new_data           = allocator_type{}.allocate(new_capacity);
#if defined(__cpp_lib_constexpr_algorithms) && __cpp_lib_constexpr_algorithms >= 201806L
        std::move(data_, data_ + size_, new_data);
#else
        auto new_data_iter = new_data;
        for (auto iter = data_, end = iter + size_; iter != end; ++iter, ++new_data_iter) {
            *new_data_iter = std::move(*iter);
        }
#endif
        allocator_type{}.deallocate(data_, capacity_);
        data_     = new_data;
        capacity_ = new_capacity;
    }

    value_type* data_{};
    std::size_t size_{};
    std::size_t capacity_{};
};

template <trie_tools::TrieParamsType TrieParams,
          string_map_detail::CompileTimeStringLiteral FirstString,
          string_map_detail::CompileTimeStringLiteral... Strings>
STRING_MAP_CONSTEVAL std::size_t CountNodesImpl(
    CountingVector<TrieParams.trie_alphabet_size>& nodes) {
    std::size_t current_node_index = 0;
    constexpr std::size_t len      = FirstString.size();
    for (std::size_t i = 0; i < len; i++) {
        std::size_t index           = TrieParams.CharToNodeIndex(FirstString[i]);
        std::size_t next_node_index = nodes[current_node_index].edges[index];
        if (next_node_index == 0) {
            std::size_t new_node_index = nodes.size();
            nodes.emplace_back_empty_node();
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
STRING_MAP_CONSTEVAL std::size_t CountNodes() {
    constexpr auto kAlphabetSize = TrieParams.trie_alphabet_size;
    CountingVector<kAlphabetSize> nodes(std::size_t(1));
    return CountNodesImpl<TrieParams, Strings...>(nodes);
}

template <string_map_detail::CompileTimeStringLiteral... Strings>
STRING_MAP_CONSTEVAL trie_tools::TrieParamsType TrieParams() {
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
    static_assert(sizeof...(Strings) == std::size(MappedValues) && std::size(MappedValues) > 0,
                  "internal error");

    template <bool InCompileTime>
    struct [[nodiscard]] InternalIterator final {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type = std::conditional_t<InCompileTime, const char, const unsigned char>;
        using pointer    = value_type*;
        using reference  = value_type&;

        explicit constexpr InternalIterator(const char* str) noexcept {
            if constexpr (InCompileTime) {
                pointer_ = str;
            } else {
                pointer_ = std::bit_cast<pointer>(str);
            }
        }
        constexpr InternalIterator& operator++() noexcept {
            ++pointer_;
            return *this;
        }
        [[nodiscard]] constexpr value_type operator*() const noexcept {
            return *pointer_;
        }
        [[nodiscard]] constexpr value_type operator[](std::size_t index) const noexcept {
            return pointer_[index];
        }
        [[nodiscard]] constexpr bool operator==(const InternalIterator& other) const noexcept {
            return pointer_ == other.pointer_;
        }
        struct CStringSentinel {};
        [[nodiscard]] constexpr bool operator==(const CStringSentinel&) const noexcept {
            return *pointer_ == '\0';
        }

        pointer pointer_{};
    };

public:
    using MappedType = decltype(MappedValues)::value_type;
    static_assert(std::is_copy_assignable_v<MappedType>);

    static constexpr MappedType kDefaultValue = DefaultMapValue;
    static constexpr char kMinChar            = static_cast<char>(TrieParams.min_char);
    static constexpr char kMaxChar            = static_cast<char>(TrieParams.max_char);

    STRING_MAP_CONSTEVAL StringMapImpl() noexcept {
        AddPattern<0, Strings...>(kRootNodeIndex + 1);
    }

    [[nodiscard]] constexpr MappedType operator()(std::nullptr_t) const noexcept = delete;
    [[nodiscard]] constexpr MappedType operator()(std::string_view str) const noexcept {
        return operator()(str.data(), str.size());
    }
    [[nodiscard]] constexpr MappedType operator()(const std::string& str) const noexcept {
        return operator()(str.data(), str.size());
    }
    template <std::size_t N>
    [[nodiscard]] constexpr MappedType operator()(const char (&str)[N]) const noexcept {
        const bool ends_with_zero_char = N > 0 && str[N - 1] == '\0';
        return operator()(str, ends_with_zero_char ? N - 1 : N);
    }
    [[nodiscard]] constexpr MappedType operator()(const char* str,
                                                  std::size_t size) const noexcept {
        if (std::is_constant_evaluated()) {
            using IteratorType = InternalIterator</*InCompileTime = */ true>;
            return operator_call_impl(IteratorType(str), IteratorType(str + size));
        } else {
            using IteratorType = InternalIterator</*InCompileTime = */ false>;
            return operator_call_impl(IteratorType(str), IteratorType(str + size));
        }
    }
    [[nodiscard]] constexpr MappedType operator()(const char* str) const noexcept {
        if (std::is_constant_evaluated()) {
            using IteratorType = InternalIterator</*InCompileTime = */ true>;
            using SentinelType = typename IteratorType::CStringSentinel;
            return operator_call_impl(IteratorType(str), SentinelType{});
        } else {
            using IteratorType = InternalIterator</*InCompileTime = */ false>;
            using SentinelType = typename IteratorType::CStringSentinel;
            return operator_call_impl(IteratorType(str), SentinelType{});
        }
    }
#if STRING_MAP_USE_SPAN
    template <std::size_t SpanExtent>
    [[nodiscard]] constexpr MappedType operator()(
        std::span<const char, SpanExtent> str) const noexcept {
        const bool ends_with_zero_char = !str.empty() && str.last() == '\0';
        return operator()(str.data(), ends_with_zero_char ? str.size() - 1 : str.size());
    }
#endif

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
    STRING_MAP_CONSTEVAL void AddPattern(std::size_t first_free_node_index) noexcept {
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
        // HINT: remove duplicates from the trie.
        [[maybe_unused]] const auto duplicate_strings_check =
            0 / (nodes_[current_node_index].node_value == kDefaultValue);
        static_assert(CurrentPackIndex < MappedValues.size(), "impl error");
        nodes_[current_node_index].node_value = MappedValues[CurrentPackIndex];
        if constexpr (sizeof...(AddStrings) > 0) {
            AddPattern<CurrentPackIndex + 1, AddStrings...>(first_free_node_index);
        }
    }

    template <class IteratorType, class SentinelIteratorType>
    constexpr MappedType operator_call_impl(IteratorType begin,
                                            SentinelIteratorType end) const noexcept {
        std::size_t current_node_index = kRootNodeIndex;
        for (; begin != end; ++begin) {
            std::size_t index = TrieParams.CharToNodeIndex(*begin);
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

        const auto returned_value = nodes_[current_node_index].node_value;

        if constexpr (kMappedTypesInfo.ordered) {
            if (returned_value != kDefaultValue && (returned_value < kMappedTypesInfo.min_value ||
                                                    returned_value > kMappedTypesInfo.max_value)) {
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
                std::unreachable();
#elif defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
                __builtin_unreachable();
#elif __has_builtin(__builtin_assume)
                __builtin_assume(false);
#endif
#endif
            }
        }

        return returned_value;
    }

    struct TMappedTypesInfo final {
        static constexpr bool kMaybeOrdered =
            std::is_arithmetic_v<MappedType> ||
            (std::is_enum_v<MappedType> && requires(MappedType x, MappedType y) {
                { x < y } -> std::convertible_to<bool>;
                { x <= y } -> std::convertible_to<bool>;
                { x > y } -> std::convertible_to<bool>;
                { x >= y } -> std::convertible_to<bool>;
            });
        using DefaultConstructibleSentinelType =
            std::conditional_t<std::is_default_constructible_v<MappedType>, MappedType, int>;

        bool ordered = false;
        DefaultConstructibleSentinelType max_value{};
        DefaultConstructibleSentinelType min_value{};
    };

    STRING_MAP_CONSTEVAL static TMappedTypesInfo get_mapped_values_info() noexcept {
        TMappedTypesInfo info{};

        // TODO: Add support for pointers (ub gets very tricky there)
        if constexpr (TMappedTypesInfo::kMaybeOrdered) {
            if constexpr (std::is_floating_point_v<MappedType>) {
                auto bad_float = [](MappedType x) constexpr noexcept -> bool {
                    return x != x || x >= std::numeric_limits<MappedType>::infinity() ||
                           x <= -std::numeric_limits<MappedType>::infinity() ||
                           x == std::numeric_limits<MappedType>::quiet_NaN() ||
                           x == std::numeric_limits<MappedType>::signaling_NaN();
                };
#if defined(__cpp_lib_constexpr_algorithms) && __cpp_lib_constexpr_algorithms >= 201806L
                if (std::any_of(MappedValues.begin(), MappedValues.end(), bad_float)) {
                    return info;
                }
#else
                for (const auto x : MappedValues) {
                    if (bad_float(x)) {
                        return info;
                    }
                }
#endif
            }

            info.ordered = true;
            const auto [min_iter, max_iter] =
                std::minmax_element(MappedValues.begin(), MappedValues.end());
            info.min_value = *min_iter;
            info.max_value = *max_iter;
        }

        return info;
    }

    static constexpr TMappedTypesInfo kMappedTypesInfo = get_mapped_values_info();
};

}  // namespace string_map_impl

template <std::size_t N>
STRING_MAP_CONSTEVAL std::array<std::size_t, N> make_index_array() noexcept {
    std::array<std::size_t, N> index_array{};
#if defined(__cpp_lib_constexpr_numeric) && __cpp_lib_constexpr_numeric >= 201911L
    std::iota(index_array.begin(), index_array.end(), 0);
#else
    for (std::size_t i = 0; i < index_array.size(); i++) {
        index_array[i] = i;
    }
#endif
    return index_array;
}

}  // namespace string_map_detail

#undef STRING_MAP_CONSTEVAL
#undef STRING_MAP_USE_SPAN

template <std::array MappedValues, decltype(MappedValues)::value_type DefaultMapValue,
          string_map_detail::CompileTimeStringLiteral... Strings>
    requires(sizeof...(Strings) == std::size(MappedValues) && std::size(MappedValues) > 0)
using StringMap = typename string_map_detail::string_map_impl::StringMapImpl<
    string_map_detail::counting_tools::TrieParams<Strings...>(), MappedValues, DefaultMapValue,
    Strings...>;

template <string_map_detail::CompileTimeStringLiteral... Strings>
using StringMatch = StringMap<string_map_detail::make_index_array<sizeof...(Strings)>(),
                              sizeof...(Strings), Strings...>;
