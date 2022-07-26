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

#ifndef XEFIS__CORE__PAINT_REQUEST_H__INCLUDED
#define XEFIS__CORE__PAINT_REQUEST_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/types.h>

// Neutrino:
#include <neutrino/noncopyable.h>

// Qt:
#include <QPaintDevice>

// Standard:
#include <cstddef>
#include <atomic>


namespace xf {

class PaintRequest
{
  public:
	class Metric
	{
	  public:
		// Ctor
		explicit
		Metric (QSize canvas_size, si::PixelDensity, si::Length pen_width, si::Length font_height);

		[[nodiscard]]
		bool
		operator== (Metric const&) const noexcept;

		[[nodiscard]]
		bool
		operator!= (Metric const&) const noexcept;

		[[nodiscard]]
		QSize
		canvas_size() const noexcept
			{ return _canvas_size; }

		[[nodiscard]]
		QRect
		canvas_rect() const noexcept
			{ return QRect (QPoint (0, 0), _canvas_size); }

		[[nodiscard]]
		si::PixelDensity
		pixel_density() const noexcept;

		[[nodiscard]]
		si::Length
		pen_width() const noexcept;

		[[nodiscard]]
		si::Length
		font_height() const noexcept;

	  private:
		QSize				_canvas_size;
		si::PixelDensity	_pixel_density;
		si::Length			_pen_width;
		si::Length			_font_height;
	};

  public:
	// Ctor
	explicit
	PaintRequest (QPaintDevice&, Metric const&, QSize previous_canvas_size);

	// Move ctor
	PaintRequest (PaintRequest&&) = default;

	// Move operator
	PaintRequest&
	operator= (PaintRequest&&) = default;

	/**
	 * Access the canvas to paint on.
	 */
	[[nodiscard]]
	QPaintDevice&
	canvas() const noexcept;

	/**
	 * Return graphics metrics.
	 */
	[[nodiscard]]
	Metric const&
	metric() const noexcept;

	/**
	 * Return true if canvas size has changed since last painting request.
	 */
	[[nodiscard]]
	bool
	size_changed() const noexcept;

  private:
	QPaintDevice*	_canvas;
	Metric			_metric;
	bool			_size_changed;
};


inline
PaintRequest::Metric::Metric (QSize canvas_size, si::PixelDensity pixel_density, si::Length pen_width, si::Length font_height):
	_canvas_size (canvas_size),
	_pixel_density (pixel_density),
	_pen_width (pen_width),
	_font_height (font_height)
{ }


inline bool
PaintRequest::Metric::operator== (PaintRequest::Metric const& other) const noexcept
{
	return _canvas_size == other._canvas_size
		&& _pixel_density == other._pixel_density
		&& _pen_width == other._pen_width
		&& _font_height == other._font_height;
}


inline bool
PaintRequest::Metric::operator!= (PaintRequest::Metric const& other) const noexcept
{
	return !(*this == other);
}


inline si::PixelDensity
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
PaintRequest::PaintRequest (QPaintDevice& canvas, Metric const& metric, QSize previous_canvas_size):
	_canvas (&canvas),
	_metric (metric),
	_size_changed (QSize (canvas.width(), canvas.height()) != previous_canvas_size)
{ }


inline QPaintDevice&
PaintRequest::canvas() const noexcept
{
	return *_canvas;
}


inline PaintRequest::Metric const&
PaintRequest::metric() const noexcept
{
	return _metric;
}


inline bool
PaintRequest::size_changed() const noexcept
{
	return _size_changed;
}

} // namespace xf

#endif

