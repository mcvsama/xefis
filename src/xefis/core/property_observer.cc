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

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property_observer.h"


namespace Xefis {

void
PropertyObserver::observe (GenericProperty& property)
{
	_objects.push_back (Object (&property));
}


void
PropertyObserver::observe (PropertyObserver& observer)
{
	_objects.push_back (Object (&observer));
}


void
PropertyObserver::observe (std::initializer_list<Object> list)
{
	_objects.insert (_objects.end(), list.begin(), list.end());
}


void
PropertyObserver::set_callback (Callback callback)
{
	_callback = callback;
}


void
PropertyObserver::data_updated (Time update_time)
{
	bool updated = false;

	for (Object& o: _objects)
	{
		PropertyNode::Serial new_serial = o.remote_serial();
		if (new_serial != o._saved_serial)
		{
			updated = true;
			o._saved_serial = new_serial;
		}
	}

	if (updated)
	{
		_update_dt = update_time - _update_time;
		_update_time = update_time;
		++_serial;
		_callback();
	}
}

} // namespace Xefis

