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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/socket.h>
#include <xefis/support/ui/widgets/panel_widget.h>

// Qt:
#include <QtCore/QTimer>
#include <QtWidgets/QWidget>

// Standard:
#include <cstddef>
#include <memory>


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
						Socket<int64_t> value_socket, Socket<bool> click_socket);

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
	 * Write to controlled sockets.
	 */
	void
	write();

  private:
	std::unique_ptr<QTimer>		_click_timer;
	QPoint						_mouse_last_position;
	bool						_mouse_pressed			{ false };
	QString						_knob_label;
	si::Angle					_angle					{ 0_deg };
	Socket<int64_t>::Type		_value					{ 0 };
	Socket<int64_t>				_value_socket;
	Socket<bool>				_click_socket;
};

} // namespace xf

#endif

