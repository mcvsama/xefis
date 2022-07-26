/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__ELECTRICAL__EXCEPTION_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__ELECTRICAL__EXCEPTION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/exception.h>

// Standard:
#include <cstddef>


namespace xf::electrical {

/**
 * Thrown when simulation could not converge on solution after a certain limit of
 * iterations.
 */
class NotConverged: public Exception
{
  public:
	using Exception::Exception;
};

} // namespace xf::electrical

#endif

