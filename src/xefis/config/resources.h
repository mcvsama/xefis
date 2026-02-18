/* vim:ts=4
 *
 * Copyleft 2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 * --
 * Here be basic, global functions and macros like asserts, debugging helpers, etc.
 */

#ifndef XEFIS__CONFIG__RESOURCES_H__INCLUDED
#define XEFIS__CONFIG__RESOURCES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

#ifndef XEFIS_PREFIX
#define XEFIS_PREFIX ""
#endif

#ifndef XEFIS_SHARED_DIRECTORY
#define XEFIS_SHARED_DIRECTORY XEFIS_PREFIX "share"
#endif

#ifndef XEFIS_XDG_SETTINGS_HOME
#define XEFIS_XDG_SETTINGS_HOME "mulabs.org/xefis"
#endif

#ifndef XEFIS_XDG_DATA_HOME
#define XEFIS_XDG_DATA_HOME "mulabs.org/xefis"
#endif

#endif
