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

#ifndef XEFIS__UTILITY__PID_CONTROL_H__INCLUDED
#define XEFIS__UTILITY__PID_CONTROL_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/range.h>
#include <xefis/utility/smoother.h>


namespace xf {

/**
 * Proportional-Integral-Derivative controller.
 * TODO safety functions: limit derivative or something so it's not 0/nan/inf and the result is limited to certain range.
 * TODO protect from infs and nans
 */
template<class tValueType>
	class PIDControl
	{
	  public:
		typedef tValueType ValueType;
		typedef double ParamType;

		class Settings
		{
		  public:
			ValueType	p;
			ValueType	i;
			ValueType	d;
		};

	  public:
		// Ctor
		PIDControl (Settings const& settings, ValueType target);

		// Ctor
		PIDControl (ParamType p, ParamType i, ParamType d, ValueType target);

		/**
		 * Enable/disable output smoothing.
		 * When enabling, smoothing_time is used to configure internal output smoother.
		 */
		void
		set_output_smoothing (bool enable, Time smoothing_time = 0_s);

		/**
		 * Set winding. That is value -1.0 is equal to 1.0.
		 * When using winding, the measured value is expected to
		 * be winded up, too.
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
		set_pid (Settings const& settings) noexcept;

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
		 * Set target value. Set target value. If winding is enabled,
		 * then the target should be normalized to [-1..1].
		 */
		void
		set_target (ValueType target) noexcept;

		/**
		 * Process value for given dt (timespan) and return new value.
		 * Input value should be normalized to [-1..1].
		 */
		ValueType
		process (ValueType input, Time dt) noexcept;

		/**
		 * Return current controller output value.
		 */
		ValueType
		output() const noexcept;

		/**
		 * Return error value.
		 */
		ValueType
		error() const noexcept;

		/**
		 * Reset to default state.
		 */
		void
		reset() noexcept;

		/**
		 * Return internal output smoother.
		 */
		Smoother<ValueType>&
		output_smoother() noexcept;

		/**
		 * Return internal output smoother.
		 */
		Smoother<ValueType> const&
		output_smoother() const noexcept;

