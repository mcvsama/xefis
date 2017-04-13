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

#ifndef XEFIS__CORE__V2__INSTRUMENT_H__INCLUDED
#define XEFIS__CORE__V2__INSTRUMENT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Qt:
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>


namespace v2 {
using namespace xf; // XXX

class Instrument:
	public Module,
	public QWidget // XXX will not be a widget in the future - instead it will draw on a canvas.
{
  public:
	// Ctor
	explicit
	Instrument (std::string const& instance = {});
};

} // namespace v2

#endif

