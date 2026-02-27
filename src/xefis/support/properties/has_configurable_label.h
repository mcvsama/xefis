/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__PROPERTIES__HAS_CONFIGURABLE_LABEL_H__INCLUDED
#define XEFIS__SUPPORT__PROPERTIES__HAS_CONFIGURABLE_LABEL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>
#include <string>


namespace xf {

class HasConfigurableLabel
{
  public:
	[[nodiscard]]
	std::string const&
	label() const noexcept
		{ return _label; }

	void
	set_label (std::string const& label)
		{ _label = label; }

	void
	set_label (std::string_view const label)
		{ _label = label; }

	void
	set_label (char const* label)
		{ _label = label; }

  private:
	std::string _label;
};

} // namespace xf

#endif
