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

// Standard:
#include <cstddef>

// Lib:
#include <QBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/widget.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/kde.h>
#include <xefis/utility/range.h>

// Local:
#include "virtual_joystick.h"


static constexpr auto kTransparentStyleSheet =
	"QWidget		{ background: rgba(220, 220, 220, 0.3); } "
	"QWidget:hover	{ background: rgba(255, 255, 255, 0.6); } ";


struct MouseControl
{
	QPointF	starting_point;
	QPointF	current_point;

	QPointF
	delta() const noexcept;
};


class VirtualJoystickWidget: public xf::Widget
{
  public:
	// Ctor
	explicit
	VirtualJoystickWidget (QWidget* parent);

	/**
	 * Return current joystick axes position. Both x and y
	 * are in range of [-1, 1].
	 */
	QPointF
	position() const noexcept;

  protected:
	// QWidget API
	void
	paintEvent (QPaintEvent*) override;

	// QWidget API
	void
	mousePressEvent (QMouseEvent*) override;

	// QWidget API
	void
	mouseReleaseEvent (QMouseEvent*) override;

	// QWidget API
	void
	mouseMoveEvent (QMouseEvent*) override;

  private:
	std::optional<MouseControl>	_control;
};


class VirtualThrottleWidget: public xf::Widget
{
  public:
	// Ctor
	explicit
	VirtualThrottleWidget (QWidget* parent);

	/**
	 * Value is in range [0, 1].
	 */
	float
	throttle() const noexcept;

  protected:
	// QWidget API
	void
	paintEvent (QPaintEvent*) override;

	// QWidget API
	void
	mousePressEvent (QMouseEvent*) override;

	// QWidget API
	void
	mouseReleaseEvent (QMouseEvent*) override;

	// QWidget API
	void
	mouseMoveEvent (QMouseEvent*) override;

  private:
	double						_throttle	{ 0.0 };
	std::optional<MouseControl>	_control;
};


QPointF
MouseControl::delta() const noexcept
{
	return current_point - starting_point;
}


VirtualJoystickWidget::VirtualJoystickWidget (QWidget* parent):
	Widget (parent)
{
	setStyleSheet (kTransparentStyleSheet);
	setFixedSize (em_pixels (20), em_pixels (20));
}


QPointF
VirtualJoystickWidget::position() const noexcept
{
	if (_control)
	{
		using std::abs;
		using xf::sgn;

		auto const size = std::min (width(), height());
		auto const pos = _control->delta() / (0.5 * size);

		if (abs (pos.x()) > 1.0 || abs (pos.y()) > 1.0)
		{
			auto const sx = sgn (pos.x());
			auto const sy = sgn (pos.y());

			if (abs (pos.x()) > abs (pos.y()))
				return QPointF (sx, -sx / pos.x() * pos.y());
			else
				return QPointF (sy / pos.y() * pos.x(), -sy);
		}
		else
			return QPointF (pos.x(), -pos.y());
	}
	else
		return { 0.0, 0.0 };
}


void
VirtualJoystickWidget::paintEvent (QPaintEvent*)
{
	auto const m = em_pixels (0.5);
	QRectF r = rect().marginsRemoved (QMargins (m, m, m, m));

	if (r.width() < r.height())
	{
		r.setHeight (r.width());
		r.moveTop (0.5 * (size().height() - r.height()));
	}
	else
	{
		r.setWidth (r.height());
		r.moveLeft (0.5 * (size().width() - r.width()));
	}

	QPainter painter (this);
	painter.setRenderHint (QPainter::Antialiasing);

	for (auto const outlining: { true, false })
	{
		auto const color = outlining ? Qt::black : Qt::white;
		auto const brush_color = outlining ? Qt::white : Qt::blue;
		auto const added_width = outlining ? em_pixels (0.2) : 0.0;
		auto const half_right = QPointF (0.5 * r.width(), 0.0);
		auto const half_down = QPointF (0.0, 0.5 * r.height());
		auto const center = r.topLeft() + half_right + half_down;

		// Box:
		painter.setBrush (Qt::NoBrush);
		painter.setPen (QPen (color, added_width + em_pixels (0.3), Qt::SolidLine, Qt::SquareCap));
		painter.drawRect (r);

		// Cross:
		painter.setPen (QPen (color, added_width + em_pixels (0.1), Qt::SolidLine, Qt::FlatCap));
		painter.drawLine (r.topLeft() + half_right, r.bottomLeft() + half_right);
		painter.drawLine (r.topLeft() + half_down, r.topRight() + half_down);

		// Current position:
		painter.setBrush (brush_color);

		auto finish_point = center;

		if (_control)
		{
			auto const pos = position();

			finish_point += QPointF (+pos.x() * 0.5 * r.width(),
									 -pos.y() * 0.5 * r.width());
		}

		auto const c = em_pixels (0.8);
		auto ellipse = QRectF (QPointF (0.0, 0.0), QSizeF (c, c));
		ellipse.moveCenter (finish_point);

		painter.setPen (QPen (brush_color, added_width + em_pixels (0.1), Qt::SolidLine, Qt::FlatCap));
		painter.drawLine (center, finish_point);
		painter.setPen (QPen (Qt::white, em_pixels (0.2)));
		painter.drawEllipse (ellipse);
	}
}


