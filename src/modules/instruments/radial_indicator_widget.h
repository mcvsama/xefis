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
	set_range (Xefis::Range<float>);

	/**
	 * Set indicated power value.
	 */
	void
	set_value (float value);

	/**
	 * Set validity of the input value.
	 */
	void
	set_value_visible (bool visible);

	/**
	 * Set warning value. This starts the section that is drawn in yellow.
	 */
	void
	set_warning_value (float);

	/**
	 * Set visibility of the warning bug.
	 */
	void
	set_warning_visible (bool visible);

	/**
	 * Set critical value.
	 */
	void
	set_critical_value (float);

	/**
	 * Set visibility of the critical bug.
	 */
	void
	set_critical_visible (bool visible);

	/**
	 * Set normal/desired value. This sets the green bug position on the scale.
	 */
	void
	set_normal_value (float);

	/**
	 * Set visibility of the normal/desired value bug.
	 */
	void
	set_normal_visible (bool visible);

	/**
	 * Set target value (setting bug).
	 */
	void
	set_target_value (float);

	/**
	 * Set target value visibility.
	 */
	void
	set_target_visible (bool visible);

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

	void
	paint_text (Xefis::Painter&, float q, float r);

	void
	paint_indicator (Xefis::Painter&, float q, float r);

  private:
	Xefis::TextPainter::Cache
						_text_painter_cache;
	// Parameters:
	Xefis::Range<float>	_range				= { 0.f, 1.f };
	float				_value				= 0.f;
	bool				_value_visible		= false;
	float				_warning_value		= 1.f;
	bool				_warning_visible	= false;
	float				_critical_value		= 1.f;
	bool				_critical_visible	= false;
	float				_normal_value		= 1.f;
	bool				_normal_visible		= false;
	float				_target_value		= 0.f;
	bool				_target_visible		= false;
};


inline void
RadialIndicatorWidget::set_range (Xefis::Range<float> range)
{
	_range = range;
	update();
}


inline void
RadialIndicatorWidget::set_value (float value)
{
	_value = value;
	update();
}


inline void
RadialIndicatorWidget::set_value_visible (bool visible)
{
	_value_visible = visible;
	update();
}


inline void
RadialIndicatorWidget::set_warning_value (float warning_value)
{
	_warning_value = warning_value;
	update();
}


inline void
RadialIndicatorWidget::set_warning_visible (bool visible)
{
	_warning_visible = visible;
	update();
}


inline void
RadialIndicatorWidget::set_critical_value (float critical_value)
{
	_critical_value = critical_value;
	update();
}


inline void
RadialIndicatorWidget::set_critical_visible (bool visible)
{
	_critical_visible = visible;
	update();
}


inline void
RadialIndicatorWidget::set_normal_value (float normal_value)
{
	_normal_value = normal_value;
	update();
}


inline void
RadialIndicatorWidget::set_normal_visible (bool visible)
{
	_normal_visible = visible;
	update();
}


inline void
RadialIndicatorWidget::set_target_value (float target_value)
{
	_target_value = target_value;
	update();
}


inline void
RadialIndicatorWidget::set_target_visible (bool visible)
{
	_target_visible = visible;
	update();
}

#endif

