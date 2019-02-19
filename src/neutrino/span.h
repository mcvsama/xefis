/* vim:ts=4
 *
 * Copyleft 2012…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef NEUTRINO__SPAN_H__INCLUDED
#define NEUTRINO__SPAN_H__INCLUDED

// Standard:
#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <vector>


namespace neutrino {

template<class pElement>
	class Span
	{
	  public:
		using Element			= pElement;
		using Value				= std::remove_cv_t<pElement>;

		// Standard compatibility:
		using element_type		= Element;
		using value_type		= std::remove_cv_t<Element>;
		using size_type			= std::size_t;
		using difference_type	= std::ptrdiff_t;
		using reference			= element_type&;
		using const_reference	= element_type const&;
		using pointer			= element_type*;
		using const_pointer		= element_type const*;

	  public:
		// Ctor
		constexpr
		Span() = default;

		// Ctor
		constexpr
		Span (Span const&) = default;

		// Ctor
		constexpr
		Span (element_type* begin, element_type* end):
			_data (begin),
			_size (std::distance (begin, end))
		{ }

		// Ctor
		constexpr
		Span (element_type* data, size_type size):
			_data (data),
			_size (size)
		{ }

		// Ctor
		template<std::size_t N>
			constexpr
			Span (Value (&data)[N]):
				_data (data),
				_size (N)
			{ }

		// Ctor
		template<std::size_t N,
				 class = std::enable_if_t<std::is_const_v<Element>>>
			constexpr
			Span (std::array<Value, N> const& array):
				_data (array.data()),
				_size (array.size())
			{ }

		// Ctor
		template<std::size_t N,
				 class = std::enable_if_t<!std::is_const_v<Element>>>
			constexpr
			Span (std::array<Value, N>& array):
				_data (array.data()),
				_size (array.size())
			{ }

		// Ctor
		template<class = std::enable_if_t<std::is_const_v<Element>>>
			constexpr
			Span (std::vector<Value> const& vector):
				_data (vector.data()),
				_size (vector.size())
			{ }

		// Ctor
		template<class = std::enable_if_t<!std::is_const_v<Element>>>
			constexpr
			Span (std::vector<Value>& vector):
				_data (vector.data()),
				_size (vector.size())
			{ }

		constexpr pointer
		begin() noexcept;

		constexpr const_pointer
		cbegin() const noexcept;

		constexpr pointer
		end() noexcept;

		constexpr const_pointer
		cend() const noexcept;

		constexpr reference
		operator[] (size_type pos) noexcept;

		constexpr const_reference
		operator[] (size_type pos) const noexcept;

		constexpr reference
		front();

		constexpr const_reference
		front() const;

		constexpr reference
		back();

		constexpr const_reference
		back() const;

		constexpr pointer
		data();

		constexpr const_pointer
		data() const;

		constexpr bool
		empty() const;

		constexpr size_type
		size() const;

		constexpr size_type
		max_size() const;

		constexpr void
		remove_prefix (size_type n);

		constexpr void
		remove_suffix (size_type n);

		/**
		 * Fill array with given value.
		 */
		constexpr void
		fill (const_reference value);

	  private:
		element_type*	_data	= nullptr;
		size_type		_size	= 0;
	};


template<class V>
	constexpr auto
	Span<V>::begin() noexcept -> pointer
	{
		return _data;
	}


template<class V>
	constexpr auto
	Span<V>::cbegin() const noexcept -> const_pointer
	{
		return _data;
	}


template<class V>
	constexpr auto
	Span<V>::end() noexcept -> pointer
	{
		return _data + _size;
	}


template<class V>
	constexpr auto
	Span<V>::cend() const noexcept -> const_pointer
	{
		return _data + _size;
	}


template<class V>
	constexpr auto
	Span<V>::operator[] (size_type pos) noexcept -> reference
	{
		return _data[pos];
	}


template<class V>
	constexpr auto
	Span<V>::operator[] (size_type pos) const noexcept -> const_reference
	{
		return _data[pos];
	}


template<class V>
	constexpr auto
	Span<V>::front() -> reference
	{
		return _data[0];
	}


template<class V>
	constexpr auto
	Span<V>::front() const -> const_reference
	{
		return _data[0];
	}


template<class V>
	constexpr auto
	Span<V>::back() -> reference
	{
		return _data[size() - 1];
	}


template<class V>
	constexpr auto
	Span<V>::back() const -> const_reference
	{
		return _data[size() - 1];
	}


template<class V>
	constexpr auto
	Span<V>::data() -> pointer
	{
		return _data;
	}


template<class V>
	constexpr auto
	Span<V>::data() const -> const_pointer
	{
		return _data;
	}


template<class V>
	constexpr bool
	Span<V>::empty() const
	{
		return size() == 0;
	}


template<class V>
	constexpr auto
	Span<V>::size() const -> size_type
	{
		return _size;
	}


template<class V>
	constexpr auto
	Span<V>::max_size() const -> size_type
	{
		return _size;
	}


template<class V>
	constexpr void
	Span<V>::remove_prefix (size_type n)
	{
		_data += n;
		_size -= n;
	}


template<class V>
	constexpr void
	Span<V>::remove_suffix (size_type n)
	{
		_size -= n;
	}


template<class V>
	constexpr void
	Span<V>::fill (const_reference value)
	{
		for (size_type i = 0; i < size(); ++i)
			_data[i] = value;
	}

} // namespace neutrino

#endif

