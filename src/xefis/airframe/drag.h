/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__AIRFRAME__DRAG_H__INCLUDED
#define XEFIS__CORE__AIRFRAME__DRAG_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/airframe/types.h>
#include <xefis/utility/datatable2d.h>


namespace Xefis {

class Drag
{
  public:
	// Ctor
	Drag (QDomElement const& config);

	/**
	 * Return drag coefficient (C_D) for given angle of attack.
	 *
	 * Uses linear interpolation.
	 */
	DragCoefficient
	get_cd (Angle const& aoa) const;

  private:
	Unique<Datatable2D<Angle, DragCoefficient>>	_aoa_to_cd;
};

} // namespace Xefis

#endif

