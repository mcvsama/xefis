/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <utility>
#include <cmath>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/text_painter.h>

// Local:
#include "efis_widget.h"


const char*	EFISWidget::AP = "A/P";
const char*	EFISWidget::AT = "A/T";
const char*	EFISWidget::LDGALT = "LDG";
const char	EFISWidget::DIGITS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
const char*	EFISWidget::MINUS_SIGN = "−";


EFISWidget::AltitudeLadder::AltitudeLadder (EFISWidget& efis, QPainter& painter):
	_efis (efis),
	_painter (painter),
	_text_painter (_painter, &_efis._text_painter_cache),
	_altitude (bound (_efis._altitude, -9999.f, +99999.f)),
	_climb_rate (bound (_efis._climb_rate, -9999.f, +9999.f)),
	_pressure (bound (_efis._pressure, 0.f, 99.99f)),
	_extent (825.f),
	_sgn (_altitude < 0.f ? -1.f : 1.f),
	_min_shown (_altitude - _extent / 2.f),
	_max_shown (_altitude + _extent / 2.f),
	_rounded_altitude (static_cast<int> (_altitude + _sgn * 10.f) / 20 * 20),
	_ladder_rect (-0.0675f * _efis.wh(), -0.375 * _efis.wh(), 0.135 * _efis.wh(), 0.75f * _efis.wh()),
	_ladder_pen (_efis.get_pen (_efis._ladder_color, 0.5f)),
	_black_box_pen (_efis.get_pen (QColor (255, 255, 255), 1.f)),
	_scale_pen_1 (_efis.get_pen (QColor (255, 255, 255), 1.f)),
	_scale_pen_2 (_efis.get_pen (QColor (255, 255, 255), 3.f)),
	_altitude_bug_pen (_efis.get_pen (QColor (0, 255, 0), 1.5f)),
	_ldg_alt_pen (_efis.get_pen (QColor (255, 220, 0), 1.5f))
{ }


void
EFISWidget::AltitudeLadder::paint()
{
	float const x = _ladder_rect.width() / 4.0f;

	_painter.save();

	_painter.setPen (_ladder_pen);
	_painter.setBrush (_efis._ladder_color);
	_painter.drawRect (_ladder_rect);

	paint_black_box (x, true);
	paint_ladder_scale (x);
	paint_bugs (x);
	paint_climb_rate (x);
	paint_black_box (x);
	paint_pressure (x);
	paint_ap_setting (x);

	_painter.restore();
}


void
EFISWidget::AltitudeLadder::paint_black_box (float x, bool only_compute_black_box_rect)
{
	QFont b_font = _efis._font_20_bold;
	float const b_digit_width = _efis._font_20_digit_width;
	float const b_digit_height = _efis._font_20_digit_height;

	QFont s_font = _efis._font_16_bold;
	float const s_digit_width = _efis._font_16_digit_width;
	float const s_digit_height = _efis._font_16_digit_height;

	int const b_digits = 2;
	int const s_digits = 3;
	float const margin = 0.2f * b_digit_width;

	QRectF b_digits_box (0.f, 0.f, b_digits * b_digit_width + margin, 2.f * b_digit_height);
	QRectF s_digits_box (0.f, 0.f, s_digits * s_digit_width + margin, 2.f * b_digit_height);
	_black_box_rect = QRectF (0.f, -0.5f * b_digits_box.height(),
							  b_digits_box.width() + s_digits_box.width(), b_digits_box.height());

	if (only_compute_black_box_rect)
		return;
	if (!_efis._altitude_visible)
		return;

	b_digits_box.translate (0.f, -0.5f * b_digits_box.height());
	s_digits_box.translate (b_digits_box.width(), -0.5f * s_digits_box.height());

	_painter.save();
	_painter.translate (-0.75f * x, 0.f);

	_painter.setPen (_black_box_pen);
	_painter.setBrush (QBrush (QColor (0, 0, 0)));
	_painter.drawPolygon (QPolygonF()
		<< QPointF (-0.5f * x, 0.f)
		<< QPointF (0.f, -0.5f * x)
		<< _black_box_rect.topLeft()
		<< _black_box_rect.topRight()
		<< _black_box_rect.bottomRight()
		<< _black_box_rect.bottomLeft()
		<< QPointF (0.f, +0.5f * x));

	_painter.setFont (b_font);

	// 11000 part of the altitude:
	QRectF box_11000 = b_digits_box.adjusted (margin, margin, 0.f, -margin);
	_text_painter.drawText (box_11000, Qt::AlignVCenter | Qt::AlignRight,
							QString::number (std::abs (_rounded_altitude / 1000)));
	if (-10000.f < _rounded_altitude && _rounded_altitude < 10000.f)
	{
		QColor color = _sgn >= 0.f ? QColor (0, 255, 0) : QColor (255, 0, 0);
		QRectF green_square_box (-0.3f * b_digit_width, -0.4f * b_digit_height, 0.6f * b_digit_width, 0.78f * b_digit_height);
		green_square_box.translate (0.5f * x + 0.75f * margin, 0.f);
		_painter.save();
		_painter.setPen (Qt::NoPen);
		_painter.setBrush (QBrush (color, Qt::Dense4Pattern));
		_painter.drawRect (green_square_box);
		_painter.restore();
	}

	_painter.setFont (s_font);

	// 00100 part of the altitude:
	QRectF box_00100 = s_digits_box.adjusted (0.f, margin, -margin, -margin);
	_text_painter.drawText (box_00100, Qt::AlignVCenter | Qt::AlignLeft, QString::number (std::abs ((_rounded_altitude / 100) % 10)));

	// 00011 part of the altitude:
	QRectF box_00011 = box_00100.adjusted (s_digit_width, 0.f, 0.f, 0.f);
	QRectF box_00011_p10 = box_00011.translated (0.f, -s_digit_height);
	QRectF box_00011_m10 = box_00011.translated (0.f, +s_digit_height);
	_painter.setClipRect (box_00011);
	_painter.translate (0.f, -s_digit_height * (_rounded_altitude - _altitude) / 20.f);
	_text_painter.drawText (box_00011_p10, Qt::AlignVCenter | Qt::AlignLeft,
							QString::number (std::abs (std::fmod (_rounded_altitude / 10.f + 2.f, 10.f))) + "0");
	_text_painter.drawText (box_00011, Qt::AlignVCenter | Qt::AlignLeft,
							QString::number (std::abs (std::fmod (_rounded_altitude / 10.f, 10.f))) + "0");
	_text_painter.drawText (box_00011_m10, Qt::AlignVCenter | Qt::AlignLeft,
							QString::number (std::abs (std::fmod (_rounded_altitude / 10.f - 2.f, 10.f))) + "0");

	_painter.restore();
}