	  private:
		bool				_winding					= false;
		bool				_output_smoothing_enabled	= false;
		Smoother<ValueType>	_output_smoother			= Time (1_ms);
		ValueType			_target						= ValueType();
		ValueType			_output						= ValueType();
		ValueType			_previous_error				= ValueType();
		ValueType			_integral					= ValueType();
		ValueType			_derivative					= ValueType();
		ParamType			_p							= 0.0;
		ParamType			_i							= 0.0;
		Range<ValueType>	_i_limit					= { -1.0, +1.0 };
		ParamType			_d							= 0.0;
		ParamType			_gain						= 1.0;
		ParamType			_error_power				= 1.0;
		Range<ValueType>	_output_limit				= { -std::numeric_limits<ValueType>::max(), std::numeric_limits<ValueType>::max() };
	};


template<class T>
	inline
	PIDControl<T>::PIDControl (Settings const& settings, ValueType target):
		_target (target),
		_p (settings.p),
		_i (settings.i),
		_d (settings.d)
	{
		reset();
	}


template<class T>
	inline
	PIDControl<T>::PIDControl (ParamType p, ParamType i, ParamType d, ValueType target):
		_target (target),
		_p (p),
		_i (i),
		_d (d)
	{
		reset();
	}


template<class T>
	inline void
	PIDControl<T>::set_output_smoothing (bool enable, Time smoothing_time)
	{
		_output_smoothing_enabled = enable;
		if (enable)
			_output_smoother.set_smoothing_time (smoothing_time);
	}


template<class T>
	inline void
	PIDControl<T>::set_winding (bool winding)
	{
		_winding = winding;
	}


template<class T>
	inline typename PIDControl<T>::ParamType
	PIDControl<T>::p() const noexcept
	{
		return _p;
	}


template<class T>
	inline void
	PIDControl<T>::set_p (ParamType p) noexcept
	{
		_p = p;
	}


template<class T>
	inline typename PIDControl<T>::ParamType
	PIDControl<T>::i() const noexcept
	{
		return _i;
	}


template<class T>
	inline void
	PIDControl<T>::set_i (ParamType i) noexcept
	{
		_i = i;
	}


template<class T>
	inline typename PIDControl<T>::ParamType
	PIDControl<T>::d() const noexcept
	{
		return _d;
	}


template<class T>
	inline void
	PIDControl<T>::set_d (ParamType d) noexcept
	{
		_d = d;
	}


template<class T>
	inline void
	PIDControl<T>::set_pid (Settings const& settings) noexcept
	{
		_p = settings.p;
		_i = settings.i;
		_d = settings.d;
	}


template<class T>
	inline void
	PIDControl<T>::set_pid (ParamType p, ParamType i, ParamType d) noexcept
	{
		_p = p;
		_i = i;
		_d = d;
	}


template<class T>
	inline typename PIDControl<T>::ParamType
	PIDControl<T>::gain() const noexcept
	{
		return _gain;
	}


template<class T>
	inline void
	PIDControl<T>::set_gain (ParamType gain) noexcept
	{
		_gain = gain;
	}


template<class T>
	inline typename PIDControl<T>::ParamType
	PIDControl<T>::error_power() const noexcept
	{
		return _error_power;
	}


template<class T>
	inline void
	PIDControl<T>::set_error_power (ParamType power) noexcept
	{
		_error_power = power;
	}


template<class T>
	inline Range<typename PIDControl<T>::ValueType>
	PIDControl<T>::i_limit() const noexcept
	{
		return _i_limit;
	}


template<class T>
	inline void
	PIDControl<T>::set_i_limit (Range<ValueType> limit) noexcept
	{
		_i_limit = limit;
	}


template<class T>
	inline Range<typename PIDControl<T>::ValueType>
	PIDControl<T>::output_limit() const noexcept
	{
		return _output_limit;
	}


template<class T>
	inline void
	PIDControl<T>::set_output_limit (Range<ValueType> limit) noexcept
	{
		_output_limit = limit;
	}


template<class T>
	inline void
	PIDControl<T>::set_target (ValueType target) noexcept
	{
		_target = target;
	}


template<class T>
	inline typename PIDControl<T>::ValueType
	PIDControl<T>::process (ValueType measured_value, Time dt) noexcept
	{
		ValueType error;
		if (_winding)
		{
			error = clamped<ValueType> (_target - measured_value, -2.0, +2.0);
			if (std::abs (error) > 1.0)
				error = error - sgn (error) * 2.0;
		}
		else
			error = _target - measured_value;
		_integral += error * dt.quantity<Second>();
		clamp<ParamType> (_integral, _i_limit);
		_derivative = (error - _previous_error) / dt.quantity<Second>();
		if (!std::isfinite (_derivative))
			_derivative = 0.0;
		_output = clamped<ValueType> (_gain * (_p * sgn (error) * std::pow<ValueType> (std::abs (error), _error_power) +
											   _i * _integral +
											   _d * _derivative),
									  _output_limit);
		_previous_error = error;

		if (_output_smoothing_enabled)
			_output = _output_smoother.process (_output, dt);

		return _output;
	}


template<class T>
	inline typename PIDControl<T>::ValueType
	PIDControl<T>::output() const noexcept
	{
		return _output;
	}


template<class T>
	inline typename PIDControl<T>::ValueType
	PIDControl<T>::error() const noexcept
	{
		return _previous_error;
	}


template<class T>
	inline void
	PIDControl<T>::reset() noexcept
	{
		_output = ValueType();
		_previous_error = ValueType();
		_integral = ValueType();
		_derivative = ValueType();
	}


template<class T>
	inline Smoother<typename PIDControl<T>::ValueType>&
	PIDControl<T>::output_smoother() noexcept
	{
		return _output_smoother;
	}


template<class T>
	inline Smoother<typename PIDControl<T>::ValueType> const&
	PIDControl<T>::output_smoother() const noexcept
	{
		return _output_smoother;
	}

} // namespace xf

#endif

