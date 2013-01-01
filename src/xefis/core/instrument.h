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

#ifndef XEFIS__CORE__INSTRUMENT_H__INCLUDED
#define XEFIS__CORE__INSTRUMENT_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>


namespace Xefis {

class Instrument:
	public Module,
	public QWidget
{
  public:
	// Ctor
	Instrument (QWidget* parent);
};


inline
Instrument::Instrument (QWidget* parent):
	QWidget (parent)
{ }

} // namespace Xefis

#endif

