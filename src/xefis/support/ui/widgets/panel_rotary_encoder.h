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

#ifndef XEFIS__SUPPORT__UI__WIDGETS__PANEL_ROTARY_ENCODER_H__INCLUDED
#define XEFIS__SUPPORT__UI__WIDGETS__PANEL_ROTARY_ENCODER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QTimer>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/property.h>
#include <xefis/support/ui/widgets/panel_widget.h>


namespace xf {

class PanelRotaryEncoder: public PanelWidget
{
	static constexpr QSize	KnobSize	= { 40, 40 };
	static constexpr int	Notches		= 12;

	Q_OBJECT

  public:
	/**
	 * Create simple rotary encoder.
	 * rotate_a and rotate_b correspond to real rotary encoder outputs (using Gray code).
	 */
	explicit
	PanelRotaryEncoder (QWidget* parent, Panel*, QString const& knob_label,
						v1::PropertyInteger value_property, v1::PropertyBoolean click_property);

  protected:
	void
	paintEvent (QPaintEvent*) override;

	void
	mousePressEvent (QMouseEvent*) override;

	void
	mouseReleaseEvent (QMouseEvent*) override;

	void
	mouseMoveEvent (QMouseEvent*) override;

	void
	wheelEvent (QWheelEvent*) override;

	void
	mouseDoubleClickEvent (QMouseEvent*) override;

  private:
	/**
	 * Write to controlled properties.
	 */
	void
	write();

  private:
	Unique<QTimer>				_click_timer;
	QPoint						_mouse_last_position;
	bool						_mouse_pressed			= false;
	QString						_knob_label;
	si::Angle					_angle					= 0_deg;
	v1::PropertyInteger::Type	_value					= 0;
	v1::PropertyInteger			_value_property;
	v1::PropertyBoolean			_click_property;
};

} // namespace xf

#endif

