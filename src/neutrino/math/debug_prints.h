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

#ifndef NEUTRINO__MATH__DEBUG_PRINTS_H__INCLUDED
#define NEUTRINO__MATH__DEBUG_PRINTS_H__INCLUDED

// Standard:
#include <cstddef>
#include <ostream>

// Lib:
#include <math/math.h>


namespace neutrino::debug {

template<class Value, std::size_t Size, class SourceFrame, class TargetFrame>
	std::ostream&
	operator<< (std::ostream& os, math::Vector<Value, Size, SourceFrame, TargetFrame> const& vector)
	{
		auto const w = os.width();

		for (std::size_t i = 0; i < Size; ++i)
			os << std::setw (w) << vector[i] << (i != Size - 1 ? " " : "");

		return os;
	}


template<class Value, std::size_t Columns, std::size_t Rows, class SourceFrame, class TargetFrame>
	std::ostream&
	operator<< (std::ostream& os, math::Matrix<Value, Columns, Rows, SourceFrame, TargetFrame> const& matrix)
	{
		auto const w = os.width();

		for (std::size_t c = 0; c < Columns; ++c)
		{
			for (std::size_t r = 0; r < Rows; ++r)
				os << std::setw (w) << matrix (c, r) << (r < Rows - 1 ? " " : "");

			if (c < Columns - 1)
				os << ", ";
		}

		return os;
	}

} // namespace neutrino::debug

#endif

