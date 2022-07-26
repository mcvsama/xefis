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

#ifndef XEFIS__CORE__SCREEN_SPEC_H__INCLUDED
#define XEFIS__CORE__SCREEN_SPEC_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qutils.h>

// Qt:
#include <QRect>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Basic screen information.
 */
class ScreenSpec
{
  public:
	// Ctor
	explicit
	ScreenSpec (QRect position_and_size,
				si::Length diagonal_length,
				si::Frequency refresh_rate,
				si::Length base_pen_width,
				si::Length base_font_height);

	/**
	 * Position and size of the display area on the screen.
	 * If not defined, whole screen area should be used.
	 */
	[[nodiscard]]
	QRect
	position_and_size() const noexcept;

	/**
	 * Diagonal length of the screen widget.
	 */
	[[nodiscard]]
	si::Length
	diagonal_length() const noexcept;

	/**
	 * Requested repaint rate of the instruments.
	 */
	[[nodiscard]]
	si::Frequency
	refresh_rate() const noexcept;

	/**
	 * Base pen width.
	 */
	[[nodiscard]]
	si::Length
	base_pen_width() const noexcept;

	/**
	 * Base font height.
	 */
	[[nodiscard]]
	si::Length
	base_font_height() const noexcept;

	/**
	 * Return pixel density for this screen.
	 */
	[[nodiscard]]
	si::PixelDensity
	pixel_density() const noexcept;

	/**
	 * Set screen scaling factor. Affects returned dimensions, diagonal length and base sizes.
	 */
	void
	set_scale (float factor);

  private:
	float				_scale				{ 1.0f };
	// Qt doesn't seem to scale fonts correctly, this is to mitigate that problem:
	float				_font_scale_fix		{ 1.0f };
	QRect				_position_and_size;
	si::Length			_diagonal_length;
	si::Frequency		_refresh_rate;
	si::Length			_base_pen_width;
	si::Length			_base_font_height;
	si::PixelDensity	_pixel_density;
};


inline
ScreenSpec::ScreenSpec (QRect position_and_size,
						si::Length diagonal_length,
						si::Frequency refresh_rate,
						si::Length base_pen_width,
						si::Length base_font_height):
	_position_and_size (position_and_size),
	_diagonal_length (diagonal_length),
	_refresh_rate (refresh_rate),
	_base_pen_width (base_pen_width),
	_base_font_height (base_font_height)
{ }


inline QRect
ScreenSpec::position_and_size() const noexcept
{
	return QRect { _position_and_size.topLeft(), _position_and_size.size() * _scale };
}


inline si::Length
ScreenSpec::diagonal_length() const noexcept
{
	return _diagonal_length;
}


inline si::Frequency
ScreenSpec::refresh_rate() const noexcept
{
	return _refresh_rate;
}


inline si::Length
ScreenSpec::base_pen_width() const noexcept
{
	return _base_pen_width;
}


inline si::Length
ScreenSpec::base_font_height() const noexcept
{
	return _base_font_height / _font_scale_fix;
}


inline si::PixelDensity
ScreenSpec::pixel_density() const noexcept
{
	return xf::diagonal (_position_and_size.size()) / _diagonal_length * _scale;
}


inline void
ScreenSpec::set_scale (float factor)
{
	_scale = factor;
	_font_scale_fix = std::pow (_scale, 0.3);
}

} // namespace xf

#endif

