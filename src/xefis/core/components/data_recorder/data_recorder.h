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

#ifndef XEFIS__CORE__COMPONENTS__DATA_RECORDER__DATA_RECORDER_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__DATA_RECORDER__DATA_RECORDER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtWidgets/QScrollArea>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "graphs_stack.h"


namespace xf {

class DataRecorder: public QWidget
{
  public:
	// Ctor
	explicit
	DataRecorder (QWidget* parent);

  private:
	GraphsStack*	_graphs_stack;
	QScrollArea*	_scroll_area;
};

} // namespace xf

#endif

