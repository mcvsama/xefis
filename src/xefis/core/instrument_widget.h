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

#ifndef XEFIS__CORE__INSTRUMENT_WIDGET_H__INCLUDED
#define XEFIS__CORE__INSTRUMENT_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QFont>
#include <QtGui/QColor>
#include <QtGui/QPen>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

class InstrumentWidget: public QWidget
{
  public:
	// Ctor
	InstrumentWidget (QWidget* parent);
};

} // namespace Xefis

#endif

