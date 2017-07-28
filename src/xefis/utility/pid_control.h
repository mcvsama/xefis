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


namespace xf {

/**
 * Proportional-Integral-Derivative controller.
 *
 * \param	pInput
 *			Type of set and measured quantity.
 * \param	pControl
 *			Type of output value used to control the measured quantity. Default - same as pInput.
 * \param	pParam
 *			Floating-point type for internal computations, default is double.
 *
 * TODO safety functions: limit derivative or something so it's not 0/nan/inf and the result is limited to certain range.
 * TODO protect from infs and nans
 */
template<class pInput, class pControl = pInput, class pParam = double>
	class PIDControl
	{
	  public:
		using Input			= pInput;
		using Control		= pControl;
		using Param			= pParam;

		using Integral		= decltype (std::declval<Input>() * std::declval<si::Time>());
		using Derivative	= decltype (std::declval<Input>() / std::declval<si::Time>());

		class Settings
		{
		  public:
			Param	p;	// Proportional term
			Param	i;	// Integral term
			Param	d;	// Derivative term
		};

	  public:
		// Ctor
		PIDControl();

		// Ctor
		explicit
		PIDControl (Settings const& settings, Input target);

		// Ctor
		explicit
		PIDControl (Param p, Param i, Param d, Input target);

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
		Param
		p() const noexcept;

		/**
		 * Set P parameter.
		 */
		void
		set_p (Param p) noexcept;

		/**
		 * Get I parameter.
		 */
		Param
		i() const noexcept;

		/**
		 * Set I parameter.
		 */
		void
		set_i (Param i) noexcept;

		/**
		 * Get D parameter.
		 */
		Param
		d() const noexcept;

		/**
		 * Set D parameter.
		 */
		void
		set_d (Param d) noexcept;

		/**
		 * Set P, I and D at once.
		 */
		void
		set_pid (Settings const& settings) noexcept;

		/**
		 * Return overall gain of all three parameters.
		 */
		Param
		gain() const noexcept;

		/**
		 * Set overall gain for all three paramters.
		 */
		void
		set_gain (Param gain) noexcept;

		/**
		 * I (integral) parameter limit.
		 */
		std::optional<Range<Integral>>
		integral_limit() const noexcept;

		/**
		 * Set I (integral) parameter limit.
		 */
		void
		set_integral_limit (Range<Integral> limit) noexcept;

		/**
		 * Set I (integral) parameter limit.
		 */
		void
		set_integral_limit (std::optional<Range<Integral>> limit) noexcept;

		/**
		 * Output limit.
		 */
		Range<Control>
		output_limit() const noexcept;

		/**
		 * Set output limit.
		 */
		void
		set_output_limit (Range<Control> limit) noexcept;

		/**
		 * Set target value. Set target value. If winding is enabled,
		 * then the target should be normalized to [-1..1].
		 */
		void
		set_target (Input target) noexcept;

		/**
		 * Process value for given dt (timespan) and return new value.
		 * Input value should be normalized to [-1..1].
		 */
		Control
		process (Input measured, si::Time dt) noexcept;

		/**
		 * Same as process (Input, Time), but also provide target value to be set.
		 */
		Control
		process (Input target, Input measured, Time dt) noexcept;

		/**
		 * Alias for process().
		 */
		Control
		operator() (Input input, Time dt) noexcept;

		/**
		 * Alias for process().
		 */
		Control
		operator() (Input target, Input measured, Time dt) noexcept;

		/**
		 * Return current controller output value.
		 * TODO change to value() to be compatible with Smoother/RangeSmoother
		 */
		Control
		output() const noexcept;

		/**
		 * Return error value.
		 */
		Input
		error() const noexcept;

		/**
		 * Reset to default state.
		 */
		void
		reset() noexcept;

