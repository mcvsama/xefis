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

#ifndef NEUTRINO__TYPES_H__INCLUDED
#define NEUTRINO__TYPES_H__INCLUDED

// Standard:
#include <cstddef>
#include <type_traits>

// Neutrino:
#include <neutrino/c++20.h>
#include <neutrino/core_types.h>
#include <neutrino/strong_type.h>


namespace neutrino {

using FontPixelSize = StrongType<float, struct FontPixelSizeType>;


/**
 * Gives corresponding floating-point type (half, float, double, long double…) for required byte-width
 * (number of bytes in the type).
 */
template<size_t Width>
	struct float_for_width
	{ };


template<size_t Width>
	using float_for_width_t = typename float_for_width<Width>::type;


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


/**
 * Gives corresponding integer type (signed char, short, int, …) for required byte-width
 * (number of bytes in the type).
 */
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


/*
 * A set of meta-functions checking if given type allows addition/subtraction/multiplication/division.
 */


template<class,
		 class = std::void_t<>>
	struct is_additive: public std::false_type
	{ };


template<class T>
	struct is_additive<T, std::void_t<decltype(std::declval<T>() + std::declval<T>())>>: public std::true_type
	{ };


template<class T>
	constexpr bool is_additive_v = is_additive<T>::value;


template<class,
		 class = std::void_t<>>
	struct is_substractive: public std::false_type
	{ };


template<class T>
	struct is_substractive<T, std::void_t<decltype(std::declval<T>() - std::declval<T>())>>: public std::true_type
	{ };


template<class T>
	constexpr bool is_substractive_v = is_substractive<T>::value;


template<class,
		 class = std::void_t<>>
	struct is_multiplicative: public std::false_type
	{ };


template<class T>
	struct is_multiplicative<T, std::void_t<decltype(std::declval<T>() * std::declval<T>())>>: public std::true_type
	{ };


template<class T>
	constexpr bool is_multiplicative_v = is_multiplicative<T>::value;


template<class,
		 class = std::void_t<>>
	struct is_divisible: public std::false_type
	{ };


template<class T>
	struct is_divisible<T, std::void_t<decltype(std::declval<T>() / std::declval<T>())>>: public std::true_type
	{ };


template<class T>
	constexpr bool is_divisible_v = is_divisible<T>::value;


/*
 * is_algebraic<T>
 * Check if T allows +, -, *, / operations.
 */


template<class T,
		 class Result = std::conditional_t<is_additive<T>() && is_substractive<T>() && is_multiplicative<T>() && is_divisible<T>(), std::true_type, std::false_type>>
	struct is_algebraic: public Result
	{ };


template<class T>
	constexpr bool is_algebraic_v = is_algebraic<T>::value;


/*
 * is_specialization<ConcreteType, TemplateName>
 */


template<class ConcreteType, template<class...> class Template>
	struct is_specialization: public std::false_type
	{ };


template<template<class...> class Template, class ...Args>
	struct is_specialization<Template<Args...>, Template>: public std::true_type
	{ };


template<class ConcreteType, template<class...> class Template>
	constexpr bool is_specialization_v = is_specialization<ConcreteType, Template>::value;


/*
 * TupleSlice, tuple_slice
 */


namespace detail
{

template<std::size_t Ofst, class Tuple, std::size_t... I>
	constexpr auto
	tuple_slice_impl (Tuple&& t, std::index_sequence<I...>)
	{
		return std::forward_as_tuple (std::get<I + Ofst> (std::forward<Tuple> (t))...);
	}


template<class... Ts>
	using TupleWithRemovedCVRefs = std::tuple<std::remove_cvref_t<Ts>...>;


template<class... Ts>
	constexpr auto
	remove_ref_from_tuple_members (std::tuple<Ts...> const& t)
	{
		return TupleWithRemovedCVRefs<Ts...> { t };
	}

}


template<std::size_t Begin, std::size_t End, class Tuple>
	constexpr auto
	tuple_slice (Tuple&& t)
	{
		static_assert (End >= Begin, "invalid slice");
		static_assert (std::tuple_size_v<std::decay_t<Tuple>> >= End, "slice index out of bounds");

		return detail::tuple_slice_impl<Begin> (std::forward<Tuple> (t), std::make_index_sequence<End - Begin>{});
	}


template<std::size_t Begin, std::size_t End, class Tuple>
	struct TupleSliceH
	{
		using type = std::remove_cvref_t<decltype (detail::remove_ref_from_tuple_members (tuple_slice<Begin, End> (std::declval<Tuple>())))>;
	};


template<std::size_t Begin, std::size_t End, class Tuple>
	using TupleSlice = typename TupleSliceH<Begin, End, Tuple>::type;


/*
 * TupleCat
 */


template<class Tuple1, class Tuple2>
	using TupleCat = decltype (std::tuple_cat (std::declval<Tuple1>(), std::declval<Tuple2>()));

} // namespace neutrino

#endif