void
EFISWidget::AltitudeLadder::paint_ladder_scale (float x)
{
	if (!_efis._altitude_visible)
		return;

	int const line_every = 100;
	int const num_every = 200;
	int const bold_every = 500;

	QFont b_ladder_font = _efis._font_13_bold;
	float const b_ladder_digit_width = _efis._font_13_digit_width;
	float const b_ladder_digit_height = _efis._font_13_digit_height;

	QFont s_ladder_font = _efis._font_10_bold;
	float const s_ladder_digit_width = _efis._font_10_digit_width;
	float const s_ladder_digit_height = _efis._font_10_digit_height;

	// Special clipping that leaves some margin around black indicator:
	QPainterPath clip_path_m;
	clip_path_m.addRect (_black_box_rect.translated (-x, 0.f).adjusted (0.f, -0.2f * x, 0.f, +0.2f * x));
	QPainterPath clip_path;
	clip_path.addRect (_ladder_rect);
	clip_path -= clip_path_m;

	_painter.save();
	_painter.setClipPath (clip_path);
	_painter.translate (-2.f * x, 0.f);

	// -+line_every is to have drawn also numbers that barely fit the scale.
	for (int ft = (static_cast<int> (_min_shown) / line_every) * line_every - line_every;
		 ft <= _max_shown + line_every;
		 ft += line_every)
	{
		float posy = ft_to_px (ft);

		_painter.setPen (ft % bold_every == 0 ? _scale_pen_2 : _scale_pen_1);
		_painter.drawLine (QPointF (0.f, posy), QPointF (0.8f * x, posy));

		if (ft % num_every == 0)
		{
			QRectF big_text_box (1.1f * x, -0.5f * b_ladder_digit_height + posy,
								 2.f * b_ladder_digit_width, b_ladder_digit_height);
			if (std::abs (ft) / 1000 > 0)
			{
				QString big_text = QString::number (ft / 1000);
				_painter.setFont (b_ladder_font);
				_text_painter.drawText (big_text_box, Qt::AlignVCenter | Qt::AlignRight, big_text);
			}

			QString small_text = QString ("%1").arg (QString::number (ft % 1000), 3, '0');
			if (ft == 0)
				small_text = "0";
			_painter.setFont (s_ladder_font);
			QRectF small_text_box (1.1f * x + 2.1f * b_ladder_digit_width, -0.5f * s_ladder_digit_height + posy,
								   3.f * s_ladder_digit_width, s_ladder_digit_height);
			_text_painter.drawText (small_text_box, Qt::AlignVCenter | Qt::AlignRight, small_text);
			// Minus sign?
			if (ft < 0)
			{
				if (ft > -1000)
					_text_painter.drawText (small_text_box.adjusted (-s_ladder_digit_width, 0.f, 0.f, 0.f),
											Qt::AlignVCenter | Qt::AlignLeft, MINUS_SIGN);
			}
		}
	}

	_painter.restore();
}


void
EFISWidget::AltitudeLadder::paint_bugs (float x)
{
	if (!_efis._altitude_visible)
		return;

	QFont altitude_bug_font = _efis._font_10_bold;
	float const altitude_bug_digit_height = _efis._font_10_digit_height;

	_painter.save();
	_painter.setFont (altitude_bug_font);

	for (auto& bug: _efis._altitude_bugs)
	{
		// AP bug should be drawn last, to be on top:
		if (bug.first == AP)
			continue;

		if (bug.second > _min_shown && bug.second < _max_shown)
		{
			float posy = ft_to_px (bug.second);
			QRectF text_rect (-4.5f * x, posy - 0.5f * altitude_bug_digit_height,
							  +2.f * x, altitude_bug_digit_height);
			_painter.setClipRect (_ladder_rect.adjusted (-x, 0.f, 0.f, 0.f));

			if (bug.first == LDGALT)
			{
				_painter.setPen (_ldg_alt_pen);
				_painter.drawLine (QPointF (-0.5f * x, posy), QPointF (-2.25f * x, posy));
			}
			else
			{
				_painter.setPen (_altitude_bug_pen);
				_painter.drawLine (QPointF (-1.5f * x, posy), QPointF (-2.25f * x, posy));
			}

			_painter.setClipping (false);
			_text_painter.drawText (text_rect, Qt::AlignVCenter | Qt::AlignRight, bug.first);
		}
	}

	// AP bug:
	auto ap_bug = _efis._altitude_bugs.find (AP);
	if (ap_bug != _efis._altitude_bugs.end())
	{
		float posy = bound (ft_to_px (ap_bug->second),
							static_cast<float> (-_ladder_rect.height() / 2), static_cast<float> (_ladder_rect.height() / 2));
		QPolygonF bug_shape = QPolygonF()
			<< QPointF (0.f, 0.f)
			<< QPointF (-0.5f * x, -0.5f * x)
			<< QPointF (-0.5f * x, _black_box_rect.top())
			<< QPointF (+1.3f * x, _black_box_rect.top())
			<< QPointF (+1.3f * x, _black_box_rect.bottom())
			<< QPointF (-0.5f * x, _black_box_rect.bottom())
			<< QPointF (-0.5f * x, +0.5f * x);
		_painter.setClipRect (_ladder_rect.translated (-x, 0.f));
		_painter.translate (-2.f * x, posy);
		_painter.setBrush (Qt::NoBrush);
		_painter.setPen (_efis.get_pen (_efis._autopilot_color.darker (400), 2.f));
		_painter.drawPolygon (bug_shape);
		_painter.setPen (_efis.get_pen (_efis._autopilot_color, 1.2f));
		_painter.drawPolygon (bug_shape);
	}

	_painter.restore();
}


