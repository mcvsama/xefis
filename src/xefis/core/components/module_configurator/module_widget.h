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

#ifndef XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__MODULE_WIDGET_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__MODULE_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QLabel>
#include <QStackedWidget>
#include <QTimer>
#include <QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/machine.h>
#include <xefis/core/components/property_tree/property_tree.h>
#include <xefis/support/ui/histogram_widget.h>

// Local:
#include "module_widget.h"


namespace xf {

/**
 * Configuration widget for module.
 * Contains generic config widgets, module's configurator widget,
 * and other stuff.
 */
class ModuleWidget: public QWidget
{
  public:
	// Ctor
	explicit
	ModuleWidget (BasicModule&, QWidget* parent);

	/**
	 * Return module.
	 */
	BasicModule&
	module() const noexcept;

  private:
	void
	refresh();

	QWidget*
	create_performance_widget();

  private:
	BasicModule&			_module;
	BasicInstrument*		_instrument					{ nullptr };
	PropertyTree*			_inputs_property_tree;
	PropertyTree*			_outputs_property_tree;
	xf::HistogramWidget*	_processing_time_histogram	{ nullptr };
	xf::HistogramWidget*	_painting_time_histogram	{ nullptr };
	QTimer*					_refresh_timer;
};


inline BasicModule&
ModuleWidget::module() const noexcept
{
	return _module;
}

} // namespace xf

#endif

