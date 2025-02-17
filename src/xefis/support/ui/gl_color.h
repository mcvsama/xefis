/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__UI__GL_COLOR_H__INCLUDED
#define XEFIS__SUPPORT__UI__GL_COLOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QColor>

// Standard:
#include <cstddef>


namespace xf {

class GLColor
{
  public:
	// Ctor
	constexpr
	GLColor():
		_color_array ({ 0.0f, 0.0f, 0.0f, 0.0f })
	{ }

	// Ctor
	constexpr
	GLColor (float r, float g, float b, float alpha = 1.0):
		_color_array ({ r, g, b, alpha })
	{ }

	[[nodiscard]]
	static constexpr GLColor
	from_rgb (uint8_t r, uint8_t g, uint8_t b, uint8_t alpha = 0xff)
		{ return GLColor (r, g, b, alpha).scaled (1.0f / 255.0f); }

	// Copy ctor
	constexpr
	GLColor (GLColor const&) = default;

	// Move ctor
	constexpr
	GLColor (GLColor&&) = default;

	// Copy operator
	constexpr GLColor&
	operator= (GLColor const&) = default;

	// Move operator
	constexpr GLColor&
	operator= (GLColor&&) = default;

	[[nodiscard]]
	constexpr GLColor
	scaled (float light_scale);

	/**
	 * Factor is in range [0, 1].
	 */
	[[nodiscard]]
	constexpr GLColor
	darker (float factor);

	/**
	 * Factor is in range [0, 1].
	 */
	[[nodiscard]]
	constexpr GLColor
	lighter (float factor);

	[[nodiscard]]
	constexpr float const*
	data() const noexcept
		{ return _color_array.data(); }

	[[nodiscard]]
	constexpr
	operator float const*() const noexcept
		{ return data(); }

	[[nodiscard]]
	constexpr float&
	operator[] (std::size_t index) noexcept
		{ return _color_array[index]; }

	[[nodiscard]]
	constexpr float
	operator[] (std::size_t index) const noexcept
		{ return _color_array[index]; }

  private:
	std::array<float, 4> _color_array;
};


constexpr GLColor
GLColor::scaled (float light_scale)
{
	GLColor copy = *this;

	for (auto& v: copy._color_array)
		v *= light_scale;

	return copy;
}


constexpr GLColor
GLColor::darker (float darker_factor)
{
	return scaled (1.0 - darker_factor);
}


constexpr GLColor
GLColor::lighter (float lighter_factor)
{
	return scaled (1.0 + lighter_factor);
}


[[nodiscard]]
constexpr GLColor
to_gl_color (QColor const& color)
{
	constexpr auto f = 1.0f / 255.0f;

	return {
		f * color.red(),
		f * color.green(),
		f * color.blue(),
		f * color.alpha(),
	};
}


[[nodiscard]]
constexpr GLColor
to_gl_color (math::Vector<float, 3> const& rgb_color)
{
	return { rgb_color[0], rgb_color[1], rgb_color[2], 1.0f };
}


[[nodiscard]]
constexpr GLColor
to_gl_color (math::Vector<float, 4> const& rgb_color)
{
	return { rgb_color[0], rgb_color[1], rgb_color[2], rgb_color[3] };
}

} // namespace xf

#endif