void
EFISWidget::AltitudeLadder::paint_climb_rate (float x)
{
	if (!_efis._climb_rate_visible)
		return;

	QPen bold_white_pen = _efis.get_pen (QColor (255, 255, 255), 1.25f);
	QPen thin_white_pen = _efis.get_pen (QColor (255, 255, 255), 0.50f);
	QBrush ladder_brush (_efis._ladder_color);

	_painter.save();

	float const y = x * 4.f;

	_painter.translate (3.75f * x, 0.f);

	_painter.setPen (_ladder_pen);
	_painter.setBrush (ladder_brush);
	_painter.drawPolygon (QPolygonF()
		<< QPointF (0.0f, -0.6 * y)
		<< QPointF (-x, -0.6 * y - x)
		<< QPointF (-x, -1.9f * y - x)
		<< QPointF (+0.3f * x, -1.9f * y - x)
		<< QPointF (1.66f * x, -y - x)
		<< QPointF (1.66f * x, +y + x)
		<< QPointF (+0.3f * x, +1.9f * y + x)
		<< QPointF (-x, +1.9f * y + x)
		<< QPointF (-x, +0.6 * y + x)
		<< QPointF (0.0f, +0.6 * y));

	float const line_w = 0.2 * x;

	_painter.setFont (_efis._font_10_bold);
	_painter.setPen (bold_white_pen);
	_painter.drawLine (QPointF (0.f, 0.f), QPointF (0.5f * x, 0.f));
	for (float kfpm: { -6.f, -2.f, -1.f, +1.f, +2.f, +6.f })
	{
		float posy = -2.f * y * scale_cbr (kfpm * 1000.f);
		QRectF num_rect (-1.55f * x, posy - x, 1.3f * x, 2.f * x);
		_painter.drawLine (QPointF (0.f, posy), QPointF (line_w, posy));
		_text_painter.drawText (num_rect, Qt::AlignVCenter | Qt::AlignRight, QString::number (std::abs (static_cast<int> (kfpm))));
	}
	_painter.setPen (thin_white_pen);
	for (float kfpm: { -4.f, -1.5f, -0.5f, +0.5f, +1.5f, +4.f })
	{
		float posy = -2.f * y * scale_cbr (kfpm * 1000.f);
		_painter.drawLine (QPointF (0.f, posy), QPointF (line_w, posy));
	}
	_painter.setClipRect (QRectF (0.15f * x, -2.75f * y - x, (1.66f - 0.15f) * x, 5.5f * y + 2.f * x));
	QPen indicator_pen = bold_white_pen;
	indicator_pen.setCapStyle (Qt::FlatCap);
	_painter.setPen (indicator_pen);
	_painter.drawLine (QPointF (3.f * x, 0.f), QPointF (line_w, -2.f * y * scale_cbr (_climb_rate)));

	// Numeric indicators:

	int abs_climb_rate = static_cast<int> (std::abs (_climb_rate)) / 10 * 10;
	if (abs_climb_rate >= 100)
	{
		float const fh = _efis._font_13_digit_height;
		float const sgn = _climb_rate > 0.f ? 1.f : -1.f;
		_painter.setClipping (false);
		_painter.setFont (_efis._font_13_bold);
		_painter.translate (-1.05f * x, sgn * -2.35f * y);
		_text_painter.drawText (QRectF (0.f, -0.5f * fh, 4.f * fh, fh),
								Qt::AlignVCenter | Qt::AlignLeft, QString::number (abs_climb_rate));
	}

	_painter.restore();
}


void
EFISWidget::AltitudeLadder::paint_pressure (float x)
{
	if (!_efis._pressure_visible)
		return;

	_painter.save();
	_painter.translate (0.f, 0.75f * x);

	QFont font_a = _efis._font_16_bold;
	QFont font_b = _efis._font_10_bold;

	QString in_str = "IN";
	QString pressure_str = QString ("%1").arg (_pressure, 0, 'f', 2) + " ";

	QRectF nn_rect (0.f, _ladder_rect.bottom(), QFontMetrics (font_a).width (pressure_str), 1.2f * _efis._font_16_digit_height);
	QRectF zz_rect (0.f, nn_rect.top(), QFontMetrics (font_b).width (in_str), nn_rect.height());
	nn_rect.moveLeft (-0.5f * (zz_rect.width() + nn_rect.width()));
	// Correct position of zz_rect to get correct baseline position:
	zz_rect.translate (0.f, QFontMetrics (font_b).descent() - QFontMetrics (font_a).descent());
	zz_rect.moveLeft (nn_rect.right());

	_painter.setPen (QPen (_efis._navigation_color, _efis.pen_width()));
	_painter.setFont (font_a);
	_text_painter.drawText (nn_rect, Qt::AlignBottom | Qt::AlignRight, pressure_str, true);
	_painter.setFont (font_b);
	_text_painter.drawText (zz_rect, Qt::AlignBottom | Qt::AlignLeft, in_str);

	_painter.restore();
}


void
EFISWidget::AltitudeLadder::paint_ap_setting (float)
{
	auto ap_bug = _efis._altitude_bugs.find (AP);
	if (ap_bug == _efis._altitude_bugs.end())
		return;

	QFont b_font = _efis._font_20_bold;
	float const b_digit_width = _efis._font_20_digit_width;
	float const b_digit_height = _efis._font_20_digit_height;

	QFont s_font = _efis._font_16_bold;
	float const s_digit_width = _efis._font_16_digit_width;

	int const b_digits = 2;
	int const s_digits = 3;
	float const margin = 0.2f * b_digit_width;

	QRectF b_digits_box (0.f, 0.f, b_digits * b_digit_width + margin, 1.3f * b_digit_height);
	QRectF s_digits_box (0.f, 0.f, s_digits * s_digit_width + margin, 1.3f * b_digit_height);
	QRectF box_rect (_ladder_rect.left(), _ladder_rect.top() - 1.4f * b_digits_box.height(),
					 b_digits_box.width() + s_digits_box.width(), b_digits_box.height());
	b_digits_box.translate (box_rect.left(), box_rect.top());
	s_digits_box.translate (b_digits_box.right(), b_digits_box.top() + 0.f * s_digits_box.height());

	_painter.save();

	_painter.setPen (_efis.get_pen (QColor (0, 0, 0), 0.5f));
	_painter.setBrush (QBrush (QColor (0, 0, 0)));
	_painter.drawRect (box_rect);

	_painter.setPen (_efis.get_pen (_efis._autopilot_color, 1.f));
	_painter.setFont (b_font);

	// 11000 part of the altitude setting:
	QRectF box_11000 = b_digits_box.adjusted (margin, margin, 0.f, -margin);
	QString minus_sign_s = ap_bug->second < 0.f ? MINUS_SIGN : "";
	_painter.drawText (box_11000, Qt::AlignVCenter | Qt::AlignRight,
					   minus_sign_s + QString::number (std::abs (static_cast<int> (ap_bug->second / 1000))));

	_painter.setFont (s_font);

	// 00111 part of the altitude setting:
	QRectF box_00111 = s_digits_box.adjusted (0.f, margin, -margin, -margin);
	_painter.drawText (box_00111, Qt::AlignVCenter | Qt::AlignLeft,
					   QString ("%1").arg (static_cast<int> (std::abs (ap_bug->second)) % 1000, 3, 'f', 0, '0'));

	_painter.restore();
}


float
EFISWidget::AltitudeLadder::scale_cbr (FeetPerMinute climb_rate) const
{
	FeetPerMinute cbr = std::abs (climb_rate);

	if (cbr < 1000.f)
		cbr = cbr / 1000.f * 0.46f;
	else if (cbr < 2000)
		cbr = 0.46f + 0.32f * (cbr - 1000.f) / 1000.f;
	else if (cbr < 6000)
		cbr = 0.78f + 0.22f * (cbr - 2000.f) / 4000.f;
	else
		cbr = 1.f;

	if (climb_rate < 0.f)
		cbr *= -1.f;

	return cbr;
}


