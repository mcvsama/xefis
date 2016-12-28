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

#ifndef XEFIS__SUPPORT__AIRFRAME__SPOILERS_H__INCLUDED
#define XEFIS__SUPPORT__AIRFRAME__SPOILERS_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/range.h>
#include <xefis/airframe/lift_mod.h>


namespace xf {

class Spoilers: public LiftMod
{
  public:
	// Ctor
	using LiftMod::LiftMod;
};

} // namespace xf

#endif

