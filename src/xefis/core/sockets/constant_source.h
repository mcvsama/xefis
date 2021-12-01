/* vim:ts=4
 *
 * Copyleft 2021  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SOCKETS__CONSTANT_SOURCE_H__INCLUDED
#define XEFIS__CORE__SOCKETS__CONSTANT_SOURCE_H__INCLUDED

// Standard:
#include <variant>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

/**
 * Wrapper for values that are supposed to act as a constant value source for Socket objects.
 * The value provided by this object is stored in Socket class.
 */
template<class Value>
	class ConstantSource
	{
	  public:
		// Ctor
		explicit
		ConstantSource (Value const& value):
			value (value)
		{ }

		Value value;
	};

} // namespace xf

#endif