EFISWidget::SpeedLadder::SpeedLadder (EFISWidget& efis, QPainter& painter):
	_efis (efis),
	_painter (painter),
	_text_painter (_painter, &_efis._text_painter_cache),
	_speed (bound (_efis._speed, 0.f, 9999.9f)),
	_mach (bound (_efis._mach, 0.f, 9.99f)),
	_minimum_speed (bound (_efis._minimum_speed, 0.f, 9999.9f)),
	_warning_speed (bound (_efis._warning_speed, 0.f, 9999.9f)),
	_maximum_speed (bound (_efis._maximum_speed, 0.f, 9999.9f)),
	_extent (124.f),
	_min_shown (_speed - _extent / 2.f),
	_max_shown (_speed + _extent / 2.f),
	_rounded_speed (static_cast<int> (_speed + 0.5f)),
	_ladder_rect (-0.0675f * _efis.wh(), -0.375 * _efis.wh(), 0.135 * _efis.wh(), 0.75f * _efis.wh()),
	_ladder_pen (_efis._ladder_color, _efis.pen_width (0.5f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin),
	_black_box_pen (QColor (255, 255, 255), _efis.pen_width(), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin),
	_scale_pen (QColor (255, 255, 255), _efis.pen_width (1.f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin),
	_speed_bug_pen (QColor (0, 255, 0), _efis.pen_width (1.5f), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin)
{ }


void
EFISWidget::SpeedLadder::paint()
{
	float const x = _ladder_rect.width() / 4.0f;

	_painter.save();

	_painter.setPen (_ladder_pen);
	_painter.setBrush (_efis._ladder_color);
	_painter.drawRect (_ladder_rect);

	paint_black_box (x, true);
	paint_ladder_scale (x);
	paint_speed_limits (x);
	paint_speed_tendency (x);
	paint_bugs (x);
	paint_black_box (x);
	paint_mach_number (x);
	paint_ap_setting (x);

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_black_box (float x, bool only_compute_black_box_rect)
{
	QFont actual_speed_font = _efis._font_20_bold;
	float const digit_width = _efis._font_20_digit_width;
	float const digit_height = _efis._font_20_digit_height;

	int digits = 3;
	if (_speed >= 1000.0f - 0.5f)
		digits = 4;
	float const margin = 0.2f * digit_width;

	_black_box_rect = QRectF (-digits * digit_width - 2.f * margin, -digit_height,
							  +digits * digit_width + 2.f * margin, 2.f * digit_height);

	if (only_compute_black_box_rect)
		return;
	if (!_efis._speed_visible)
		return;

	_painter.save();
	_painter.translate (+0.75f * x, 0.f);

	_painter.setPen (_black_box_pen);
	_painter.setBrush (QBrush (QColor (0, 0, 0)));
	_painter.drawPolygon (QPolygonF()
		<< QPointF (+0.5f * x, 0.f)
		<< QPointF (0.f, -0.5f * x)
		<< _black_box_rect.topRight()
		<< _black_box_rect.topLeft()
		<< _black_box_rect.bottomLeft()
		<< _black_box_rect.bottomRight()
		<< QPointF (0.f, +0.5f * x));

	// 110 part of the speed:
	_painter.setFont (actual_speed_font);
	QRectF box_10 = _black_box_rect.adjusted (margin, margin, -margin - digit_width, -margin);
	_text_painter.drawText (box_10, Qt::AlignVCenter | Qt::AlignRight, QString::number (static_cast<int> (_speed + 0.5f) / 10));

	// 001 part of the speed:
	QRectF box_01 (box_10.right(), box_10.top(), digit_width, box_10.height());
	QRectF box_01_p1 = box_01.translated (0.f, -digit_height);
	QRectF box_01_m1 = box_01.translated (0.f, +digit_height);
	_painter.setClipRect (box_01);
	_painter.translate (0.f, -digit_height * (_rounded_speed - _speed));
	_text_painter.drawText (box_01_p1, Qt::AlignVCenter | Qt::AlignLeft,
							QString::number (static_cast<int> (floored_mod (1.f * _rounded_speed + 1.f, 10.f))));
	_text_painter.drawText (box_01, Qt::AlignVCenter | Qt::AlignLeft,
							QString::number (static_cast<int> (floored_mod (1.f * _rounded_speed, 10.f))));
	// Don't draw negative values:
	if (_speed > 0.5f)
		_text_painter.drawText (box_01_m1, Qt::AlignVCenter | Qt::AlignLeft,
								QString::number (static_cast<int> (floored_mod (1.f * _rounded_speed - 1.f, 10.f))));

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_ladder_scale (float x)
{
	if (!_efis._speed_visible)
		return;

	QFont ladder_font = _efis._font_13_bold;
	float const ladder_digit_width = _efis._font_13_digit_width;
	float const ladder_digit_height = _efis._font_13_digit_height;

	_painter.setFont (ladder_font);

	int const line_every = 10;
	int const num_every = 20;

	if (_min_shown < 0.f)
		_min_shown = 0.f;

	// Special clipping that leaves some margin around black indicator:
	QPainterPath clip_path_m;
	clip_path_m.addRect (_black_box_rect.translated (x, 0.f).adjusted (0.f, -0.2f * x, 0.f, +0.2f * x));
	QPainterPath clip_path;
	clip_path.addRect (_ladder_rect);
	clip_path -= clip_path_m;

	_painter.save();
	_painter.setClipPath (clip_path);
	_painter.translate (2.f * x, 0.f);

	_painter.setPen (_scale_pen);
	// -+line_every is to have drawn also numbers that barely fit the scale.
	for (int kt = (static_cast<int> (_min_shown) / line_every) * line_every - line_every;
		 kt <= _max_shown + line_every;
		 kt += line_every)
	{
		if (kt < 0)
			continue;
		float posy = kt_to_px (kt);
		_painter.drawLine (QPointF (-0.8f * x, posy), QPointF (0.f, posy));

		if (kt % num_every == 0)
			_text_painter.drawText (QRectF (-4.f * ladder_digit_width - 1.25f * x, -0.5f * ladder_digit_height + posy,
											+4.f * ladder_digit_width, ladder_digit_height),
									Qt::AlignVCenter | Qt::AlignRight, QString::number (kt));
	}

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_speed_limits (float x)
{
	if (!_efis._speed_visible)
		return;

	QPointF ydif (0.f, _efis.pen_width (0.25f));
	QPen pen_b (QColor (0, 0, 0), _efis.pen_width (10.f), Qt::SolidLine, Qt::FlatCap);
	QPen pen_r (QColor (255, 0, 0), _efis.pen_width (10.f), Qt::DotLine, Qt::FlatCap);
	QPen pen_y (QColor (255, 140, 0), _efis.pen_width (10.f), Qt::SolidLine, Qt::FlatCap);
	pen_r.setDashPattern (QVector<qreal> (2, 0.5f));

	_painter.save();
	_painter.translate (0.45f * x, 0.f);
	_painter.setClipRect (_ladder_rect.adjusted (0.f, -ydif.y(), 0.f, ydif.y()));

	float max_posy = kt_to_px (_maximum_speed);
	float wrn_posy = kt_to_px (_warning_speed);
	float min_posy = kt_to_px (_minimum_speed);
	QPointF zero_point (_ladder_rect.right(), std::min (_ladder_rect.bottom() + ydif.y(), 1.0 * kt_to_px (0.f)));

	if (_efis._maximum_speed_visible && _maximum_speed < _max_shown)
	{
		_painter.setPen (pen_b);
		_painter.drawLine (QPointF (_ladder_rect.right(), max_posy), _ladder_rect.topRight() - ydif);
		_painter.setPen (pen_r);
		_painter.drawLine (QPointF (_ladder_rect.right(), max_posy), _ladder_rect.topRight() - ydif);
	}

	if (_efis._warning_speed_visible && _warning_speed > _min_shown)
	{
		_painter.setPen (pen_y);
		_painter.drawLine (QPointF (_ladder_rect.right(), wrn_posy), zero_point);
	}

	if (_efis._minimum_speed_visible && _minimum_speed > _min_shown)
	{
		_painter.setPen (pen_b);
		_painter.drawLine (QPointF (_ladder_rect.right(), min_posy), zero_point);
		_painter.setPen (pen_r);
		_painter.drawLine (QPointF (_ladder_rect.right(), min_posy), zero_point);
	}

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_speed_tendency (float x)
{
	if (!_efis._speed_tendency_visible || !_efis._speed_visible)
		return;

	QPen pen (_efis.get_pen (_efis._navigation_color, 1.25f));
	pen.setCapStyle (Qt::RoundCap);
	pen.setJoinStyle (Qt::RoundJoin);

	_painter.save();
	_painter.setPen (pen);
	_painter.translate (1.2f * x, 0.f);
	if (_efis._speed_tendency < _efis._speed)
		_painter.scale (1.f, -1.f);
	float length = std::min (_ladder_rect.height() / 2.f, 1.0 * std::abs (kt_to_px (std::max (0.f, _efis._speed_tendency)))) - 0.5f * x;
	_painter.setClipRect (QRectF (_ladder_rect.topLeft(), QPointF (_ladder_rect.right(), 0.f)));
	if (length > 0)
		_painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, -length));
	_painter.translate (0.f, -length);
	_painter.drawPolygon (QPolygonF()
		<< QPointF (0.f, -0.5f * x)
		<< QPointF (-0.2f * x, 0.f)
		<< QPointF (+0.2f * x, 0.f));
	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_bugs (float x)
{
	if (!_efis._speed_visible)
		return;

	QFont speed_bug_font = _efis._font_10_bold;
	float const speed_bug_digit_height = _efis._font_10_digit_height;

	_painter.save();
	_painter.setFont (speed_bug_font);

	for (auto& bug: _efis._speed_bugs)
	{
		// AT bug should be drawn last, to be on top:
		if (bug.first == AT)
			continue;

		if (bug.second > _min_shown && bug.second < _max_shown)
		{
			float posy = kt_to_px (bug.second);
			_painter.setPen (_speed_bug_pen);
			_painter.setClipRect (_ladder_rect.translated (x, 0.f));
			_painter.drawLine (QPointF (1.5f * x, posy), QPointF (2.25f * x, posy));
			_painter.setClipping (false);
			_text_painter.drawText (QRectF (2.5f * x, posy - 0.5f * speed_bug_digit_height,
											2.f * x, speed_bug_digit_height),
									Qt::AlignVCenter | Qt::AlignLeft, bug.first);
		}
	}

	// AT bug:
	auto ap_bug = _efis._speed_bugs.find (AT);
	if (ap_bug != _efis._speed_bugs.end())
	{
		float posy = bound (kt_to_px (ap_bug->second),
							static_cast<float> (-_ladder_rect.height() / 2.f), static_cast<float> (_ladder_rect.height() / 2.f));
		QPolygonF bug_shape = QPolygonF()
			<< QPointF (0.f, 0.f)
			<< QPointF (+0.5f * x, -0.5f * x)
			<< QPointF (2.f * x, -0.5f * x)
			<< QPointF (2.f * x, +0.5f * x)
			<< QPointF (+0.5f * x, +0.5f * x);
		_painter.setClipRect (_ladder_rect.translated (2.5f * x, 0.f));
		_painter.translate (1.25f * x, posy);
		_painter.setBrush (Qt::NoBrush);
		_painter.setPen (_efis.get_pen (_efis._autopilot_color.darker (400), 2.f)),
		_painter.drawPolygon (bug_shape);
		_painter.setPen (_efis.get_pen (_efis._autopilot_color, 1.2f)),
		_painter.drawPolygon (bug_shape);
	}

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_mach_number (float x)
{
	if (!_efis._mach_visible)
		return;

	_painter.save();
	_painter.translate (0.f, 0.75f * x);

	QFont font_a = _efis._font_16_bold;
	QFont font_b = _efis._font_10_bold;

	QString m_str = "M";
	QString mach_str = " " + QString ("%1").arg (_mach, 0, 'f', 3);

	QRectF nn_rect (0.f, _ladder_rect.bottom(), QFontMetrics (font_a).width (mach_str), 1.2f * _efis._font_16_digit_height);
	QRectF zz_rect (0.f, nn_rect.top(), QFontMetrics (font_b).width (m_str), nn_rect.height());
	zz_rect.moveLeft (-0.5f * (zz_rect.width() + nn_rect.width()));
	// Correct position of zz_rect to get correct baseline position:
	zz_rect.translate (0.f, QFontMetrics (font_b).descent() - QFontMetrics (font_a).descent());
	nn_rect.moveLeft (zz_rect.right());

	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.f));
	_painter.setFont (font_a);
	_text_painter.drawText (nn_rect, Qt::AlignBottom | Qt::AlignLeft, mach_str, true);
	_painter.setFont (font_b);
	_text_painter.drawText (zz_rect, Qt::AlignBottom | Qt::AlignRight, m_str);

	_painter.restore();
}


void
EFISWidget::SpeedLadder::paint_ap_setting (float)
{
	auto ap_bug = _efis._speed_bugs.find (AT);
	if (ap_bug == _efis._speed_bugs.end())
		return;

	QFont actual_speed_font = _efis._font_20_bold;
	float const digit_width = _efis._font_20_digit_width;
	float const digit_height = _efis._font_20_digit_height;

	int digits = 4;
	float const margin = 0.2f * digit_width;

	QRectF digits_box (0.f, 0.f, digits * digit_width + 2.f * margin, 1.3f * digit_height);
	QRectF box_rect (_ladder_rect.right() - digits_box.width(), _ladder_rect.top() - 1.4f * digits_box.height(),
					 digits_box.width(), digits_box.height());

	_painter.save();

	_painter.setPen (_efis.get_pen (QColor (0, 0, 0), 0.5f));
	_painter.setBrush (QBrush (QColor (0, 0, 0)));
	_painter.drawRect (box_rect);

	_painter.setPen (_efis.get_pen (_efis._autopilot_color, 1.f));
	_painter.setFont (actual_speed_font);

	QRectF box = box_rect.adjusted (margin, margin, -margin, -margin);
	_painter.drawText (box, Qt::AlignVCenter | Qt::AlignRight, QString::number (std::abs (static_cast<int> (ap_bug->second))));

	_painter.restore();
}


EFISWidget::AttitudeDirectorIndicator::AttitudeDirectorIndicator (EFISWidget& efis, QPainter& painter):
	_efis (efis),
	_painter (painter),
	_text_painter (_painter, &_efis._text_painter_cache),
	_pitch (_efis._pitch),
	_roll (_efis._roll),
	_heading (_efis._heading)
{
	float p = floored_mod (_pitch + 180.f, 360.f) - 180.f;
	float r = floored_mod (_roll + 180.f, 360.f) - 180.f;
	float hdg = floored_mod (_heading, 360.f);

	// Mirroring, eg. -180° pitch is the same
	// as 0° pitch with roll inverted:
	if (p < -90.f)
	{
		p = -180.f - p;
		r = +180.f - r;
	}
	else if (p > 90.f)
	{
		p = +180.f - p;
		r = +180.f - r;
	}

	_pitch_transform.reset();
	_pitch_transform.translate (0.f, -pitch_to_px (p));

	_roll_transform.reset();
	_roll_transform.rotate (-r);

	_heading_transform.reset();
	_heading_transform.translate (-heading_to_px (hdg), 0.f);

	// Total transform of horizon (heading is not really necessary here):
	_horizon_transform = _pitch_transform * _roll_transform;
}


void
EFISWidget::AttitudeDirectorIndicator::paint()
{
	paint_horizon();
	paint_flight_path_marker();
	paint_pitch();
	paint_roll();
	paint_heading();
}


void
EFISWidget::AttitudeDirectorIndicator::paint_horizon()
{
	_painter.save();

	if (_efis._pitch_visibility && _efis._roll_visibility)
	{
		_painter.setTransform (_horizon_transform * _efis._center_transform);

		float const max = std::max (_efis.width(), _efis.height());
		float const w_max = 2.f * max;
		float const h_max = 10.f * max;
		// Sky and ground:
		_painter.fillRect (-w_max, -h_max, 2.f * w_max, h_max + 1.f, QBrush (_efis._sky_color, Qt::SolidPattern));
		_painter.fillRect (-w_max, 0.f, 2.f * w_max, h_max, QBrush (_efis._ground_color, Qt::SolidPattern));

	}
	else
	{
		_painter.resetTransform();
		_painter.setPen (Qt::NoPen);
		_painter.setBrush (QBrush (QColor (0, 0, 0)));
		_painter.drawRect (_efis.rect());
	}

	_painter.restore();
}


// TODO refactor
void
EFISWidget::AttitudeDirectorIndicator::paint_pitch()
{
	if (!_efis._pitch_visibility)
		return;

	float const w = _efis.wh() * 2.f / 9.f;
	float const z = 0.5f * w;
	float const fpxs = _efis._font_10_bold.pixelSize();

	// TODO cache pens, name them correctly

	_painter.save();

	// Clip rectangle before and after rotation:
	_painter.setClipPath (get_pitch_scale_clipping_path() - _flight_path_marker);
	_painter.setTransform (_roll_transform * _efis._center_transform);
	_painter.setClipRect (QRectF (-w, -0.9f * w, 2.f * w, 2.2f * w), Qt::IntersectClip);
	_painter.setTransform (_horizon_transform * _efis._center_transform);
	_painter.setFont (_efis._font_10_bold);

	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.f));
	// 10° lines, exclude +/-90°:
	for (int deg = -180; deg < 180; deg += 10)
	{
		if (deg == -90 || deg == 0 || deg == 90)
			continue;
		float d = pitch_to_px (deg);
		_painter.drawLine (QPointF (-z, d), QPointF (z, d));
		// Degs number:
		int abs_deg = std::abs (deg);
		QString deg_t = QString::number (abs_deg > 90 ? 180 - abs_deg : abs_deg);
		// Text:
		QRectF lbox (-z - 4.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		QRectF rbox (+z + 0.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		_text_painter.drawText (lbox, Qt::AlignVCenter | Qt::AlignRight, deg_t);
		_text_painter.drawText (rbox, Qt::AlignVCenter | Qt::AlignLeft, deg_t);
	}
	// 5° lines:
	for (int deg = -180; deg < 180; deg += 5)
	{
		if (deg % 10 == 0)
			continue;
		float d = pitch_to_px (deg);
		_painter.drawLine (QPointF (-z / 2.f, d), QPointF (z / 2.f, d));
	}
	// 2.5° lines:
	for (int deg = -1800; deg < 1800; deg += 25)
	{
		if (deg % 50 == 0)
			continue;
		float d = pitch_to_px (deg / 10.f);
		_painter.drawLine (QPointF (-z / 4.f, d), QPointF (z / 4.f, d));
	}

	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.75f));
	// -90°, 90° lines:
	for (float deg: { -90.f, 90.f })
	{
		float d = pitch_to_px (deg);
		_painter.drawLine (QPointF (-z * 1.5f, d), QPointF (z * 1.5f, d));
		QRectF lbox (-1.5f * z - 4.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		QRectF rbox (+1.5f * z + 0.25f * fpxs, d - 0.5f * fpxs, 4.f * fpxs, fpxs);
		_text_painter.drawText (lbox, Qt::AlignVCenter | Qt::AlignRight, "90");
		_text_painter.drawText (rbox, Qt::AlignVCenter | Qt::AlignLeft, "90");
	}

	_painter.restore();
}


// TODO refactor
void
EFISWidget::AttitudeDirectorIndicator::paint_roll()
{
	if (!_efis._roll_visibility)
		return;

	float const w = _efis.wh() * 3.f / 9.f;

	_painter.save();

	QPen pen = _efis.get_pen (QColor (255, 255, 255), 1.f);
	_painter.setPen (pen);
	_painter.setBrush (QBrush (QColor (255, 255, 255)));

	_painter.setTransform (_efis._center_transform);
	_painter.setClipRect (QRectF (-w, -w, 2.f * w, 2.25f * w));
	for (float deg: { -60.f, -45.f, -30.f, -20.f, -10.f, 0.f, +10.f, +20.f, +30.f, +45.f, +60.f })
	{
		_painter.setTransform (_efis._center_transform);
		_painter.rotate (1.f * deg);
		_painter.translate (0.f, -0.795f * w);

		if (deg == 0.f)
		{
			// Triangle:
			QPointF p0 (0.f, 0.f);
			QPointF px (0.025f * w, 0.f);
			QPointF py (0.f, 0.05f * w);
			_painter.drawPolygon (QPolygonF() << p0 << p0 - px - py << p0 + px - py);
		}
		else
		{
			float length = -0.05f * w;
			if (std::abs (std::fmod (deg, 30.f)) < 1.f)
				length *= 2.f;
			_painter.drawLine (QPointF (0.f, 0.f), QPointF (0.f, length));
		}
	}

	float const bold_width = _efis.pen_width (3.f);
	QPointF a (0, 0.01f * w); // Miter
	QPointF b (-0.052f * w, 0.1f * w);
	QPointF c (+0.052f * w, 0.1f * w);
	QPointF x0 (0.001f * w, 0.f);
	QPointF y0 (0.f, 0.005f * w);
	QPointF x1 (0.001f * w, 0.f);
	QPointF y1 (0.f, 1.f * bold_width);

	_painter.setTransform (_roll_transform * _efis._center_transform);
	_painter.translate (0.f, -0.79f * w);
	_painter.setBrush (QBrush (QColor (255, 255, 255)));
	_painter.drawPolyline (QPolygonF() << b << a << c);
	_painter.drawPolygon (QPolygonF() << b - x0 + y0 << b + x1 + y1 << c - x1 + y1 << c + x0 + y0);

	_painter.restore();
}


// TODO refactor
void
EFISWidget::AttitudeDirectorIndicator::paint_heading()
{
	float const w = _efis.wh() * 2.25f / 9.f;
	float const fpxs = _efis._font_10_bold.pixelSize();

	if (!_efis._pitch_visibility || !_efis._roll_visibility)
		return;

	_painter.save();
	// Clip rectangle before and after rotation:
	_painter.setTransform (_efis._center_transform);
	_painter.setClipPath (get_pitch_scale_clipping_path());
	_painter.setTransform (_roll_transform * _efis._center_transform);
	_painter.setClipRect (QRectF (-1.1f * w, -0.8f * w, 2.2f * w, 1.9f * w), Qt::IntersectClip);
	_painter.setTransform (_horizon_transform * _efis._center_transform);
	_painter.setFont (_efis._font_10_bold);

	_painter.setTransform (_horizon_transform * _efis._center_transform);
	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.25f));
	_painter.drawLine (QPointF (-1.25 * w, 0.f), QPointF (1.25f * w, 0.f));
	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.f));

	if (!_efis._heading_visibility)
	{
		_painter.restore();
		return;
	}

	_painter.setTransform (_heading_transform * _horizon_transform * _efis._center_transform);
	for (int deg = -360; deg < 450; deg += 10)
	{
		float d10 = heading_to_px (deg);
		float d05 = heading_to_px (deg + 5);
		QString text = QString::number (floored_mod (1.f * deg, 360.f) / 10);
		if (text == "0")
			text = "N";
		else if (text == "9")
			text = "E";
		else if (text == "18")
			text = "S";
		else if (text == "27")
			text = "W";
		// 10° lines:
		_painter.drawLine (QPointF (d10, -w / 18.f), QPointF (d10, 0.f));
		_text_painter.drawText (QRectF (d10 - 2.f * fpxs, 0.05f * fpxs, 4.f * fpxs, fpxs),
								Qt::AlignVCenter | Qt::AlignHCenter, text);
		// 5° lines:
		_painter.drawLine (QPointF (d05, -w / 36.f), QPointF (d05, 0.f));
	}

	_painter.restore();
}


