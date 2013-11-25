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

// Boost:
#include <boost/circular_buffer.hpp>

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
		Smoother (Time smoothing_time = 1_ms, Time precision = 1_ms) noexcept;

		/**
		 * Set new smoothing time.
		 */
		void
		set_smoothing_time (Time smoothing_time) noexcept;

		/**
		 * Set sampling precision.
		 */
		void
		set_precision (Time precision) noexcept;

		/**
		 * Return smoothing time.
		 */
		Time
		smoothing_time() const noexcept;

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

		/**
		 * Return most recently pushed sample.
		 */
		ValueType
		last_sample() const noexcept;

	  private:
		ValueType
		encircle (ValueType s) const noexcept;

		ValueType
		decircle (ValueType s) const noexcept;

	  private:
		Time								_smoothing_time;
		Time								_precision;
		ValueType							_z;
		Range<ValueType>					_winding;
		bool								_winding_enabled	= false;
		bool								_invalidate			= false;
		boost::circular_buffer<ValueType>	_history;
		boost::circular_buffer<ValueType>	_history_cos;
		boost::circular_buffer<ValueType>	_history_sin;
	};


template<class V>
	inline
	Smoother<V>::Smoother (Time smoothing_time, Time precision) noexcept
	{
		set_smoothing_time (smoothing_time);
		set_precision (precision);
		invalidate();
	}


template<class V>
	inline void
	Smoother<V>::set_smoothing_time (Time smoothing_time) noexcept
	{
		_smoothing_time = smoothing_time;
		int millis = _smoothing_time.ms();
		if (millis < 1)
			millis = 1;
		_history.resize (millis);
		_history_cos.resize (millis);
		_history_sin.resize (millis);
		invalidate();
	}


template<class V>
	inline void
	Smoother<V>::set_precision (Time precision) noexcept
	{
		_precision = precision;
		invalidate();
	}


template<class V>
	inline Time
	Smoother<V>::smoothing_time() const noexcept
	{
		return _smoothing_time;
	}


template<class V>
	inline void
	Smoother<V>::reset (ValueType value) noexcept
	{
		std::fill (_history.begin(), _history.end(), value);
		if (_winding_enabled)
		{
			std::fill (_history_cos.begin(), _history_cos.end(), std::cos (encircle (value)));
			std::fill (_history_sin.begin(), _history_sin.end(), std::sin (encircle (value)));
			_z = floored_mod<ValueType> (_z, _winding.min(), _winding.max());
		}
		else
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
		reset (range.min());
	}


template<class V>
	inline typename Smoother<V>::ValueType
	Smoother<V>::process (ValueType s, Time dt) noexcept
	{
		if (!std::isfinite (s))
			return _z;

		if (_invalidate)
		{
			_invalidate = false;
			reset (s);
		}

		if (dt > 10 * _smoothing_time)
			dt = 10 * _smoothing_time;

		int iterations = std::round (dt / _precision);
		if (iterations > 1)
		{
			if (_winding_enabled)
			{
				ValueType p = _history.back();
				ValueType cos_p = _history_cos.back();
				ValueType sin_p = _history_sin.back();
				ValueType rad_s = encircle (s);
				ValueType cos_s = std::cos (rad_s);
				ValueType sin_s = std::sin (rad_s);

				for (int i = 0; i < iterations; ++i)
				{
					ValueType d = static_cast<ValueType> (i + 1) / iterations;
					_history.push_back (p + d * (s - p));
					_history_cos.push_back (cos_p + d * (cos_s - cos_p));
					_history_sin.push_back (sin_p + d * (sin_s - sin_p));
				}

				ValueType x = 0.0;
				ValueType y = 0.0;
				for (typename boost::circular_buffer<ValueType>::size_type i = 0; i < _history.size(); ++i)
				{
					x += _history_cos[i];
					y += _history_sin[i];
				}
				_z = floored_mod<ValueType> (decircle (std::atan2 (y, x)), _winding.min(), _winding.max());
			}
			else
			{
				ValueType p = _history.back();
				for (int i = 0; i < iterations; ++i)
					_history.push_back (p + (static_cast<ValueType> (i + 1) / iterations) * (s - p));

				_z = 0.0;
				for (ValueType v: _history)
					_z += v;
				_z /= _history.size();
			}
		}
		else
		{
			if (_winding_enabled)
				_z = floored_mod<ValueType> (s, _winding.min(), _winding.max());
			else
				_z = s;
		}

		return _z;
	}


template<class V>
	inline typename Smoother<V>::ValueType
	Smoother<V>::value() const noexcept
	{
		return _z;
	}


template<class V>
	inline typename Smoother<V>::ValueType
	Smoother<V>::last_sample() const noexcept
	{
		return _history.back();
	}


template<class V>
	inline typename Smoother<V>::ValueType
	Smoother<V>::encircle (ValueType s) const noexcept
	{
		return renormalize (s, _winding, Range<ValueType> (0.0, 2.0 * M_PI));
	}


template<class V>
	inline typename Smoother<V>::ValueType
	Smoother<V>::decircle (ValueType s) const noexcept
	{
		return renormalize (s, Range<ValueType> (0.0, 2.0 * M_PI), _winding);
	}

} // namespace Xefis

#endif

