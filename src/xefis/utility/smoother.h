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


namespace xf {

/**
 * Contains all type-independend methods for the smoother.
 */
class SmootherBase
{
  public:
	// Dtor
	virtual ~SmootherBase() {}

	/**
	 * Return smoothing time.
	 */
	Time
	smoothing_time() const noexcept;

	/**
	 * Set new smoothing time.
	 * It's the size of the smoothing window. After that time, output value will reach target value.
	 */
	void
	set_smoothing_time (Time smoothing_time) noexcept;

	/**
	 * Return sampling precision.
	 */
	Time
	precision() const noexcept;

	/**
	 * Set sampling precision.
	 */
	void
	set_precision (Time precision) noexcept;

	/**
	 * Resets the smoother when the next process() is called,
	 * to the value given in process() call.
	 */
	void
	invalidate() noexcept;

  protected:
	/**
	 * Subclass implementation.
	 */
	virtual void
	set_smoothing_time_impl (int milliseconds) noexcept = 0;

  protected:
	Time	_smoothing_time;
	Time	_precision;
	bool	_invalidate = false;
};


/**
 * Implementation of moving averages.
 */
template<class tValueType>
	class Smoother: public SmootherBase
	{
	  public:
		typedef tValueType ValueType;

	  public:
		// Ctor
		Smoother (Time smoothing_time = 1_ms, Time precision = 1_ms) noexcept;

		/**
		 * Resets smoother to initial state (or given value).
		 */
		void
		reset (ValueType value = ValueType()) noexcept;

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

	  protected:
		void
		set_smoothing_time_impl (int milliseconds) noexcept override;

	  private:
		ValueType
		encircle (ValueType s) const noexcept;

		ValueType
		decircle (ValueType s) const noexcept;

		void
		recompute_window() noexcept;

	  private:
		Time								_accumulated_dt		= 0_s;
		ValueType							_z;
		Range<ValueType>					_winding;
		bool								_winding_enabled	= false;
		boost::circular_buffer<ValueType>	_history;
		boost::circular_buffer<ValueType>	_history_cos;
		boost::circular_buffer<ValueType>	_history_sin;
		std::vector<ValueType>				_window;
	};


inline Time
SmootherBase::smoothing_time() const noexcept
{
	return _smoothing_time;
}


inline void
SmootherBase::set_smoothing_time (Time smoothing_time) noexcept
{
	_smoothing_time = smoothing_time;
	int millis = _smoothing_time.quantity<Millisecond>();
	// Due to the nature of Hann window, minimum number of samples is 3,
	// therefore minimum smoothing time is 3 ms.
	if (millis < 3)
		millis = 3;
	set_smoothing_time_impl (millis);
}


inline Time
SmootherBase::precision() const noexcept
{
	return _precision;
}


inline void
SmootherBase::set_precision (Time precision) noexcept
{
	_precision = precision;
	invalidate();
}


inline void
SmootherBase::invalidate() noexcept
{
	_invalidate = true;
}


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
	Smoother<V>::set_smoothing_time_impl (int millis) noexcept
	{
		_history.resize (millis);
		_history_cos.resize (millis);
		_history_sin.resize (millis);
		_window.resize (millis);
		recompute_window();
		invalidate();
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
		_accumulated_dt += dt;

		if (!std::isfinite (s))
			return _z;

		if (_invalidate)
		{
			_invalidate = false;
			reset (s);
		}

		if (_accumulated_dt > 10 * _smoothing_time)
			_accumulated_dt = 10 * _smoothing_time;

		int iterations = _accumulated_dt / _precision;

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
					x += _history_cos[i] * _window[i];
					y += _history_sin[i] * _window[i];
				}
				x /= _history.size() - 1;
				y /= _history.size() - 1;
				x *= 2.0;
				y *= 2.0; // Window energy correction.
				_z = floored_mod<ValueType> (decircle (std::atan2 (y, x)), _winding.min(), _winding.max());
			}
			else
			{
				ValueType p = _history.back();
				// Linear interpolation:
				for (int i = 0; i < iterations; ++i)
					_history.push_back (p + (static_cast<ValueType> (i + 1) / iterations) * (s - p));

				_z = 0.0;
				for (std::size_t i = 0; i < _history.size(); ++i)
					_z += _history[i] * _window[i];
				_z /= _history.size() - 1; // Some coeffs are 0 in the window.
				_z *= 2.0; // Window energy correction.
			}

			_accumulated_dt = 0_s;
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


template<class V>
	inline void
	Smoother<V>::recompute_window() noexcept
	{
		std::size_t N = _window.size();
		for (std::size_t n = 0; n < N; ++n)
			_window[n] = 0.5 * (1.0 - std::cos (2.0 * M_PI * n / (N - 1)));
	}

} // namespace xf

#endif

