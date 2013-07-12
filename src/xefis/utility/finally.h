/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__FINALLY_H__INCLUDED
#define XEFIS__UTILITY__FINALLY_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

/**
 * Define an object which executes given function upon destruction.
 * Useful substitude for "finally" construct, nonexistent in C++.
 */
class Finally
{
  public:
	typedef std::function<void()> Callback;

  public:
	Finally (Callback callback) noexcept;

	~Finally();

  private:
	Callback _callback;
};


inline
Finally::Finally (Callback callback) noexcept:
	_callback (callback)
{ }


inline
Finally::~Finally()
{
	if (_callback)
		_callback();
}

} // namespace Xefis

#endif

