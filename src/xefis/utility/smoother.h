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
	virtual
	~SmootherBase() = default;

	/**
	 * Return smoothing time.
	 */
	si::Time
	smoothing_time() const noexcept;

	/**
	 * Set new smoothing time.
	 * It's the size of the smoothing window. After that time, output value will reach target value.
	 */
	void
	set_smoothing_time (si::Time smoothing_time) noexcept;

	/**
	 * Return sampling precision.
	 */
	si::Time
	precision() const noexcept;

	/**
	 * Set sampling precision.
	 */
	void
	set_precision (si::Time precision) noexcept;

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
	si::Time	_smoothing_time;
	si::Time	_precision;
	bool		_invalidate = false;
};


/**
 * Implementation of moving averages.
 */
template<class pValue>
	class Smoother: public SmootherBase
	{
	  public:
		typedef pValue Value;

	  public:
		// Ctor
		explicit
		Smoother (si::Time smoothing_time = 1_ms, si::Time precision = 1_ms) noexcept;

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
		void
		recompute_window() noexcept;

	  private:
		si::Time						_accumulated_dt		= 0_s;
		Value							_z;
		boost::circular_buffer<Value>	_history;
		std::vector<double>				_window;
	};


inline si::Time
SmootherBase::smoothing_time() const noexcept
{
	return _smoothing_time;
}


inline void
SmootherBase::set_smoothing_time (si::Time smoothing_time) noexcept
{
	_smoothing_time = smoothing_time;
	int millis = _smoothing_time.quantity<Millisecond>();
	// Due to the nature of the Hann window, minimum number of samples is 3,
	// therefore the minimum smoothing time is 3 ms.
	if (millis < 3)
		millis = 3;
	set_smoothing_time_impl (millis);
}


inline si::Time
SmootherBase::precision() const noexcept
{
	return _precision;
}


inline void
SmootherBase::set_precision (si::Time precision) noexcept
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
	Smoother<V>::Smoother (si::Time smoothing_time, si::Time precision) noexcept
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
		_window.resize (millis);
		recompute_window();
		invalidate();
	}


template<class V>
	inline void
	Smoother<V>::reset (Value value) noexcept
	{
		std::fill (_history.begin(), _history.end(), value);
		_z = value;
	}


template<class V>
	inline typename Smoother<V>::Value
	Smoother<V>::process (Value s, si::Time dt) noexcept
	{
		using si::isfinite;

		_accumulated_dt += dt;

		if (!isfinite (s))
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
			Value p = _history.back();
			// Linear interpolation:
			for (int i = 0; i < iterations; ++i)
				_history.push_back (p + (static_cast<double> (i + 1) / iterations) * (s - p));

			_z = Value (0.0);
			for (std::size_t i = 0; i < _history.size(); ++i)
				_z += _history[i] * _window[i];
			_z /= _history.size() - 1; // Some coeffs are 0 in the window.
			_z *= 2.0; // Window energy correction.

			_accumulated_dt = 0_s;
		}

		return _z;
	}


template<class V>
	inline typename Smoother<V>::Value
	Smoother<V>::operator() (Value s, si::Time dt) noexcept
	{
		return process (s, dt);
	}


template<class V>
	inline typename Smoother<V>::Value
	Smoother<V>::value() const noexcept
	{
		return _z;
	}


template<class V>
	inline typename Smoother<V>::Value
	Smoother<V>::last_sample() const noexcept
	{
		return _history.back();
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

