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

#ifndef XEFIS__CORE__V2__PAINT_REQUEST_H__INCLUDED
#define XEFIS__CORE__V2__PAINT_REQUEST_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QImage>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/types.h>


namespace xf {

class PaintRequest: private Noncopyable
{
  public:
	class Metric
	{
	  public:
		// Ctor
		explicit
		Metric (PixelDensity, si::Length pen_width, si::Length font_height);

		bool
		operator== (Metric const&);

		bool
		operator!= (Metric const&);

		PixelDensity
		pixel_density() const noexcept;

		si::Length
		pen_width() const noexcept;

		si::Length
		font_height() const noexcept;

	  private:
		PixelDensity	_pixel_density;
		si::Length		_pen_width;
		si::Length		_font_height;
	};

  public:
	// Ctor
	explicit
	PaintRequest (QImage&, Metric);

	QImage&
	canvas() noexcept;

	QImage const&
	canvas() const noexcept;

	Metric const&
	metric() const noexcept;

  private:
	QImage&	_canvas;
	Metric	_metric;
};


inline
PaintRequest::Metric::Metric (PixelDensity pixel_density, si::Length pen_width, si::Length font_height):
	_pixel_density (pixel_density),
	_pen_width (pen_width),
	_font_height (font_height)
{ }


inline bool
PaintRequest::Metric::operator== (PaintRequest::Metric const& other)
{
	return _pixel_density == other._pixel_density
		&& _pen_width == other._pen_width
		&& _font_height == other._font_height;
}


inline bool
PaintRequest::Metric::operator!= (PaintRequest::Metric const& other)
{
	return !(*this == other);
}


inline PixelDensity
PaintRequest::Metric::pixel_density() const noexcept
{
	return _pixel_density;
}


inline si::Length
PaintRequest::Metric::pen_width() const noexcept
{
	return _pen_width;
}


inline si::Length
PaintRequest::Metric::font_height() const noexcept
{
	return _font_height;
}


inline
PaintRequest::PaintRequest (QImage& canvas, Metric metric):
	_canvas (canvas),
	_metric (metric)
{ }


inline QImage&
PaintRequest::canvas() noexcept
{
	return _canvas;
}


inline QImage const&
PaintRequest::canvas() const noexcept
{
	return _canvas;
}


inline PaintRequest::Metric const&
PaintRequest::metric() const noexcept
{
	return _metric;
}

} // namespace xf

#endif

