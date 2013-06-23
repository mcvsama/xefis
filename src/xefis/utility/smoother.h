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

#ifndef XEFIS__UTILITY__SMOOTHER_H__INCLUDED
#define XEFIS__UTILITY__SMOOTHER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <algorithm>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/range.h>


namespace Xefis {

/**
 * Implementation of one-pole LPF.
 * See http://musicdsp.org/showone.php?id=257
 */
template<class tValueType>
	class Smoother
	{
	  public:
		typedef tValueType ValueType;
		typedef float SamplesType;

	  public:
		Smoother (Time smooth_time = 1_ms) noexcept;

		void
		set_smooth_time (Time smooth_time) noexcept;

		/**
		 * Resets smoother to initial state (or given value).
		 */
		void
		reset (ValueType value = ValueType()) noexcept;

		/**
		 * Resets the smoother when next process() is called,
		 * to the value given in process() call.
		 */
		void
		invalidate() noexcept;

		/**
		 * Enable winding and set valid values range.
		 */
		void
		set_winding (Range<ValueType> range);

		/**
		 * Return smoothed sample from given input sample
		 * and time from last update.
		 */
		ValueType
		process (ValueType s, Time dt) noexcept;

		/**
		 * Return last processed value.
		 */
		ValueType
		value() const noexcept;

	  private:
		ValueType
		process_single_sample (ValueType s) noexcept;

	  private:
		SamplesType				_time;// DEPREACTED TODO
		ValueType				_z;
		Range<ValueType>		_winding;
		bool					_winding_enabled	= false;
		bool					_invalidate			= false;
		std::vector<ValueType>	_history;
	};


template<class V>
	inline
	Smoother<V>::Smoother (Time smooth_time) noexcept
	{
		set_smooth_time (smooth_time);
		invalidate();
	}


template<class V>
	inline void
	Smoother<V>::set_smooth_time (Time smooth_time) noexcept
	{
		int millis = smooth_time.ms();
		if (millis < 1)
			millis = 1;
		_time = std::pow (0.01f, 2.0f / millis);
		_history.resize (millis);
		invalidate();
	}


template<class V>
	inline void
	Smoother<V>::reset (ValueType value) noexcept
	{
		std::fill (_history.begin(), _history.end(), value);
		_z = value;
	}


template<class V>
	inline void
	Smoother<V>::invalidate() noexcept
	{
		_invalidate = true;
	}


template<class V>
	inline void
	Smoother<V>::set_winding (Range<ValueType> range)
	{
		_winding = range;
		_winding_enabled = true;
	}


template<class V>
	inline typename Smoother<V>::ValueType
	Smoother<V>::process (ValueType s, Time dt) noexcept
	{
		if (_invalidate)
		{
			_invalidate = false;
			reset (s);
		}

		int iterations = dt.ms();
		ValueType z = _z;
		if (iterations > 1)
			for (int i = 0; i < iterations; ++i)
				process_single_sample (z + (static_cast<ValueType> (i + 1) / iterations) * (s - z));
		return _z;
	}


template<class V>
	inline typename Smoother<V>::ValueType
	Smoother<V>::process_single_sample (ValueType s) noexcept
	{
		// Protect from NaNs and infs:
		if (!std::isfinite (_z))
			reset (s);

		if (_winding_enabled)
		{
			ValueType c = (_winding.min() + _winding.max()) / 2.0;
			ValueType h = _winding.extent() / 2.0;
			if (s < _z - c)
				s = s + _winding.extent();
			else if (s > _z + c)
				s = s - _winding.extent();
			return _z = floored_mod (_time * (_z - s) + s, _winding.min(), _winding.max());
		}
		else
		{
			std::rotate (_history.begin(), _history.begin() + _history.size() - 1, _history.end());
			_history[0] = s;

			_z = 0.0;
			for (ValueType const& v: _history)
				_z += v;
			_z /= _history.size();

			return _z;
		}
	}


template<class V>
	inline typename Smoother<V>::ValueType
	Smoother<V>::value() const noexcept
	{
		return _z;
	}

} // namespace Xefis

#endif