	  private:
		bool							_winding					= false;
		Input							_target						{ };
		Input							_previous_error				{ };
		Integral						_integral					{ };
		Derivative						_derivative					{ };
		Param							_p							= 0.0;
		Param							_i							= 0.0;
		std::optional<Range<Integral>>	_integral_limit;
		Param							_d							= 0.0;
		Param							_gain						= 1.0;
		Control							_output						{ };
		Range<Control>					_output_limit				{ -std::numeric_limits<Control>::max(), std::numeric_limits<Control>::max() };
	};


template<class V, class C, class P>
	inline
	PIDControl<V, C, P>::PIDControl()
	{ }


template<class V, class C, class P>
	inline
	PIDControl<V, C, P>::PIDControl (Settings const& settings, Input target):
		_target (target),
		_p (settings.p),
		_i (settings.i),
		_d (settings.d)
	{
		reset();
	}


template<class V, class C, class P>
	inline
	PIDControl<V, C, P>::PIDControl (Param p, Param i, Param d, Input target):
		_target (target),
		_p (p),
		_i (i),
		_d (d)
	{
		reset();
	}


template<class V, class C, class P>
	inline void
	PIDControl<V, C, P>::set_winding (bool winding)
	{
		_winding = winding;
	}


template<class V, class C, class P>
	inline typename PIDControl<V, C, P>::Param
	PIDControl<V, C, P>::p() const noexcept
	{
		return _p;
	}


template<class V, class C, class P>
	inline void
	PIDControl<V, C, P>::set_p (Param p) noexcept
	{
		_p = p;
	}


template<class V, class C, class P>
	inline typename PIDControl<V, C, P>::Param
	PIDControl<V, C, P>::i() const noexcept
	{
		return _i;
	}


template<class V, class C, class P>
	inline void
	PIDControl<V, C, P>::set_i (Param i) noexcept
	{
		_i = i;
	}


template<class V, class C, class P>
	inline typename PIDControl<V, C, P>::Param
	PIDControl<V, C, P>::d() const noexcept
	{
		return _d;
	}


template<class V, class C, class P>
	inline void
	PIDControl<V, C, P>::set_d (Param d) noexcept
	{
		_d = d;
	}


template<class V, class C, class P>
	inline void
	PIDControl<V, C, P>::set_pid (Settings const& settings) noexcept
	{
		_p = settings.p;
		_i = settings.i;
		_d = settings.d;
	}


template<class V, class C, class P>
	inline typename PIDControl<V, C, P>::Param
	PIDControl<V, C, P>::gain() const noexcept
	{
		return _gain;
	}


template<class V, class C, class P>
	inline void
	PIDControl<V, C, P>::set_gain (Param gain) noexcept
	{
		_gain = gain;
	}


template<class V, class C, class P>
	inline std::optional<Range<typename PIDControl<V, C, P>::Integral>>
	PIDControl<V, C, P>::integral_limit() const noexcept
	{
		return _integral_limit;
	}


template<class V, class C, class P>
	inline void
	PIDControl<V, C, P>::set_integral_limit (Range<Integral> limit) noexcept
	{
		_integral_limit = limit;
	}


template<class V, class C, class P>
	inline void
	PIDControl<V, C, P>::set_integral_limit (std::optional<Range<Integral>> limit) noexcept
	{
		_integral_limit = limit;
	}


template<class V, class C, class P>
	inline Range<typename PIDControl<V, C, P>::Control>
	PIDControl<V, C, P>::output_limit() const noexcept
	{
		return _output_limit;
	}


template<class V, class C, class P>
	inline void
	PIDControl<V, C, P>::set_output_limit (Range<Control> limit) noexcept
	{
		_output_limit = limit;
	}


template<class V, class C, class P>
	inline void
	PIDControl<V, C, P>::set_target (Input target) noexcept
	{
		_target = target;
	}


template<class V, class C, class P>
	inline typename PIDControl<V, C, P>::Control
	PIDControl<V, C, P>::process (Input measured, Time dt) noexcept
	{
		using si::isfinite;
		using si::abs;
		using std::isfinite;
		using std::abs;

		Input error;

		if (_winding)
		{
			// TODO do it better
			error = clamped<Input> (_target - measured, Input (-2.0), Input (+2.0));
			if (abs (error) > Input (1.0))
				error = error - sgn (error) * Input (2.0);
		}
		else
			error = _target - measured;

		// I:
		_integral += error * dt;
		if (_integral_limit)
			clamp (_integral, *_integral_limit);
		// D:
		_derivative = (error - _previous_error) / dt;
		if (!si::isfinite (_derivative))
			_derivative = Derivative();
		// P and the rest:
		auto computed = _gain * (_p * error + _i * _integral / 1_s + _d * _derivative * 1_s);
		_output = clamped<Control> (Control (si::quantity (computed)), _output_limit);
		_previous_error = error;

		return _output;
	}


template<class V, class C, class P>
	inline typename PIDControl<V, C, P>::Control
	PIDControl<V, C, P>::process (Input target, Input measured, Time dt) noexcept
	{
		set_target (target);
		return process (measured, dt);
	}


template<class V, class C, class P>
	inline typename PIDControl<V, C, P>::Control
	PIDControl<V, C, P>::operator() (Input measured, Time dt) noexcept
	{
		return process (measured, dt);
	}


template<class V, class C, class P>
	inline typename PIDControl<V, C, P>::Control
	PIDControl<V, C, P>::operator() (Input target, Input measured, Time dt) noexcept
	{
		return process (target, measured, dt);
	}


template<class V, class C, class P>
	inline typename PIDControl<V, C, P>::Control
	PIDControl<V, C, P>::output() const noexcept
	{
		return _output;
	}


template<class V, class C, class P>
	inline typename PIDControl<V, C, P>::Input
	PIDControl<V, C, P>::error() const noexcept
	{
		return _previous_error;
	}


template<class V, class C, class P>
	inline void
	PIDControl<V, C, P>::reset() noexcept
	{
		_output = Control();
		_previous_error = Input();
		_integral = Integral();
		_derivative = Derivative();
	}

} // namespace xf

#endif