void
EFISWidget::AttitudeDirectorIndicator::paint_flight_path_marker()
{
	if (!_efis._flight_path_visible)
		return;

	float x = 0.013f * _efis.wh();
	float w = _efis.pen_width (3.f);
	float r = 0.5f * w;

	QPointF marker_position (-heading_to_px (_efis._flight_path_beta), -pitch_to_px (_efis._flight_path_alpha));
	_painter.save();

	_flight_path_marker = QPainterPath();
	_flight_path_marker.setFillRule (Qt::WindingFill);
	_flight_path_marker.addEllipse (QRectF (-x - 0.5f * w, -x - 0.5f * w, 2.f * x + w, 2.f * x + w));
	_flight_path_marker.addRoundedRect (QRectF (-4.f * x - 0.5f * w, -0.5f * w, +3.f * x + w, w), r, r);
	_flight_path_marker.addRoundedRect (QRectF (+1.f * x - 0.5f * w, -0.5f * w, +3.f * x + w, w), r, r);
	_flight_path_marker.addRoundedRect (QRectF (-0.5f * w, -2.f * x - 0.5f * w, w, x + w), r, r);
	_flight_path_marker.translate (marker_position);

	auto draw_marker = [&]() -> void {
		_painter.drawEllipse (QRectF (-x, -x, 2.f * x, 2.f * x));
		_painter.drawLine (QPointF (+x, 0.f), QPointF (+4.f * x, 0.f));
		_painter.drawLine (QPointF (-x, 0.f), QPointF (-4.f * x, 0.f));
		_painter.drawLine (QPointF (0.f, -x), QPointF (0.f, -2.f * x));
	};

	_painter.setClipRect (QRectF (-0.325f * _efis.wh(), -0.4f * _efis.wh(), 0.65f * _efis.wh(), 0.8f * _efis.wh()));
	_painter.translate (marker_position);
	_painter.setPen (_efis.get_pen (QColor (255, 255, 255), 1.25f));
	draw_marker();

	_painter.restore();
}


