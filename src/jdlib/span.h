// SPDX-License-Identifier: GPL-2.0-only
/**
 * @file span.h
 * @brief C++20 std::span のサブセットを実装するヘッダー
 */
#ifndef JDLIB_SPAN_H
#define JDLIB_SPAN_H

#include <array>
#include <iterator>
#include <cassert>
#include <type_traits>


namespace JDLIB
{

/**
 * @brief Subset of C++20 std::span.
 *
 * 配列型のビュークラスで所有権を持たず参照するオブジェクトの寿命は管理しない。
 * - 順方向イテレーターはポインターを使って実装
 * - テンプレート引数の整数によるサイズ指定は未実装
 */
template<typename T>
class span
{
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    /// same as pointer type
    using iterator = T*;
    using reverse_iterator = std::reverse_iterator<iterator>;

private:
    pointer m_data;
    size_type m_size;

public:
    constexpr span() noexcept
        : m_data{}
        , m_size{}
    {}
    template<typename Ptr>
    constexpr span( Ptr data, size_type count ) noexcept
        : m_data{ data }
        , m_size{ count }
    {}
    template<std::size_t N>
    constexpr span( element_type (&arr)[N] ) noexcept
        : m_data{ arr }
        , m_size{ N }
    {}
    template<typename U, std::size_t N>
    constexpr span( std::array<U, N>& arr ) noexcept
        : m_data{ arr.data() }
        , m_size{ arr.size() }
    {}
    template<typename U, std::size_t N>
    constexpr span( const std::array<U, N>& arr ) noexcept
        : m_data{ arr.data() }
        , m_size{ arr.size() }
    {}
    template<typename U>
    constexpr span( U&& container )
        : m_data{ std::data( container ) }
        , m_size{ std::size( container ) }
    {}
    constexpr span( const span& other ) noexcept = default;

    constexpr span& operator=( const span& other ) noexcept = default;

    constexpr iterator begin() const { return m_data; }
    constexpr iterator end() const { return m_data + m_size; }
    constexpr reverse_iterator rbegin() const { return reverse_iterator{ m_data + m_size }; }
    constexpr reverse_iterator rend() const { return reverse_iterator{ m_data }; }

    constexpr reference front() const { assert( m_size > 0 ); return m_data[0]; }
    constexpr reference back() const { assert( m_size > 0 ); return m_data[m_size - 1]; }
    constexpr reference operator[]( size_type i ) const { assert( i <= m_size ); return m_data[i]; }
    constexpr pointer data() const noexcept { return m_data; }

    constexpr size_type size() const noexcept { return m_size; }
    [[nodiscard]]
    constexpr bool empty() const noexcept { return m_size == 0; }

    constexpr span first( size_type count ) const
    {
        assert( count <= m_size );
        return span{ m_data, count };
    }
    constexpr span last( size_type count ) const
    {
        assert( count <= m_size );
        return span{ m_data + m_size - count, count };
    }
    constexpr span subspan( size_type offset, size_type count ) const
    {
        assert( offset + count <= m_size );
        return span{ m_data + offset, count };
    }
};

} // namespace JDLIB
#endif // JDLIB_SPAN_H
