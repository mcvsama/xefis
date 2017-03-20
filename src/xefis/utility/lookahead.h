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


namespace xf {

template<class pValue>
	class Lookahead
	{
	  public:
		typedef pValue Value;

	  public:
		// Ctor
		explicit
		Lookahead (si::Time lookahead_time) noexcept;

		/**
		 * Set lookahead time.
		 */
		void
		set_lookahead_time (si::Time) noexcept;

		/**
		 * Don't compute new result until given time
		 * has passed.
		 */
		void
		set_minimum_integration_time (si::Time) noexcept;

		/**
		 * Mark value as invalid and reset computations
		 * until next process() call.
		 */
		void
		invalidate() noexcept;

		/**
		 * Process new input sample, taken after dt time.
		 */
		Value
		process (Value input, si::Time dt) noexcept;

		/**
		 * Alias for process().
		 */
		Value
		operator() (Value input, si::Time dt) noexcept;

	  private:
		si::Time	_lookahead_time;
		si::Time	_time						= 0_s;
		si::Time	_minimum_integration_time	= 0_s;
		Value		_last_input;
		Value		_last_output;
		bool		_invalidate					= false;
	};


template<class V>
	inline
	Lookahead<V>::Lookahead (si::Time lookahead_time) noexcept:
		_lookahead_time (lookahead_time)
	{ }


template<class V>
	inline void
	Lookahead<V>::set_lookahead_time (si::Time lookahead_time) noexcept
	{
		_lookahead_time = lookahead_time;
		invalidate();
	}


template<class V>
	inline void
	Lookahead<V>::set_minimum_integration_time (si::Time time) noexcept
	{
		_minimum_integration_time = time;
	}


template<class V>
	inline void
	Lookahead<V>::invalidate() noexcept
	{
		_invalidate = true;
	}


template<class V>
	inline typename Lookahead<V>::Value
	Lookahead<V>::process (Value input, si::Time dt) noexcept
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


template<class V>
	inline typename Lookahead<V>::Value
	Lookahead<V>::operator() (Value input, si::Time dt) noexcept
	{
		return process (input, dt);
	}

} // namespace xf

#endif

