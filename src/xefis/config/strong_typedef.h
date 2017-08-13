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

#ifndef XEFIS__CONFIG__STRONG_TYPEDEF_H__INCLUDED
#define XEFIS__CONFIG__STRONG_TYPEDEF_H__INCLUDED

// Standard:
#include <cstddef>


#define XEFIS_STRONG_TYPEDEF(base_type, new_type)		\
	struct new_type: public StrongWrapper<base_type>	\
	{													\
		using StrongWrapper::StrongWrapper;				\
		using StrongWrapper::operator=;					\
	};


namespace xf {

template<class tValue>
	class StrongWrapper
	{
	  public:
		typedef tValue Value;

	  private:
		Value _value;

	  public:
		constexpr
		StrongWrapper() noexcept = default;

		explicit constexpr
		StrongWrapper (Value const& value) noexcept (noexcept (Value (value)));

		explicit constexpr
		StrongWrapper (StrongWrapper const& other) noexcept (noexcept (Value (other._value))) = default;

		StrongWrapper&
		operator= (Value const& value) noexcept (noexcept (_value = value));

		StrongWrapper&
		operator= (StrongWrapper const& other) noexcept (noexcept (_value = other._value));

		bool
		operator== (StrongWrapper const& other) const noexcept (noexcept (_value == other._value));

		bool
		operator< (StrongWrapper const& other) const noexcept (noexcept (_value < other._value));

		Value&
		value();

		Value const&
		value() const;
	};


template<class V>
	constexpr
	StrongWrapper<V>::StrongWrapper (Value const& value) noexcept (noexcept (Value (value))):
		_value (value)
	{ }


template<class V>
	inline StrongWrapper<V>&
	StrongWrapper<V>::operator= (Value const& value) noexcept (noexcept (_value = value))
	{
		_value = value;
		return *this;
	}


template<class V>
	inline StrongWrapper<V>&
	StrongWrapper<V>::operator= (StrongWrapper const& other) noexcept (noexcept (_value = other._value))
	{
		_value = other._value;
		return *this;
	}


template<class V>
	inline bool
	StrongWrapper<V>::operator== (StrongWrapper const& other) const noexcept (noexcept (_value == other._value))
	{
		return _value == other._value;
	}


template<class V>
	inline bool
	StrongWrapper<V>::operator< (StrongWrapper const& other) const noexcept (noexcept (_value < other._value))
	{
		return _value < other._value;
	}


template<class V>
	inline typename StrongWrapper<V>::Value&
	StrongWrapper<V>::value()
	{
		return _value;
	}


template<class V>
	inline typename StrongWrapper<V>::Value const&
	StrongWrapper<V>::value() const
	{
		return _value;
	}

} // namespace xf

namespace std::rel_ops {

struct IncludeRelOps
{ };

} // namespace std::rel_ops

#endif

