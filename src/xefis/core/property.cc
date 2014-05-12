/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property.h"


namespace Xefis {

void
TypedProperty::create (std::string const& path, std::string const& type)
{
	if (type == "boolean")
		PropertyBoolean (path).ensure_existence();
	else if (type == "integer")
		PropertyInteger (path).ensure_existence();
	else if (type == "float")
		PropertyFloat (path).ensure_existence();
	else if (type == "string")
		PropertyString (path).ensure_existence();
	else if (type == "acceleration")
		PropertyAcceleration (path).ensure_existence();
	else if (type == "angle")
		PropertyAngle (path).ensure_existence();
	else if (type == "capacity")
		PropertyCapacity (path).ensure_existence();
	else if (type == "current")
		PropertyCurrent (path).ensure_existence();
	else if (type == "pressure")
		PropertyPressure (path).ensure_existence();
	else if (type == "frequency")
		PropertyFrequency (path);
	else if (type == "length")
		PropertyLength (path).ensure_existence();
	else if (type == "speed")
		PropertySpeed (path).ensure_existence();
	else if (type == "temperature")
		PropertyTemperature (path).ensure_existence();
	else if (type == "time")
		PropertyTime (path).ensure_existence();
	else if (type == "weight")
		PropertyWeight (path).ensure_existence();
	else
		throw BadType (type);
}

} // namespace Xefis

