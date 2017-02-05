/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CONFIGS__CTHULHU__CTHULHU_H__INCLUDED
#define XEFIS__CONFIGS__CTHULHU__CTHULHU_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/machine.h>
#include <xefis/core/xefis.h>


class Cthulhu: public x2::Machine
{
  public:
	// Ctor
	Cthulhu (xf::Xefis*);

  private:
	void
	setup_ht16k33s();
};

#endif

