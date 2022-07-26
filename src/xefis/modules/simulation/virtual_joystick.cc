/* vim:ts=4
 *
 * Copyleft 2022  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Local:
#include "virtual_joystick.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/paint_helper.h>
#include <xefis/support/ui/widget.h>
#include <xefis/utility/kde.h>

// Neutrino:
#include <neutrino/numeric.h>
#include <neutrino/range.h>

// Lib:
#include <QBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>

// Standard:
#include <cstddef>


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
	static constexpr float kMarginEm = 0.5;

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
	xf::PaintHelper				_paint_helper	{ *this, palette(), font() };
};


class VirtualLinearWidget: public xf::Widget
{
	static constexpr xf::Range	kInternalRange	{ 0.0f, 1.0f };
	static constexpr float		kMarginEm		{ 0.5 };

  public:
	enum Orientation
	{
		Horizontal,
		Vertical,
	};

	enum Style
	{
		Filled,
		BarOnly,
	};

  public:
	// Ctor
	explicit
	VirtualLinearWidget (xf::Range<float>, Orientation, Style, QWidget* parent);

	/**
	 * Set value.
	 */
	void
	set_value (float);

	/**
	 * Value is in range [0, 1].
	 */
	float
	value() const noexcept;

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
	xf::Range<float>			_range;
	Orientation					_orientation;
	Style						_style;
	float						_value			{ 0.0 };
	std::optional<MouseControl>	_control;
	xf::PaintHelper				_paint_helper	{ *this, palette(), font() };
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
	setFixedSize (_paint_helper.em_pixels (20), _paint_helper.em_pixels (20));
}


