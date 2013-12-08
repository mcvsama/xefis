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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "panel_numeric_display.h"


namespace Xefis {

PanelNumericDisplay::PanelNumericDisplay (QWidget* parent, Panel* panel, unsigned int num_digits, bool pad_with_zeros, PropertyInteger value_property):
	PanelWidget (parent, panel),
	_num_digits (num_digits),
	_pad_with_zeros (pad_with_zeros),
	_value_property (value_property)
{
	_digits_to_display.resize (_num_digits);

	_digit_images[0] = Resources::Digits::digit_0();
	_digit_images[1] = Resources::Digits::digit_1();
	_digit_images[2] = Resources::Digits::digit_2();
	_digit_images[3] = Resources::Digits::digit_3();
	_digit_images[4] = Resources::Digits::digit_4();
	_digit_images[5] = Resources::Digits::digit_5();
	_digit_images[6] = Resources::Digits::digit_6();
	_digit_images[7] = Resources::Digits::digit_7();
	_digit_images[8] = Resources::Digits::digit_8();
	_digit_images[9] = Resources::Digits::digit_9();
	_digit_images[10] = Resources::Digits::digit_minus();
	_digit_images[11] = Resources::Digits::digit_empty();

	setMinimumSize (_digit_images[0].width() * _num_digits + 2 * (BorderWidth + Margin), _digit_images[0].height() + 2 * (BorderWidth + Margin));

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
	for (unsigned int d = 0; d < _digits_to_display.size(); ++d)
		painter.drawPixmap (BorderWidth + Margin + d * _digit_images[0].width() + correction.x() + re.left(),
							BorderWidth + Margin + correction.y() + re.top(),
							_digits_to_display[d]);
}


void
PanelNumericDisplay::data_updated()
{
	read();
}


void
PanelNumericDisplay::read()
{
	QString digits;
	if (_value_property.is_nil())
		digits = QString (_num_digits, ' ');
	else
		digits = convert_to_digits (*_value_property, _num_digits, _pad_with_zeros);

	for (unsigned int i = 0; i < _num_digits; ++i)
	{
		auto c = digits[i];

		if (c == '-')
			_digits_to_display[i] = _digit_images[MinusSymbolIndex];
		else if (c.isDigit())
			_digits_to_display[i] = _digit_images[c.digitValue()];
		else
			_digits_to_display[i] = _digit_images[EmptySymbolIndex];
	}

	update();
}


QString
PanelNumericDisplay::convert_to_digits (int64_t value, unsigned int num_digits, bool pad_with_zeros)
{
	if (num_digits == 0)
		return QString();

	if (value < 0)
		pad_with_zeros = false;

	QString result = QString ("%1").arg (value, num_digits, 10, pad_with_zeros ? QChar ('0') : QChar (' '));

	if (value >= 0)
	{
		if (result.size() > static_cast<int> (num_digits))
			result = QString (num_digits, QChar ('9'));
	}
	else
	{
		if (result.size() > static_cast<int> (num_digits))
			result = '-' + QString (num_digits - 1, QChar ('9'));
	}

	return result;
}

} // namespace Xefis