QPainterPath
EFISWidget::AttitudeDirectorIndicator::get_pitch_scale_clipping_path() const
{
	float const w = _efis.wh() * 2.f / 9.f;

	QPainterPath clip_path;
	clip_path.setFillRule (Qt::WindingFill);
	clip_path.addEllipse (QRectF (-1.15f * w, -1.175f * w, 2.30f * w, 2.35f * w));
	clip_path.addRect (QRectF (-1.15f * w, 0.f, 2.30f * w, 1.375f * w));

	return clip_path - _flight_path_marker;
}


EFISWidget::EFISWidget (QWidget* parent):
	QWidget (parent, nullptr)
{
	setAttribute (Qt::WA_NoBackground);
	_sky_color.setHsv (213, 217, 255);
	_ground_color.setHsv (34, 233, 127);
	_ladder_color = QColor (51, 38, 93, 0x80);
	_autopilot_color = QColor (250, 120, 255);
	_navigation_color = QColor (40, 255, 40);
	_ladder_border_color = QColor (0, 0, 0, 0x70);
	_font = Xefis::Services::instrument_font();

	update_fonts();
}


void
EFISWidget::paintEvent (QPaintEvent* paint_event)
{
	float const w = width();
	float const h = height();

	_center_transform.reset();
	_center_transform.translate (w / 2.f, h / 2.f);

	// Draw on buffer:
	QPixmap buffer (w, h);
	QPainter painter (&buffer);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);

	painter.setTransform (_center_transform);

	if (_input_alert_visible)
		paint_input_alert (painter);
	else
	{
		painter.save();
		AttitudeDirectorIndicator adi (*this, painter);
		adi.paint();
		painter.restore();

		paint_center_cross (painter);
		paint_altitude_agl (painter);

		painter.save();
		SpeedLadder sl (*this, painter);
		painter.translate (-0.4f * wh(), 0.f);
		sl.paint();
		painter.restore();

		painter.save();
		AltitudeLadder al (*this, painter);
		painter.translate (+0.4f * wh(), 0.f);
		al.paint();
		painter.restore();
	}

	// Copy buffer to screen:
	QPainter (this).drawPixmap (paint_event->rect().topLeft(), buffer, paint_event->rect());
}