QPointF
VirtualJoystickWidget::position() const noexcept
{
	if (_control)
	{
		using std::abs;
		using xf::sgn;

		auto const m2 = 2 * _paint_helper.em_pixels (kMarginEm);
		auto const size = std::min (width() - m2, height() - m2);
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
	auto const m = _paint_helper.em_pixels (kMarginEm);
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
		auto const added_width = outlining ? _paint_helper.em_pixels (0.2) : 0.0;
		auto const half_right = QPointF (0.5 * r.width(), 0.0);
		auto const half_down = QPointF (0.0, 0.5 * r.height());
		auto const center = r.topLeft() + half_right + half_down;

		// Box:
		painter.setBrush (Qt::NoBrush);
		painter.setPen (QPen (color, added_width + _paint_helper.em_pixels (0.3), Qt::SolidLine, Qt::SquareCap));
		painter.drawRect (r);

		// Cross:
		painter.setPen (QPen (color, added_width + _paint_helper.em_pixels (0.1), Qt::SolidLine, Qt::FlatCap));
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

		auto const c = _paint_helper.em_pixels (0.8);
		auto ellipse = QRectF (QPointF (0.0, 0.0), QSizeF (c, c));
		ellipse.moveCenter (finish_point);

		painter.setPen (QPen (brush_color, added_width + _paint_helper.em_pixels (0.1), Qt::SolidLine, Qt::FlatCap));
		painter.drawLine (center, finish_point);
		painter.setPen (QPen (Qt::white, _paint_helper.em_pixels (0.2)));
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


VirtualLinearWidget::VirtualLinearWidget (xf::Range<float> range, Orientation orientation, Style style, QWidget* parent):
	Widget (parent),
	_range (range),
	_orientation (orientation),
	_style (style)
{
	setStyleSheet (kTransparentStyleSheet);

	switch (_orientation)
	{
		case Horizontal:
			setFixedSize (_paint_helper.em_pixels (20), _paint_helper.em_pixels (5));
			break;

		case Vertical:
			setFixedSize (_paint_helper.em_pixels (5), _paint_helper.em_pixels (20));
			break;
	}
}


void
VirtualLinearWidget::set_value (float v)
{
	_value = xf::renormalize (v, _range, kInternalRange);
	update();
}


float
VirtualLinearWidget::value() const noexcept
{
	float v = 0.0;

	if (_control)
	{
		switch (_orientation)
		{
			case Horizontal:
				v = xf::clamped<float> (_value + _control->delta().x() / width(), kInternalRange);
				break;

			case Vertical:
				v = xf::clamped<float> (_value - _control->delta().y() / height(), kInternalRange);
				break;
		}
	}
	else
		v = xf::clamped<float> (_value, kInternalRange);

	return xf::renormalize (v, kInternalRange, _range);
}


void
VirtualLinearWidget::paintEvent (QPaintEvent*)
{
	auto const m = _paint_helper.em_pixels (kMarginEm);
	QRectF r = rect();

	QPainter painter (this);
	painter.setRenderHint (QPainter::Antialiasing);

	if (_orientation == Horizontal)
	{
		painter.translate (+0.5 * QPointF (r.size().width(), r.size().height()));
		painter.rotate (90);
		painter.translate (-0.5 * QPointF (r.size().height(), r.size().width()));
		r = r.transposed();
	}

	r = r.marginsRemoved (QMargins (m, m, m, m));

	for (auto const outlining: { true, false })
	{
		auto const color = outlining ? Qt::black : Qt::white;
		auto const added_width = outlining ? _paint_helper.em_pixels (0.2) : 0.0;
		auto const y = xf::renormalize (value(), _range, kInternalRange) * r.height();

		// Throttle:
		painter.setPen (QPen (color, added_width + _paint_helper.em_pixels (0.1), Qt::SolidLine, Qt::FlatCap));

		if (outlining && _style == Filled)
			painter.fillRect (QRectF (QPointF (r.left(), r.bottom() - y), r.bottomRight()), QColor (0, 255, 0, 200));

		painter.drawLine (QPointF (r.left(), r.bottom() - y), QPointF (r.right(), r.bottom() - y));

		// Box:
		painter.setBrush (Qt::NoBrush);
		painter.setPen (QPen (color, added_width + _paint_helper.em_pixels (0.3), Qt::SolidLine, Qt::SquareCap));
		painter.drawRect (r);
	}
}


void
VirtualLinearWidget::mousePressEvent (QMouseEvent* event)
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
VirtualLinearWidget::mouseReleaseEvent (QMouseEvent*)
{
	set_value (value());
	_control.reset();
	update();
}


void
VirtualLinearWidget::mouseMoveEvent (QMouseEvent* event)
{
	if (_control)
		_control->current_point = event->pos();

	update();
}


VirtualJoystick::VirtualJoystick (std::string_view const& instance):
	VirtualJoystickIO (instance)
{
	using namespace std::literals;

	_widget = new xf::Widget (nullptr);
	_widget->setWindowTitle (QString::fromStdString ("XEFIS virtual joystick" + (instance.empty() ? ""s : ": "s + instance)));
	xf::set_kde_blur_background (*_widget, true);

	_joystick_widget = new VirtualJoystickWidget (_widget);

	_throttle_widget = new VirtualLinearWidget ({ 0.0f, 1.0f }, VirtualLinearWidget::Vertical, VirtualLinearWidget::Filled, _widget);
	_throttle_widget->set_value (0.0f);

	_rudder_widget = new VirtualLinearWidget ({ -1.0f, 1.0f }, VirtualLinearWidget::Horizontal, VirtualLinearWidget::BarOnly, _widget);
	_rudder_widget->set_value (0.0f);

	auto* layout = new QGridLayout (_widget);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_throttle_widget, 0, 0);
	layout->addWidget (_joystick_widget, 0, 1);
	layout->addWidget (_rudder_widget, 1, 1);
	layout->setSizeConstraint (QLayout::SetFixedSize);

	_widget->show();
}


void
VirtualJoystick::process (xf::Cycle const&)
{
	auto const jpos = _joystick_widget->position();
	_io.x_axis = jpos.x();
	_io.y_axis = jpos.y();
	_io.throttle = _throttle_widget->value();
	_io.rudder = _rudder_widget->value();
}

