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

#ifndef XEFIS__UTILITY__RANGE_SMOOTHER_H__INCLUDED
#define XEFIS__UTILITY__RANGE_SMOOTHER_H__INCLUDED

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
#include <xefis/utility/smoother.h>


namespace xf {

/**
 * Implementation of moving averages.
 */
template<class pValue>
	class RangeSmoother: public SmootherBase
	{
	  public:
		typedef pValue Value;

	  public:
		// Ctor
		explicit
		RangeSmoother (Range<Value> range, si::Time smoothing_time = 1_ms, si::Time precision = 1_ms) noexcept;

		/**
		 * Resets smoother to initial state (or given value).
		 */
		void
		reset (Value value = Value()) noexcept;

		/**
		 * Return smoothed sample from given input sample
		 * and time from last update.
		 */
		Value
		process (Value s, si::Time dt) noexcept;

		/**
		 * Alias for process().
		 */
		Value
		operator() (Value s, si::Time dt) noexcept;

		/**
		 * Return last processed value.
		 */
		Value
		value() const noexcept;

		/**
		 * Return most recently pushed sample.
		 */
		Value
		last_sample() const noexcept;

	  protected:
		void
		set_smoothing_time_impl (int milliseconds) noexcept override;

	  private:
		double
		encircle (Value s) const noexcept;

		Value
		decircle (double s) const noexcept;

		void
		recompute_window() noexcept;

	  private:
		si::Time						_accumulated_dt		= 0_s;
		Value							_z;
		Range<Value>					_range;
		boost::circular_buffer<Value>	_history;
		boost::circular_buffer<double>	_history_cos;
		boost::circular_buffer<double>	_history_sin;
		std::vector<double>				_window;
	};


template<class V>
	inline
	RangeSmoother<V>::RangeSmoother (Range<Value> range, si::Time smoothing_time, si::Time precision) noexcept:
		_range (range)
	{
		set_smoothing_time (smoothing_time);
		set_precision (precision);
		invalidate();
	}


template<class V>
	inline void
	RangeSmoother<V>::set_smoothing_time_impl (int millis) noexcept
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
	RangeSmoother<V>::reset (Value value) noexcept
	{
		std::fill (_history.begin(), _history.end(), value);
		std::fill (_history_cos.begin(), _history_cos.end(), std::cos (encircle (value)));
		std::fill (_history_sin.begin(), _history_sin.end(), std::sin (encircle (value)));
		_z = floored_mod<Value> (_z, _range);
	}


template<class V>
	inline typename RangeSmoother<V>::Value
	RangeSmoother<V>::process (Value s, si::Time dt) noexcept
	{
		_accumulated_dt += dt;

		if (!si::isfinite (s))
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
			auto p = _history.back();
			auto cos_p = _history_cos.back();
			auto sin_p = _history_sin.back();
			auto rad_s = encircle (s);
			auto cos_s = std::cos (rad_s);
			auto sin_s = std::sin (rad_s);

			for (int i = 0; i < iterations; ++i)
			{
				double d = static_cast<double> (i + 1) / iterations;
				_history.push_back (p + d * (s - p));
				_history_cos.push_back (cos_p + d * (cos_s - cos_p));
				_history_sin.push_back (sin_p + d * (sin_s - sin_p));
			}

			double x = 0.0;
			double y = 0.0;

			for (typename boost::circular_buffer<Value>::size_type i = 0; i < _history.size(); ++i)
			{
				x += _history_cos[i] * _window[i];
				y += _history_sin[i] * _window[i];
			}

			x /= _history.size() - 1;
			y /= _history.size() - 1;
			x *= 2.0;
			y *= 2.0; // Window energy correction.
			_z = floored_mod<Value> (decircle (std::atan2 (y, x)), _range);

			_accumulated_dt = 0_s;
		}

		return _z;
	}


template<class V>
	inline typename RangeSmoother<V>::Value
	RangeSmoother<V>::operator() (Value s, si::Time dt) noexcept
	{
		return process (s, dt);
	}


template<class V>
	inline typename RangeSmoother<V>::Value
	RangeSmoother<V>::value() const noexcept
	{
		return _z;
	}


template<class V>
	inline typename RangeSmoother<V>::Value
	RangeSmoother<V>::last_sample() const noexcept
	{
		return _history.back();
	}


template<class V>
	inline double
	RangeSmoother<V>::encircle (Value s) const noexcept
	{
		return renormalize (s, _range, Range<double> (0.0, 2.0 * M_PI));
	}


template<class V>
	inline typename RangeSmoother<V>::Value
	RangeSmoother<V>::decircle (double s) const noexcept
	{
		return renormalize (s, Range<double> (0.0, 2.0 * M_PI), _range);
	}


template<class V>
	inline void
	RangeSmoother<V>::recompute_window() noexcept
	{
		std::size_t N = _window.size();
		for (std::size_t n = 0; n < N; ++n)
			_window[n] = 0.5 * (1.0 - std::cos (2.0 * M_PI * n / (N - 1)));
	}

} // namespace xf

#endif

