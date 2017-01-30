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

#ifndef XEFIS__UTILITY__OWNER_TOKEN_H__INCLUDED
#define XEFIS__UTILITY__OWNER_TOKEN_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/noncopyable.h>


namespace xf {

/**
 * Utility class for objects that can be moved, but not copied and want to have the default move constructor
 * and non-trivial destructor.
 *
 * An object that can be moved but not copied. Initially the object has 'owner' token, but when it's moved
 * the token is moved to the new object. You can test if object has the 'owner' token with the operator bool().
 *
 * Usage:
 *
 * class X
 * {
 *     X (X&&) = default;
 *
 *     ~X()
 *     {
 *         if (_owned)
 *             cleanup();
 *     }
 *
 *   private:
 *     OwnerToken _owned;
 * }
 */
class OwnerToken: public Noncopyable
{
  public:
	constexpr
	OwnerToken() = default;

	constexpr
	OwnerToken (OwnerToken&&);

	/**
	 * Return true if this object has the 'owner' token.
	 */
	constexpr
	operator bool() const noexcept;

  private:
	bool _has_token = true;
};


constexpr
OwnerToken::OwnerToken (OwnerToken&& other):
	_has_token (other._has_token)
{
	other._has_token = false;
}


constexpr
OwnerToken::operator bool() const noexcept
{
	return _has_token;
}

} // namespace xf

#endif

