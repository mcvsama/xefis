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
#include <xefis/utility/time_helper.h>
#include <xefis/utility/types.h>


namespace xf {

class PaintRequest;


/**
 * RAII-style accessor to the PaintRequest object.
 * Marks the PaintRequest as finished automatically when AsyncPaintRequest is destroyed.
 */
class AsyncPaintRequest
{
  public:
	// Ctor
	explicit
	AsyncPaintRequest (PaintRequest&);

	// Copy ctor
	AsyncPaintRequest (AsyncPaintRequest const&) = delete;

	// Move ctor
	AsyncPaintRequest (AsyncPaintRequest&&);

	// Copy operator
	AsyncPaintRequest&
	operator= (AsyncPaintRequest const&) = delete;

	// Move operator
	AsyncPaintRequest&
	operator= (AsyncPaintRequest&&);

	// Dtor
	~AsyncPaintRequest();

	/**
	 * Access the PaintRequest object.
	 */
	[[nodiscard]]
	PaintRequest&
	paint_request() noexcept;

	/**
	 * Access the PaintRequest object.
	 */
	[[nodiscard]]
	PaintRequest const&
	paint_request() const noexcept;

  private:
	PaintRequest* _paint_request;
};


class PaintRequest: private Noncopyable
{
	friend class AsyncPaintRequest;

  public:
	class Metric
	{
	  public:
		// Ctor
		explicit
		Metric (QSize, PixelDensity, si::Length pen_width, si::Length font_height);

		[[nodiscard]]
		bool
		operator== (Metric const&) const noexcept;

		[[nodiscard]]
		bool
		operator!= (Metric const&) const noexcept;

		[[nodiscard]]
		QSize
		canvas_size() const noexcept;

		[[nodiscard]]
		PixelDensity
		pixel_density() const noexcept;

		[[nodiscard]]
		si::Length
		pen_width() const noexcept;

		[[nodiscard]]
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

	/**
	 * Access the canvas to paint on.
	 */
	[[nodiscard]]
	QImage&
	canvas() noexcept;

	/**
	 * Access the canvas to paint on.
	 */
	[[nodiscard]]
	QImage const&
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

	/**
	 * Get the token that will allow painting asynchronously.
	 * This PaintRequest will be marked as finished when the returned token is destroyed.
	 * When it happens, calling any method on this object is undefined-behaviour.
	 */
	[[nodiscard]]
	AsyncPaintRequest
	make_async();

	/**
	 * Return true if request was marked as finished.
	 */
	[[nodiscard]]
	bool
	finished() const noexcept;

	/**
	 * Setup the started-at timestamp.
	 */
	void
	set_started_at (si::Time);

	/**
	 * Return the time at which rendering was started.
	 */
	[[nodiscard]]
	std::optional<si::Time>
	started_at() const noexcept;

	/**
	 * Return the time at which rendering was finished.
	 */
	[[nodiscard]]
	std::optional<si::Time>
	finished_at() const noexcept;

  private:
	std::atomic<bool>		_finished		{ true };
	std::optional<si::Time>	_started_at;
	std::optional<si::Time>	_finished_at;
	QImage*					_canvas;
	Metric					_metric;
	bool					_size_changed;
};


inline
AsyncPaintRequest::AsyncPaintRequest (PaintRequest& paint_request):
	_paint_request (&paint_request)
{
	_paint_request->_finished = false;
}


inline
AsyncPaintRequest::AsyncPaintRequest (AsyncPaintRequest&& other):
	_paint_request (other._paint_request)
{
	other._paint_request = nullptr;
}


inline
AsyncPaintRequest::~AsyncPaintRequest()
{
	if (_paint_request)
	{
		_paint_request->_finished_at = TimeHelper::now();
		_paint_request->_finished = true;
	}
}


inline PaintRequest&
AsyncPaintRequest::paint_request() noexcept
{
	return *_paint_request;
}


inline PaintRequest const&
AsyncPaintRequest::paint_request() const noexcept
{
	return *_paint_request;
}


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


inline AsyncPaintRequest
PaintRequest::make_async()
{
	return AsyncPaintRequest (*this);
}


inline bool
PaintRequest::finished() const noexcept
{
	return _finished;
}


inline void
PaintRequest::set_started_at (si::Time time)
{
	_started_at = time;
}


inline std::optional<si::Time>
PaintRequest::started_at() const noexcept
{
	return _started_at;
}


inline std::optional<si::Time>
PaintRequest::finished_at() const noexcept
{
	return _finished_at;
}

} // namespace xf

#endif

