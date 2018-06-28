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

// Standard:
#include <cstddef>
#include <atomic>

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
		Metric (QSize, PixelDensity, si::Length pen_width, si::Length font_height);

		bool
		operator== (Metric const&) const noexcept;

		bool
		operator!= (Metric const&) const noexcept;

		QSize
		canvas_size() const noexcept;

		PixelDensity
		pixel_density() const noexcept;

		si::Length
		pen_width() const noexcept;

		si::Length
		font_height() const noexcept;

	  private:
		QSize			_canvas_size;
		PixelDensity	_pixel_density;
		si::Length		_pen_width;
		si::Length		_font_height;
	};

  public:
	// Ctor
	explicit
	PaintRequest (QImage&, Metric, QSize previous_canvas_size);

	// Move ctor
	PaintRequest (PaintRequest&&) = default;

	// Move operator
	PaintRequest&
	operator= (PaintRequest&&) = default;

	/**
	 * Access the canvas to paint on.
	 */
	QImage&
	canvas() noexcept;

	/**
	 * Access the canvas to paint on.
	 */
	QImage const&
	canvas() const noexcept;

	/**
	 * Return graphics metrics.
	 */
	Metric const&
	metric() const noexcept;

	/**
	 * Return true if canvas size has changed since last painting request.
	 */
	bool
	size_changed() const noexcept;

	/**
	 * Inform the system that the result will be painted asynchronously.
	 * After it's done, API user is required to call finished().
	 */
	void
	will_finish_asynchronously() noexcept;

	/**
	 * Inform the system that the asynchronous painting has finished.
	 * After calling this method, calling any other method on this object
	 * is undefined-behaviour.
	 */
	void
	set_finished() noexcept;

	/**
	 * Return true if request was marked as finished.
	 */
	bool
	finished() const noexcept;

  private:
	std::atomic<bool>	_finished { true };
	QImage*				_canvas;
	Metric				_metric;
	bool				_size_changed;
};


inline
PaintRequest::Metric::Metric (QSize canvas_size, PixelDensity pixel_density, si::Length pen_width, si::Length font_height):
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


inline QSize
PaintRequest::Metric::canvas_size() const noexcept
{
	return _canvas_size;
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
PaintRequest::PaintRequest (QImage& canvas, Metric metric, QSize previous_canvas_size):
	_canvas (&canvas),
	_metric (metric),
	_size_changed (canvas.size() != previous_canvas_size)
{ }


inline QImage&
PaintRequest::canvas() noexcept
{
	return *_canvas;
}


inline QImage const&
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


inline void
PaintRequest::will_finish_asynchronously() noexcept
{
	_finished.store (false);
}


inline void
PaintRequest::set_finished() noexcept
{
	_finished.store (true);
}


inline bool
PaintRequest::finished() const noexcept
{
	return _finished.load();
}

} // namespace xf

#endif

