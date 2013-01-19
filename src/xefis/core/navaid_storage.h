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

#ifndef XEFIS__CORE__NAV_STORAGE_H__INCLUDED
#define XEFIS__CORE__NAV_STORAGE_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "navaid.h"


class NavaidStorage
{
  public:
	typedef std::set<Navaid> Navaids;

  public:
	NavaidStorage();

	Navaids
	get_navs (LatLng const& position, Miles radius) const;

  private:
	Navaids _navaids;
};

#endif