void
EFISWidget::resizeEvent (QResizeEvent*)
{
	update_fonts();
}


void
EFISWidget::paint_center_cross (QPainter& painter)
{
	float const w = wh() * 3.f / 9.f;

	QPen white_pen = get_pen (QColor (255, 255, 255), 1.5f);

	painter.save();

	painter.setTransform (_center_transform);
	painter.setPen (white_pen);
	painter.setBrush (QBrush (QColor (0, 0, 0)));

	QPointF x (0.025f * w, 0.f);
	QPointF y (0.f, 0.025f * w);
	QPolygonF a = QPolygonF()
		<< -x - y
		<<  x - y
		<<  x + y
		<< -x + y;
	QPolygonF b = QPolygonF()
		<< -25.f * x - y
		<< -11.f * x - y
		<< -11.f * x + 4.f * y
		<< -13.f * x + 4.f * y
		<< -13.f * x + y
		<< -25.f * x + y;

	painter.drawPolygon (a);
	painter.drawPolygon (b);
	painter.scale (-1.f, 1.f);
	painter.drawPolygon (b);

	painter.restore();
}


void
EFISWidget::paint_altitude_agl (QPainter& painter)
{
	if (!_altitude_agl_visible)
		return;

	float aagl = bound (_altitude_agl, -9999.f, +99999.f);
	QFont radar_altimeter_font = _font_20_bold;
	float const digit_width = _font_20_digit_width;
	float const digit_height = _font_20_digit_height;

	int digits = 4;
	if (_altitude_agl > 9999)
		digits = 5;
	float const margin = 0.2f * digit_width;

	QRectF box_rect (0.f, 0.f, digits * digit_width + 2.f * margin, 1.3f * digit_height);
	box_rect.translate (-box_rect.width() / 2.f, 0.35f * wh());

	painter.save();

	painter.setPen (get_pen (QColor (0, 0, 0), 1.f));
	painter.setBrush (QBrush (QColor (0, 0, 0)));
	painter.drawRect (box_rect);

	painter.setPen (get_pen (QColor (255, 255, 255), 1.f));
	painter.setFont (radar_altimeter_font);

	QRectF box = box_rect.adjusted (margin, margin, -margin, -margin);
	painter.drawText (box, Qt::AlignVCenter | Qt::AlignHCenter, QString ("%1").arg (std::round (aagl)));

	painter.restore();
}


