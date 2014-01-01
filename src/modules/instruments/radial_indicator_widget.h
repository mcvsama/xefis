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

#ifndef XEFIS__MODULES__INSTRUMENTS__RADIAL_INDICATOR_WIDGET_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__RADIAL_INDICATOR_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QPaintEvent>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument_widget.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/utility/painter.h>
#include <xefis/utility/range.h>


class RadialIndicatorWidget:
	public Xefis::InstrumentWidget,
	public Xefis::InstrumentAids
{
  public:
	// Ctor
	RadialIndicatorWidget (QWidget* parent);

	/**
	 * Set new range to be used with RadialIndicatorWidget.
	 */
	void
	set_range (Xefis::Range<double>);

	/**
	 * Set precision (digits after decimal point).
	 * Negative values are accepted (value will be divided by 10^n).
	 */
	void
	set_precision (int precision);

	/**
	 * Set modulo value. If > 0, value will be converted to int,
	 * divided by n and the multipled by n again.
	 */
	void
	set_modulo (unsigned int modulo);

	void
	set_value (Optional<double>);

	void
	set_warning_value (Optional<double>);

	void
	set_critical_value (Optional<double>);

	void
	set_reference_value (Optional<double>);

	void
	set_target_value (Optional<double>);

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

	QString
	stringify_value (double value) const;

	void
	paint_text (float q, float r);

	void
	paint_indicator (float q, float r);

  private:
	Xefis::Range<double>		_range				= { 0.f, 1.f };
	int							_precision			= 0;
	int							_modulo				= 0;
	Optional<double>			_value;
	Optional<double>			_warning_value;
	Optional<double>			_critical_value;
	Optional<double>			_reference_value;
	Optional<double>			_target_value;
};


inline void
RadialIndicatorWidget::set_range (Xefis::Range<double> range)
{
	_range = range;
	update();
}


inline void
RadialIndicatorWidget::set_precision (int precision)
{
	_precision = precision;
	update();
}


inline void
RadialIndicatorWidget::set_modulo (unsigned int modulo)
{
	_modulo = modulo;
	update();
}


inline void
RadialIndicatorWidget::set_value (Optional<double> value)
{
	_value = value;
	update();
}


inline void
RadialIndicatorWidget::set_warning_value (Optional<double> warning_value)
{
	_warning_value = warning_value;
	update();
}


inline void
RadialIndicatorWidget::set_critical_value (Optional<double> critical_value)
{
	_critical_value = critical_value;
	update();
}


inline void
RadialIndicatorWidget::set_reference_value (Optional<double> reference_value)
{
	_reference_value = reference_value;
	update();
}


inline void
RadialIndicatorWidget::set_target_value (Optional<double> target_value)
{
	_target_value = target_value;
	update();
}

#endif

