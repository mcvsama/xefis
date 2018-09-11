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

#ifndef XEFIS__CORE__PROPERTY_CONVERTER_H__INCLUDED
#define XEFIS__CORE__PROPERTY_CONVERTER_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Boost:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

class PropertyConversionSettings
{
  public:
	static constexpr const char kDefaultNilValue[] = u8"∅";

  public:
	// Common settings:
	std::string						nil_value				{ kDefaultNilValue };

	// Bool properties:
	std::string						true_value				{ "true" };
	std::string						false_value				{ "false" };

	// int64_t, double and SI properties:
	boost::format					numeric_format			{ "%1%" };

	// Preferred unit:
	std::vector<si::DynamicUnit>	preferred_units;
};

} // namespace xf

#endif

