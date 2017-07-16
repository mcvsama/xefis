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

// Qt:
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "panel_numeric_display.h"


namespace xf {

PanelNumericDisplay::PanelNumericDisplay (QWidget* parent, Panel* panel, unsigned int num_digits, std::string unit, PropertyPath const& value_property_path):
	PanelWidget (parent, panel),
	_num_digits (num_digits),
	_unit (unit)
{
	_digit_images[0] = resources::digits::digit_0();
	_digit_images[1] = resources::digits::digit_1();
	_digit_images[2] = resources::digits::digit_2();
	_digit_images[3] = resources::digits::digit_3();
	_digit_images[4] = resources::digits::digit_4();
	_digit_images[5] = resources::digits::digit_5();
	_digit_images[6] = resources::digits::digit_6();
	_digit_images[7] = resources::digits::digit_7();
	_digit_images[8] = resources::digits::digit_8();
	_digit_images[9] = resources::digits::digit_9();
	_digit_images[MinusSymbolIndex] = resources::digits::digit_minus();
	_digit_images[EmptySymbolIndex] = resources::digits::digit_empty();
	_digit_images[DotSymbolIndex] = resources::digits::digit_dot();

	_digits_to_display.resize (_num_digits + 1, nullptr); // Add margin (+1) for dot image.
	setMinimumSize (_digit_images[0].width() * _num_digits + 2 * (BorderWidth + Margin),
					_digit_images[0].height() + 2 * (BorderWidth + Margin));
	_value_property.set_path (value_property_path);
}


PanelNumericDisplay::PanelNumericDisplay (QWidget* parent, Panel* panel, unsigned int num_digits, std::string unit, PropertyPath const& value_property_path, std::string const& format):
	PanelNumericDisplay (parent, panel, num_digits, unit, value_property_path)
{
	_static_format = boost::format (format);
	read();
}


PanelNumericDisplay::PanelNumericDisplay (QWidget* parent, Panel* panel, unsigned int num_digits, std::string unit, PropertyPath const& value_property_path, v1::PropertyString const& format_property):
	PanelNumericDisplay (parent, panel, num_digits, unit, value_property_path)
{
	_dynamic_format = format_property;
	read();
}


void
PanelNumericDisplay::paintEvent (QPaintEvent*)
{
	QPainter painter (this);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);

	QSize ms = minimumSize();
	QRect re (0.5f * (width() - ms.width()), 0.5f * (height() - ms.height()), ms.width() + 1, ms.height() + 1);

	QPalette pal = palette();

	QPointF pt1 (re.center().y() - re.top(), re.center().y() - re.top());
	QPointF pt2 (re.bottomRight() - pt1);
	pt1 += re.topLeft();

	QPolygonF poly1;
	poly1 << re.bottomLeft() << re.topLeft() << re.topRight() << pt2 << pt1;
	QPolygonF poly2;
	poly2 << re.bottomLeft() << re.bottomRight() << re.topRight() << pt2 << pt1;

	painter.setPen (Qt::NoPen);
	painter.setBrush (pal.color (QPalette::Window).darker (150));
	painter.drawPolygon (poly1);
	painter.setBrush (pal.color (QPalette::Window).lighter (200));
	painter.drawPolygon (poly2);

	painter.fillRect (re.adjusted (BorderWidth, BorderWidth, -1 - BorderWidth, -1 - BorderWidth), Qt::black);

	QPointF correction (1.f, 0.f);
	int digit_pos = 0;

	for (unsigned int d = 0; d < _digits_to_display.size(); ++d)
	{
		if (!_digits_to_display[d])
			continue;

		if (_digits_to_display[d] == &_digit_images[DotSymbolIndex])
			digit_pos -= 1;

		if (digit_pos < 0)
			digit_pos = 0;

		painter.drawPixmap (BorderWidth + Margin + digit_pos * _digit_images[0].width() + correction.x() + re.left(),
							BorderWidth + Margin + correction.y() + re.top(),
							*_digits_to_display[d]);

		digit_pos += 1;
	}
}


void
PanelNumericDisplay::data_updated()
{
	read();
}


void
PanelNumericDisplay::read()
{
	if (!_value_property.fresh())
		return;

	std::string digits;

	if (_value_property.is_nil())
		digits = std::string (_num_digits, ' ');
	else
		digits = convert_to_digits (_value_property.to_float (_unit));

	std::fill (_digits_to_display.begin(), _digits_to_display.end(), nullptr);

	for (unsigned int i = 0; i < digits.size(); ++i)
	{
		auto c = i < digits.size()
			? digits[i]
			: ' ';

		if (c == '-')
			_digits_to_display[i] = &_digit_images[MinusSymbolIndex];
		else if (c == '.')
			_digits_to_display[i] = &_digit_images[DotSymbolIndex];
		else if (std::isdigit (c))
			_digits_to_display[i] = &_digit_images[c - '0'];
		else
			_digits_to_display[i] = &_digit_images[EmptySymbolIndex];
	}

	update();
}


std::string
PanelNumericDisplay::convert_to_digits (double value)
{
	std::string result;

	try {
		if (_dynamic_format.configured())
		{
			if (_dynamic_format.valid())
				result = (boost::format (*_dynamic_format) % value).str();
		}
		else
			result = (_static_format % value).str();

		std::size_t allowed_size = _num_digits;
		if (result.find ('.') != std::string::npos)
			allowed_size += 1;

		if (value >= 0)
		{
			if (result.size() > allowed_size)
				result = std::string (_num_digits, '9');
		}
		else
		{
			if (result.size() > allowed_size)
				result = '-' + std::string (_num_digits - 1, '9');
		}
	}
	catch (boost::io::format_error&)
	{
		result = "-.";
	}

	return result;
}

} // namespace xf

