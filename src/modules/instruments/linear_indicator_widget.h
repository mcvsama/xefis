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

#ifndef XEFIS__MODULES__INSTRUMENTS__LINEAR_INDICATOR_WIDGET_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__LINEAR_INDICATOR_WIDGET_H__INCLUDED

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


class LinearIndicatorWidget:
	public Xefis::InstrumentWidget,
	public Xefis::InstrumentAids
{
  public:
	// Ctor
	LinearIndicatorWidget (QWidget* parent);

	/**
	 * Set mirrored style.
	 */
	void
	set_mirrored_style (bool);

	/**
	 * Set new range to be used with LinearIndicatorWidget.
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

	/**
	 * Set number of digits displayed.
	 */
	void
	set_digits (unsigned int digits);

	void
	set_value (Optional<double>);

	void
	set_minimum_critical_value (Optional<double>);

	void
	set_minimum_warning_value (Optional<double>);

	void
	set_maximum_warning_value (Optional<double>);

	void
	set_maximum_critical_value (Optional<double>);

	void
	set_normal_value (Optional<double>);

	void
	set_target_value (Optional<double>);

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

	QString
	stringify_value (double value) const;

	QString
	pad_string (QString const& input) const;

  private:
	Xefis::TextPainter::Cache
						_text_painter_cache;
	// Parameters:
	bool					_mirrored			= false;
	Xefis::Range<double>	_range				= { 0.f, 1.f };
	int						_precision			= 0;
	unsigned int			_modulo				= 0;
	unsigned int			_digits				= 3;
	Optional<double>		_value;
	Optional<double>		_minimum_critical_value;
	Optional<double>		_minimum_warning_value;
	Optional<double>		_maximum_warning_value;
	Optional<double>		_maximum_critical_value;
	Optional<double>		_normal_value;
	Optional<double>		_target_value;
};


inline void
LinearIndicatorWidget::set_mirrored_style (bool mirrored)
{
	_mirrored = mirrored;
	update();
}


inline void
LinearIndicatorWidget::set_range (Xefis::Range<double> range)
{
	_range = range;
	update();
}


inline void
LinearIndicatorWidget::set_precision (int precision)
{
	_precision = precision;
	update();
}


inline void
LinearIndicatorWidget::set_modulo (unsigned int modulo)
{
	_modulo = modulo;
	update();
}


inline void
LinearIndicatorWidget::set_digits (unsigned int digits)
{
	_digits = digits;
	update();
}


inline void
LinearIndicatorWidget::set_value (Optional<double> value)
{
	_value = value;
	update();
}


inline void
LinearIndicatorWidget::set_minimum_critical_value (Optional<double> value)
{
	_minimum_critical_value = value;
	update();
}


inline void
LinearIndicatorWidget::set_minimum_warning_value (Optional<double> value)
{
	_minimum_warning_value = value;
	update();
}


inline void
LinearIndicatorWidget::set_maximum_warning_value (Optional<double> value)
{
	_maximum_warning_value = value;
	update();
}


inline void
LinearIndicatorWidget::set_maximum_critical_value (Optional<double> value)
{
	_maximum_critical_value = value;
	update();
}


inline void
LinearIndicatorWidget::set_normal_value (Optional<double> value)
{
	_normal_value = value;
	update();
}


inline void
LinearIndicatorWidget::set_target_value (Optional<double> value)
{
	_target_value = value;
	update();
}

#endif

