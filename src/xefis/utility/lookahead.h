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
		explicit Lookahead (Time lookahead_time);

		void
		set_minimum_integration_time (Time);

		Type
		process (Type input, Time dt);

	  private:
		Time	_lookahead_time;
		Time	_time = 0_s;
		Time	_minimum_integration_time = 0_s;
		Type	_last_input;
		Type	_last_output;
	};


template<class T>
	Lookahead<T>::Lookahead (Time lookahead_time):
		_lookahead_time (lookahead_time)
	{ }


template<class T>
	void
	Lookahead<T>::set_minimum_integration_time (Time time)
	{
		_minimum_integration_time = time;
	}


template<class T>
	typename Lookahead<T>::Type
	Lookahead<T>::process (Type input, Time dt)
	{
		_time += dt;

		if (_time > _minimum_integration_time)
		{
			_last_output = _last_input + _lookahead_time / _time * (input - _last_input);
			_last_input = input;
			_time = 0_s;
		}

		return _last_output;
	}

} // namespace Xefis

#endif

