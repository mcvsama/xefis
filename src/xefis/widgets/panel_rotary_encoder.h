/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__WIDGETS__PANEL_ROTARY_ENCODER_H__INCLUDED
#define XEFIS__WIDGETS__PANEL_ROTARY_ENCODER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QTimer>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/widgets/panel_widget.h>


namespace Xefis {

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
	PanelRotaryEncoder (QWidget* parent, Panel*, QString const& knob_label,
						PropertyBoolean rotate_a, PropertyBoolean rotate_b,
						PropertyBoolean rotate_up, PropertyBoolean rotate_down,
						PropertyBoolean click_property);

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

	/**
	 * "Press" up or down buttons according to rotation delta.
	 */
	void
	rotate (int delta);

	/**
	 * Apply N Gray code steps. Value is Gray-code.
	 */
	uint8_t
	apply_steps (uint8_t value, int steps);

  private:
	Unique<QTimer>	_click_timer;
	Unique<QTimer>	_rotate_up_timer;
	Unique<QTimer>	_rotate_down_timer;
	QPoint			_mouse_last_position;
	bool			_mouse_pressed			= false;
	uint8_t			_value					= 0; // Gray code
	QString			_knob_label;
	Angle			_angle					= 0_deg;
	PropertyBoolean	_rotate_a;
	PropertyBoolean	_rotate_b;
	PropertyBoolean	_rotate_up;
	PropertyBoolean	_rotate_down;
	PropertyBoolean	_click_property;
};

} // namespace Xefis

#endif

