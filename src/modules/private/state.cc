/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include <memory>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/exception.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "state.h"


State::State (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager)
{
	for (QDomElement const& e: config)
	{
		if (e == "property")
		{
			QString type = e.attribute ("type");

			if (type == "boolean")
				_managed_properties.insert (new ManagedBoolean (e));
			else if (type == "integer")
				_managed_properties.insert (new ManagedInteger (e));
			else if (type == "float")
				_managed_properties.insert (new ManagedFloat (e));
			else if (type == "string")
				_managed_properties.insert (new ManagedString (e));
			// SI properties:
			else if (type == "angle")
				_managed_properties.insert (new ManagedSIProperty<Xefis::PropertyAngle> (e));
			else if (type == "pressure")
				_managed_properties.insert (new ManagedSIProperty<Xefis::PropertyPressure> (e));
			else if (type == "frequency")
				_managed_properties.insert (new ManagedSIProperty<Xefis::PropertyFrequency> (e));
			else if (type == "length")
				_managed_properties.insert (new ManagedSIProperty<Xefis::PropertyLength> (e));
			else if (type == "time")
				_managed_properties.insert (new ManagedSIProperty<Xefis::PropertyTime> (e));
			else if (type == "speed")
				_managed_properties.insert (new ManagedSIProperty<Xefis::PropertySpeed> (e));
		}
		else if (e == "var")
			_vars[e.attribute ("name")] = e.attribute ("path");
		else if (e == "observe")
			_observed_properties.insert (new ObservedProperty (this, e));
	}
}

