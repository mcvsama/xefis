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

#ifndef XEFIS__UTILITY__LOOKAHEAD_H__INCLUDED
#define XEFIS__UTILITY__LOOKAHEAD_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace Xefis {

template<class tType>
	class Lookahead
	{
	  public:
		typedef tType Type;

	  public:
		// Ctor:
		explicit Lookahead (Time lookahead_time) noexcept;

		/**
		 * Set lookahead time.
		 */
		void
		set_lookahead_time (Time) noexcept;

		/**
		 * Don't compute new result until given time
		 * has passed.
		 */
		void
		set_minimum_integration_time (Time) noexcept;

		/**
		 * Mark value as invalid and reset computations
		 * until next process() call.
		 */
		void
		invalidate() noexcept;

		/**
		 * Process new input sample, taken after dt time.
		 */
		Type
		process (Type input, Time dt) noexcept;

	  private:
		Time	_lookahead_time;
		Time	_time						= 0_s;
		Time	_minimum_integration_time	= 0_s;
		Type	_last_input;
		Type	_last_output;
		bool	_invalidate					= false;
	};


template<class T>
	inline
	Lookahead<T>::Lookahead (Time lookahead_time) noexcept:
		_lookahead_time (lookahead_time)
	{ }


template<class T>
	inline void
	Lookahead<T>::set_lookahead_time (Time lookahead_time) noexcept
	{
		_lookahead_time = lookahead_time;
		invalidate();
	}


template<class T>
	inline void
	Lookahead<T>::set_minimum_integration_time (Time time) noexcept
	{
		_minimum_integration_time = time;
	}


template<class T>
	inline void
	Lookahead<T>::invalidate() noexcept
	{
		_invalidate = true;
	}


template<class T>
	inline typename Lookahead<T>::Type
	Lookahead<T>::process (Type input, Time dt) noexcept
	{
		_time += dt;

		if (_invalidate)
		{
			_last_input = input;
			_last_output = input;
			_invalidate = false;
		}

		if (_time > _minimum_integration_time)
		{
			_last_output = (_lookahead_time * (input - _last_input)) / _time + input;
			_last_input = input;
			_time = 0_s;
		}

		return _last_output;
	}

} // namespace Xefis

#endif

