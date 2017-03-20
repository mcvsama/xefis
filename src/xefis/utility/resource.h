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

#ifndef XEFIS__UTILITY__RESOURCE_H__INCLUDED
#define XEFIS__UTILITY__RESOURCE_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

/**
 * Define an object which executes given function upon destruction.
 * Useful substitude for "finally" construct, nonexistent in C++.
 */
class Resource
{
  public:
	typedef std::function<void()> Callback;

  public:
	// Ctor
	Resource() noexcept = default;

	// Ctor
	explicit
	Resource (Callback callback) noexcept;

	// Copy ctor
	Resource (Resource const&) = delete;

	// Move ctor
	Resource (Resource&& other) noexcept;

	// Copy operator
	Resource&
	operator= (Resource const&) = delete;

	// Move operator
	Resource&
	operator= (Resource&& other) noexcept;

	/**
	 * Assign new resource. Release previous resource.
	 */
	Resource&
	operator= (Callback callback) noexcept;

	/**
	 * Releases resouce (calls the callback).
	 */
	~Resource();

	/**
	 * Destroy the tracked resource.
	 */
	void
	destroy();

	/**
	 * Release the resource so that it's not tracked anymore.
	 */
	void
	release();

  private:
	Callback _callback;
};


inline
Resource::Resource (Callback callback) noexcept:
	_callback (callback)
{ }


inline Resource&
Resource::operator= (Resource&& other) noexcept
{
	destroy();
	_callback = other._callback;
	other.release();
	return *this;
}


inline Resource&
Resource::operator= (Callback callback) noexcept
{
	destroy();
	_callback = callback;
	return *this;
}


inline
Resource::~Resource()
{
	destroy();
}


inline void
Resource::destroy()
{
	if (_callback)
		_callback();
	release();
}


inline void
Resource::release()
{
	_callback = nullptr;
}

} // namespace xf

#endif

