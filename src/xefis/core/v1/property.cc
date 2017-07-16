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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property.h"


namespace v1 {
using namespace xf;

void
GenericProperty::create (PropertyPath const& path, PropertyType const& type)
{
	std::string const& type_str = type.string();

	if (type_str == "boolean")
		PropertyBoolean (path).ensure_existence();
	else if (type_str == "integer")
		PropertyInteger (path).ensure_existence();
	else if (type_str == "float")
		PropertyFloat (path).ensure_existence();
	else if (type_str == "string")
		PropertyString (path).ensure_existence();
	else if (type_str == "acceleration")
		PropertyAcceleration (path).ensure_existence();
	else if (type_str == "angle")
		PropertyAngle (path).ensure_existence();
	else if (type_str == "area")
		PropertyArea (path).ensure_existence();
	else if (type_str == "charge")
		PropertyCharge (path).ensure_existence();
	else if (type_str == "current")
		PropertyCurrent (path).ensure_existence();
	else if (type_str == "density")
		PropertyDensity (path).ensure_existence();
	else if (type_str == "energy")
		PropertyEnergy (path).ensure_existence();
	else if (type_str == "force")
		PropertyForce (path).ensure_existence();
	else if (type_str == "power")
		PropertyPower (path).ensure_existence();
	else if (type_str == "pressure")
		PropertyPressure (path).ensure_existence();
	else if (type_str == "frequency")
		PropertyFrequency (path).ensure_existence();
	else if (type_str == "angular-velocity")
		Property<AngularVelocity> (path).ensure_existence();
	else if (type_str == "length")
		PropertyLength (path).ensure_existence();
	else if (type_str == "speed")
		PropertySpeed (path).ensure_existence();
	else if (type_str == "temperature")
		PropertyTemperature (path).ensure_existence();
	else if (type_str == "time")
		PropertyTime (path).ensure_existence();
	else if (type_str == "torque")
		PropertyTorque (path).ensure_existence();
	else if (type_str == "volume")
		PropertyVolume (path).ensure_existence();
	else if (type_str == "mass")
		PropertyMass (path).ensure_existence();
	else
		throw BadType (type_str);
}

} // namespace v1

