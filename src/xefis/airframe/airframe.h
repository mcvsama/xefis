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

#ifndef XEFIS__CORE__AIRFRAME__AIRFRAME_H__INCLUDED
#define XEFIS__CORE__AIRFRAME__AIRFRAME_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/airframe/flaps.h>


namespace Xefis {

class Application;


/**
 * Contains submodules that describe an airframe.
 */
class Airframe
{
  public:
	// Ctor
	Airframe (Application* application, QDomElement const& config);

  public:
	Flaps const&
	flaps() const;

  private:
	Unique<Flaps>	_flaps;
};


inline Flaps const&
Airframe::flaps() const
{
	if (!_flaps)
		throw BadConfiguration ("flaps submodule not configured");
	return *_flaps;
}

} // namespace Xefis

#endif

