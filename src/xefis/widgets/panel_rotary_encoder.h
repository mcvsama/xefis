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
	PanelRotaryEncoder (QWidget* parent, Panel*, QString const& knob_label, PropertyBoolean rotate_a, PropertyBoolean rotate_b);

	/**
	 * Create clickable rotary encoder.
	 */
	PanelRotaryEncoder (QWidget* parent, Panel*, QString const& knob_label, PropertyBoolean rotate_a, PropertyBoolean rotate_b, PropertyBoolean click_property);

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

  private slots:
	void
	click_timeout();

  private:
	/**
	 * Write to controlled properties.
	 */
	void
	write();

	/**
	 * Apply N Gray code steps. Value is Gray-code.
	 */
	uint8_t
	apply_steps (uint8_t value, int steps);

  private:
	QTimer*			_click_timer			= nullptr;
	QPoint			_mouse_last_position;
	bool			_mouse_pressed			= false;
	uint8_t			_value					= 0; // Gray code
	QString			_knob_label;
	Angle			_angle					= 0_deg;
	PropertyBoolean	_rotate_a;
	PropertyBoolean	_rotate_b;
	PropertyBoolean	_click_property;
};

} // namespace Xefis

#endif

