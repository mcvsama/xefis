/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef NEUTRINO__SEQUENCE_H__INCLUDED
#define NEUTRINO__SEQUENCE_H__INCLUDED

// Standard:
#include <cstddef>


namespace neutrino {

// TODO optional value-mapping function, to eg. take out raw pointer from unique_ptrs
template<class pIterator>
	class Sequence
	{
	  public:
		using Iterator = pIterator;

	  public:
		// Ctor
		Sequence (Iterator begin, Iterator end);

		constexpr Iterator
		begin() const noexcept;

		constexpr Iterator
		end() const noexcept;

	  private:
		Iterator const	_begin;
		Iterator const	_end;
	};


template<class I>
	inline
	Sequence<I>::Sequence (Iterator begin, Iterator end):
		_begin (begin),
		_end (end)
	{ }


template<class I>
	constexpr auto
	Sequence<I>::begin() const noexcept -> Iterator
	{
		return _begin;
	}


template<class I>
	constexpr auto
	Sequence<I>::end() const noexcept -> Iterator
	{
		return _end;
	}

} // namespace neutrino

#endif

