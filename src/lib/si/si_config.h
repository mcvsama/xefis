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

#ifndef XEFIS__LIB__SI__SI_CONFIG_H__INCLUDED
#define XEFIS__LIB__SI__SI_CONFIG_H__INCLUDED

// Xefis:
#include <xefis/config/exception.h>


namespace xf {

class Exception;

} // namespace xf


namespace si_config {

using Exception = xf::Exception;

} // namespace si_config

#endif

