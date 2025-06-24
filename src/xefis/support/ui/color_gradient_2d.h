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

#ifndef XEFIS__SUPPORT__UI__COLOR_GRADIENT_2D_H__INCLUDED
#define XEFIS__SUPPORT__UI__COLOR_GRADIENT_2D_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/math/field.h>

// Standard:
#include <cstddef>


namespace xf {

template<class pArgumentX, class pArgumentY>
	class ColorGradient2D
	{
	  public:
		using ArgumentX	= pArgumentX;
		using ArgumentY	= pArgumentY;
		using Color		= math::Vector<float, 3>;
		using Field		= nu::Field<ArgumentX, ArgumentY, Color>;
		using DataMap	= Field::DataMap;

	  public:
		// Ctor
		explicit
		ColorGradient2D (DataMap const& data_map);

		// Ctor
		explicit
		ColorGradient2D (DataMap&& data_map);

		// Copy ctor
		explicit
		ColorGradient2D (ColorGradient2D const& other) = default;

		// Move ctor
		explicit
		ColorGradient2D (ColorGradient2D&& other) = default;

		// Copy operator
		ColorGradient2D&
		operator= (ColorGradient2D const&) = default;

		// Copy operator
		ColorGradient2D&
		operator= (ColorGradient2D&&) = default;

		// Return extrapolated color
		Color
		operator[] (ArgumentX const& x, ArgumentY const& y) const
			{ return _field (x, y); }
		// FIXME error:
		// in Field: return renormalize (Color), Range<Color>, Range<Color>);

	  private:
		Field _field;
	};


template<class X, class Y>
	inline
	ColorGradient2D<X, Y>::ColorGradient2D (DataMap const& data_map):
		_field (data_map)
	{ }


template<class X, class Y>
	inline
	ColorGradient2D<X, Y>::ColorGradient2D (DataMap&& data_map):
		_field (std::move (data_map))
	{ }

} // namespace xf

#endif

