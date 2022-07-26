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

#ifndef XEFIS__UTILITY__NAMED_INSTANCE_H__INCLUDED
#define XEFIS__UTILITY__NAMED_INSTANCE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>
#include <string_view>


namespace xf {

class NamedInstance
{
  public:
	// Ctor
	explicit
	NamedInstance (std::string_view const& instance):
		_instance (instance)
	{ }

	/**
	 * Return module instance name.
	 */
	[[nodiscard]]
	std::string const&
	instance() const noexcept;

  private:
	std::string _instance;
};


inline std::string const&
NamedInstance::instance() const noexcept
{
	return _instance;
}

} // namespace xf

#endif

