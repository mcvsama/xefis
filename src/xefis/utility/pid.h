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

#ifndef XEFIS__UTILITY__PID_H__INCLUDED
#define XEFIS__UTILITY__PID_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/range.h>


namespace Xefis {

/**
 * Proportional-Integral-Derivative controller.
 * TODO safety functions: limit derivative or something so it's not 0/nan/inf and the result is limited to certain range.
 * TODO protect from infs and nans
 * TODO add output limiter
 */
template<class tValueType>
	class PID
	{
		typedef tValueType ValueType;
		typedef double ParamType;
		typedef double TimeType;

	  public:
		PID (ParamType p, ParamType i, ParamType d, ValueType target);

		/**
		 * Set winding. That is value -1.0 is equal to 1.0.
		 */
		void
		set_winding (bool winding);

		/**
		 * Get P parameter.
		 */
		ParamType
		p() const noexcept;

		/**
		 * Set P parameter.
		 */
		void
		set_p (ParamType p) noexcept;

		/**
		 * Get I parameter.
		 */
		ParamType
		i() const noexcept;

		/**
		 * Set I parameter.
		 */
		void
		set_i (ParamType i) noexcept;

		/**
		 * Get D parameter.
		 */
		ParamType
		d() const noexcept;

		/**
		 * Set D parameter.
		 */
		void
		set_d (ParamType d) noexcept;

		/**
		 * Set P, I and D at once.
		 */
		void
		set_pid (ParamType p, ParamType i, ParamType d) noexcept;

		/**
		 * Return overall gain of all three parameters.
		 */
		ParamType
		gain() const noexcept;

		/**
		 * Set overall gain for all three paramters.
		 */
		void
		set_gain (ParamType gain) noexcept;

		/**
		 * Get power of proportional error.
		 */
		ParamType
		error_power() const noexcept;

		/**
		 * Set power of proportional error.
		 */
		void
		set_error_power (ParamType) noexcept;

		/**
		 * I (integral) parameter limit.
		 */
		Range<ValueType>
		i_limit() const noexcept;

		/**
		 * Set I (integral) parameter limit.
		 */
		void
		set_i_limit (Range<ValueType> limit) noexcept;

		/**
		 * Output limit.
		 */
		Range<ValueType>
		output_limit() const noexcept;

		/**
		 * Set output limit.
		 */
		void
		set_output_limit (Range<ValueType> limit) noexcept;

		/**
		 * Set target value. Should be normalized to [-1..1].
		 */
		void
		set_target (ValueType target) noexcept;

		/**
		 * Process value for given dt (timespan) and return new value.
		 * Input value should be normalized to [-1..1].
		 */
		ValueType
		process (ValueType input, TimeType dt) noexcept;

		/**
		 * Return current controller output value.
		 */
		ValueType
		output() const noexcept;

	  private:
		bool				_winding		= false;
		ValueType			_target			= ValueType();
		ValueType			_output			= ValueType();
		ValueType			_previous_error	= ValueType();
		ValueType			_integral		= ValueType();
		ValueType			_derivative		= ValueType();
		ParamType			_p				= 0.0;
		ParamType			_i				= 0.0;
		Range<ValueType>	_i_limit		= { -1.0, +1.0 };
		ParamType			_d				= 0.0;
		ParamType			_gain			= 1.0;
		ParamType			_error_power	= 1.0;
		Range<ValueType>	_output_limit	= { -std::numeric_limits<ValueType>::max(), std::numeric_limits<ValueType>::max() };
	};


template<class T>
	inline
	PID<T>::PID (ParamType p, ParamType i, ParamType d, ValueType target):
		_target (target),
		_p (p),
		_i (i),
		_d (d)
	{ }


template<class T>
	inline void
	PID<T>::set_winding (bool winding)
	{
		_winding = winding;
	}


template<class T>
	inline typename PID<T>::ParamType
	PID<T>::p() const noexcept
	{
		return _p;
	}


template<class T>
	inline void
	PID<T>::set_p (ParamType p) noexcept
	{
		_p = p;
	}


template<class T>
	inline typename PID<T>::ParamType
	PID<T>::i() const noexcept
	{
		return _i;
	}


template<class T>
	inline void
	PID<T>::set_i (ParamType i) noexcept
	{
		_i = i;
	}


template<class T>
	inline typename PID<T>::ParamType
	PID<T>::d() const noexcept
	{
		return _d;
	}


template<class T>
	inline void
	PID<T>::set_d (ParamType d) noexcept
	{
		_d = d;
	}


template<class T>
	inline void
	PID<T>::set_pid (ParamType p, ParamType i, ParamType d) noexcept
	{
		_p = p;
		_i = i;
		_d = d;
	}


template<class T>
	inline typename PID<T>::ParamType
	PID<T>::gain() const noexcept
	{
		return _gain;
	}


template<class T>
	inline void
	PID<T>::set_gain (ParamType gain) noexcept
	{
		_gain = gain;
	}


template<class T>
	inline typename PID<T>::ParamType
	PID<T>::error_power() const noexcept
	{
		return _error_power;
	}


template<class T>
	inline void
	PID<T>::set_error_power (ParamType power) noexcept
	{
		_error_power = power;
	}


template<class T>
	inline Range<typename PID<T>::ValueType>
	PID<T>::i_limit() const noexcept
	{
		return _i_limit;
	}


template<class T>
	inline void
	PID<T>::set_i_limit (Range<ValueType> limit) noexcept
	{
		_i_limit = limit;
	}


template<class T>
	inline Range<typename PID<T>::ValueType>
	PID<T>::output_limit() const noexcept
	{
		return _output_limit;
	}


template<class T>
	inline void
	PID<T>::set_output_limit (Range<ValueType> limit) noexcept
	{
		_output_limit = limit;
	}


template<class T>
	inline void
	PID<T>::set_target (ValueType target) noexcept
	{
		_target = target;
	}


template<class T>
	inline typename PID<T>::ValueType
	PID<T>::process (ValueType measured_value, TimeType dt) noexcept
	{
		ValueType error = bound<ValueType> (_target - measured_value, -1.0, +1.0);
		_integral += error * dt;
		_integral = bound<ParamType> (_integral, _i_limit);
		_derivative = (error - _previous_error) / dt;
		// TODO Ensure that _derivative is finite:
//		_output = bound<double> (_gain * (_p * sgn (error) * std::pow<ValueType> (std::abs (error), _error_power) + _i * _integral + _d * _derivative), _output_limit);
		_output = bound<double> (_gain * (_p * error + _i * _integral + _d * _derivative), _output_limit);
		_previous_error = error;
		return _output;
	}


template<class T>
	inline typename PID<T>::ValueType
	PID<T>::output() const noexcept
	{
		return _output;
	}

} // namespace Xefis

#endif

