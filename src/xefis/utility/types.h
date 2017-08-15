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

#ifndef XEFIS__UTILITY__TYPES_H__INCLUDED
#define XEFIS__UTILITY__TYPES_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/strong_type.h>


namespace xf {

using FontSize = StrongType<float, struct FontSizeType>;


template<size_t Width>
	struct float_for_width
	{ };


template<size_t Width>
	using float_for_width_t = typename float_for_width<Width>::type;


template<>
	struct float_for_width<2>
	{
		using type = float16_t;
	};


template<>
	struct float_for_width<4>
	{
		using type = float32_t;
	};


template<>
	struct float_for_width<8>
	{
		using type = float64_t;
	};


template<>
	struct float_for_width<16>
	{
		using type = float128_t;
	};


template<size_t Width>
	struct int_for_width
	{ };


template<size_t Width>
	using int_for_width_t = typename int_for_width<Width>::type;


template<>
	struct int_for_width<1>
	{
		using type = int8_t;
	};


template<>
	struct int_for_width<2>
	{
		using type = int16_t;
	};


template<>
	struct int_for_width<4>
	{
		using type = int32_t;
	};


template<>
	struct int_for_width<8>
	{
		using type = int64_t;
	};


template<size_t Width>
	struct uint_for_width
	{ };


template<size_t Width>
	using uint_for_width_t = typename uint_for_width<Width>::type;


template<>
	struct uint_for_width<1>
	{
		using type = uint8_t;
	};


template<>
	struct uint_for_width<2>
	{
		using type = uint16_t;
	};


template<>
	struct uint_for_width<4>
	{
		using type = uint32_t;
	};


template<>
	struct uint_for_width<8>
	{
		using type = uint64_t;
	};

} // namespace xf

#endif