void
VirtualJoystickWidget::mousePressEvent (QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		event->accept();
		_control.emplace();
		_control->starting_point = event->pos();
		_control->current_point = event->pos();
		update();
	}
}


void
VirtualJoystickWidget::mouseReleaseEvent (QMouseEvent*)
{
	_control.reset();
	update();
}


void
VirtualJoystickWidget::mouseMoveEvent (QMouseEvent* event)
{
	if (_control)
		_control->current_point = event->pos();

	update();
}


VirtualThrottleWidget::VirtualThrottleWidget (QWidget* parent):
	Widget (parent)
{
	setStyleSheet (kTransparentStyleSheet);
	setFixedSize (em_pixels (5), em_pixels (20));
}


float
VirtualThrottleWidget::throttle() const noexcept
{
	constexpr xf::Range range { 0.0, 1.0 };

	if (_control)
		return xf::clamped (_throttle - _control->delta().y() / height(), range);
	else
		return xf::clamped (_throttle, range);
}


void
VirtualThrottleWidget::paintEvent (QPaintEvent*)
{
	auto const m = em_pixels (0.5);
	QRectF r = rect().marginsRemoved (QMargins (m, m, m, m));

	QPainter painter (this);
	painter.setRenderHint (QPainter::Antialiasing);

	for (auto const outlining: { true, false })
	{
		auto const color = outlining ? Qt::black : Qt::white;
		auto const added_width = outlining ? em_pixels (0.2) : 0.0;
		auto const y = throttle() * r.height();

		// Throttle:
		painter.setPen (QPen (color, added_width + em_pixels (0.1), Qt::SolidLine, Qt::FlatCap));

		if (outlining)
			painter.fillRect (QRectF (QPointF (r.left(), r.bottom() - y), r.bottomRight()), QColor (0, 255, 0, 200));

		painter.drawLine (QPointF (r.left(), r.bottom() - y), QPointF (r.right(), r.bottom() - y));

		// Box:
		painter.setBrush (Qt::NoBrush);
		painter.setPen (QPen (color, added_width + em_pixels (0.3), Qt::SolidLine, Qt::SquareCap));
		painter.drawRect (r);

	}
}


void
VirtualThrottleWidget::mousePressEvent (QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		event->accept();
		_control.emplace();
		_control->starting_point = event->pos();
		_control->current_point = event->pos();
		update();
	}
}


void
VirtualThrottleWidget::mouseReleaseEvent (QMouseEvent*)
{
	_throttle = throttle();
	_control.reset();
	update();
}


void
VirtualThrottleWidget::mouseMoveEvent (QMouseEvent* event)
{
	if (_control)
		_control->current_point = event->pos();

	update();
}


VirtualJoystick::VirtualJoystick (std::unique_ptr<VirtualJoystickIO> module_io, std::string_view const& instance):
	Module (std::move (module_io), instance)
{
	using namespace std::literals;

	_widget = new xf::Widget (nullptr);
	_widget->setWindowTitle (QString::fromStdString ("XEFIS virtual joystick" + (instance.empty() ? ""s : ": "s + instance)));
	xf::set_kde_blur_background (*_widget, true);

	_joystick_widget = new VirtualJoystickWidget (_widget);
	_throttle_widget = new VirtualThrottleWidget (_widget);

	auto* layout = new QHBoxLayout (_widget);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_throttle_widget);
	layout->addWidget (_joystick_widget);
	layout->setSizeConstraint (QLayout::SetFixedSize);

	_widget->show();
}


void
VirtualJoystick::process (xf::Cycle const&)
{
	auto const jpos = _joystick_widget->position();
	io.x_axis = jpos.x();
	io.y_axis = jpos.y();
	io.throttle = _throttle_widget->throttle();
}

