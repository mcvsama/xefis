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

#ifndef NEUTRINO__RESPONSIBILITY_H__INCLUDED
#define NEUTRINO__RESPONSIBILITY_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>


namespace neutrino {

/**
 * Define an object which executes given function upon destruction.
 * Useful substitude for "finally" construct, nonexistent in C++.
 */
class Responsibility
{
  public:
	typedef std::function<void()> Callback;

  public:
	// Ctor
	Responsibility() noexcept = default;

	// Ctor
	explicit
	Responsibility (Callback callback) noexcept;

	// Copy ctor
	Responsibility (Responsibility const&) = delete;

	// Move ctor
	Responsibility (Responsibility&& other) noexcept;

	// Copy operator
	Responsibility&
	operator= (Responsibility const&) = delete;

	// Move operator
	Responsibility&
	operator= (Responsibility&& other) noexcept;

	/**
	 * Assign new responsibility. Release previous one.
	 */
	Responsibility&
	operator= (Callback callback) noexcept;

	/**
	 * Releases resouce (calls the callback).
	 */
	~Responsibility();

	/**
	 * Execute the tracked responsibility.
	 */
	void
	execute();

	/**
	 * Release the responsibility so that it's not tracked anymore.
	 */
	void
	release();

  private:
	Callback _callback;
};


inline
Responsibility::Responsibility (Callback callback) noexcept:
	_callback (callback)
{ }


inline Responsibility&
Responsibility::operator= (Responsibility&& other) noexcept
{
	execute();
	_callback = other._callback;
	other.release();
	return *this;
}


inline Responsibility&
Responsibility::operator= (Callback callback) noexcept
{
	execute();
	_callback = callback;
	return *this;
}


inline
Responsibility::~Responsibility()
{
	execute();
}


inline void
Responsibility::execute()
{
	if (_callback)
		_callback();
	release();
}


inline void
Responsibility::release()
{
	_callback = nullptr;
}

} // namespace neutrino

#endif

