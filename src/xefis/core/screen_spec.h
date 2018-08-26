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

// Standard:
#include <cstddef>
#include <optional>

// Qt:
#include <QRect>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

/**
 * Basic screen information.
 */
class ScreenSpec
{
  public:
	// Ctor
	explicit
	ScreenSpec (std::optional<QRect> position_and_size,
				std::optional<si::Length> diagonal_length,
				si::Frequency refresh_rate,
				si::Length base_pen_width,
				si::Length base_font_height);

	/**
	 * Position and size of the display area on the screen.
	 * If not defined, whole screen area should be used.
	 */
	std::optional<QRect>
	position_and_size() const noexcept;

	/**
	 * Diagonal length of the screen widget.
	 */
	std::optional<si::Length>
	diagonal_length() const noexcept;

	/**
	 * Requested repaint rate of the instruments.
	 */
	si::Frequency
	refresh_rate() const noexcept;

	/**
	 * Set base pen width.
	 */
	si::Length
	base_pen_width() const noexcept;

	/**
	 * Set base font height.
	 */
	si::Length
	base_font_height() const noexcept;

	/**
	 * Set screen scaling factor. Affects returned dimensions and diagonal length.
	 */
	void
	set_scale (float factor);

  private:
	float						_scale { 1.0f };
	std::optional<QRect>		_position_and_size;
	std::optional<si::Length>	_diagonal_length;
	si::Frequency				_refresh_rate;
	si::Length					_base_pen_width;
	si::Length					_base_font_height;
};


inline
ScreenSpec::ScreenSpec (std::optional<QRect> position_and_size,
						std::optional<si::Length> diagonal_length,
						si::Frequency refresh_rate,
						si::Length base_pen_width,
						si::Length base_font_height):
	_position_and_size (position_and_size),
	_diagonal_length (diagonal_length),
	_refresh_rate (refresh_rate),
	_base_pen_width (base_pen_width),
	_base_font_height (base_font_height)
{ }


inline std::optional<QRect>
ScreenSpec::position_and_size() const noexcept
{
	if (_position_and_size)
		return QRect { _position_and_size->topLeft(), _position_and_size->size() * _scale };
	else
		return std::nullopt;
}


inline std::optional<si::Length>
ScreenSpec::diagonal_length() const noexcept
{
	if (_diagonal_length)
		return *_diagonal_length / _scale;
	else
		return std::nullopt;
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
	return _base_font_height;
}


inline void
ScreenSpec::set_scale (float factor)
{
	_scale = factor;
}

} // namespace xf

#endif