void
EFISWidget::paint_input_alert (QPainter& painter)
{
	painter.save();

	QFont font = _font;
	font.setPixelSize (font_size (30.f));
	font.setBold (true);

	QString alert = "NO INPUT";

	QFontMetrics font_metrics (font);
	int width = font_metrics.width (alert);

	QPen pen = get_pen (QColor (255, 255, 255), 2.f);

	painter.resetTransform();
	painter.setPen (Qt::NoPen);
	painter.setBrush (QBrush (QColor (0, 0, 0)));
	painter.drawRect (rect());

	painter.setTransform (_center_transform);
	painter.setPen (pen);
	painter.setBrush (QBrush (QColor (0xdd, 0, 0)));
	painter.setFont (font);

	QRectF rect (-0.6f * width, -0.5f * font_metrics.height(), 1.2f * width, 1.2f * font_metrics.height());

	painter.drawRect (rect);
	painter.drawText (rect, Qt::AlignVCenter | Qt::AlignHCenter, alert);

	painter.restore();
}


int
EFISWidget::get_digit_width (QFont& font) const
{
	QFontMetrics font_metrics (font);
	int digit_width = 0;
	for (char c: DIGITS)
		digit_width = std::max (digit_width, font_metrics.width (c));
	return digit_width;
}


void
EFISWidget::update_fonts()
{
	float const height_scale_factor = 0.7f;

	_font_10_bold = _font;
	_font_10_bold.setPixelSize (font_size (10.f));
	_font_10_bold.setBold (true);
	_font_10_digit_width = get_digit_width (_font_10_bold);
	_font_10_digit_height = height_scale_factor * QFontMetrics (_font_10_bold).height();

	_font_13_bold = _font;
	_font_13_bold.setPixelSize (font_size (13.f));
	_font_13_bold.setBold (true);
	_font_13_digit_width = get_digit_width (_font_13_bold);
	_font_13_digit_height = QFontMetrics (_font_13_bold).height();
	_font_13_digit_height = height_scale_factor * QFontMetrics (_font_13_bold).height();

	_font_16_bold = _font;
	_font_16_bold.setPixelSize (font_size (16.f));
	_font_16_bold.setBold (true);
	_font_16_digit_width = get_digit_width (_font_16_bold);
	_font_16_digit_height = QFontMetrics (_font_16_bold).height();
	_font_16_digit_height = height_scale_factor * QFontMetrics (_font_16_bold).height();

	_font_20_bold = _font;
	_font_20_bold.setPixelSize (font_size (20.f));
	_font_20_bold.setBold (true);
	_font_20_digit_width = get_digit_width (_font_20_bold);
	_font_20_digit_height = QFontMetrics (_font_20_bold).height();
	_font_20_digit_height = height_scale_factor * QFontMetrics (_font_20_bold).height();
}

