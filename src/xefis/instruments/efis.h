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

#ifndef XEFIS__INSTRUMENTS__EFIS_H__INCLUDED
#define XEFIS__INSTRUMENTS__EFIS_H__INCLUDED

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QPaintEvent>
#include <QtGui/QColor>
#include <QtGui/QPixmap>

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


class EFIS: public QWidget
{
	Q_OBJECT

  public:
	EFIS (QWidget* parent);

	Degrees
	roll() const;

	void
	set_roll (Degrees);

	Degrees
	pitch() const;

	void
	set_pitch (Degrees);

	Degrees
	heading() const;

	void
	set_heading (Degrees);

	/**
	 * Return field of view.
	 * Default is 120°. Usable maximum: 180°.
	 */
	Degrees
	fov() const;

	/**
	 * Set field of view.
	 */
	void
	set_fov (Degrees);

	void
	set_sky_color (QColor const&);

	void
	set_ground_color (QColor const&);

  public slots:
	// XXX
	void
	test();

  protected:
	void
	paintEvent (QPaintEvent*) override;

	void
	paint_horizon (QPainter& painter);

	void
	paint_pitch_scale (QPainter& painter);

	void
	paint_heading (QPainter& heading);

	void
	paint_roll (QPainter& painter);

  private:
	float
	pitch_to_px (Degrees degrees) const;

	float
	heading_to_px (Degrees degrees) const;

	float
	pen_width (float scale = 1.0f) const;

  private:
	QColor		_sky_color;
	QColor		_ground_color;
	QTransform	_center_transform;
	QTransform	_pitch_transform;
	QTransform	_roll_transform;
	QTransform	_heading_transform;
	QTransform	_horizon_transform;
	QFont		_font;
	Degrees		_fov		= 120.f;

	// Parameters:
	Degrees		_pitch		= 0.f;
	Degrees		_roll		= 0.f;
	Degrees		_heading	= 0.f;
};


inline Degrees
EFIS::roll() const
{
	return _roll;
}


inline void
EFIS::set_roll (Degrees degrees)
{
	_roll = degrees;
	update();
}


inline Degrees
EFIS::pitch() const
{
	return _pitch;
}


inline void
EFIS::set_pitch (Degrees degrees)
{
	_pitch = degrees;
	update();
}


inline Degrees
EFIS::heading() const
{
	return _heading;
}


inline void
EFIS::set_heading (Degrees degrees)
{
	_heading = degrees;
	update();
}


inline Degrees
EFIS::fov() const
{
	return _fov;
}


inline void
EFIS::set_fov (Degrees degrees)
{
	_fov = degrees;
	update();
}


inline void
EFIS::set_sky_color (QColor const& color)
{
	_sky_color = color;
	update();
}


inline void
EFIS::set_ground_color (QColor const& color)
{
	_ground_color = color;
	update();
}


inline float
EFIS::pitch_to_px (Degrees degrees) const
{
	return -degrees / _fov * std::max (width(), height());
}


inline float
EFIS::heading_to_px (Degrees degrees) const
{
	return pitch_to_px (-degrees);
}

inline float
EFIS::pen_width (float scale) const
{
	return scale * std::max (width(), height()) / 500.f;
}

#endif

