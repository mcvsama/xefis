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

#ifndef XEFIS__SUPPORT__CONTROL__PID_CONTROLLER_H__INCLUDED
#define XEFIS__SUPPORT__CONTROL__PID_CONTROLLER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>
#include <neutrino/range.h>

// Standard:
#include <cstddef>
#include <cmath>


namespace xf {

template<class Param = double>
	struct PIDSettings
	{
		Param	p;	// Proportional term
		Param	i;	// Integral term
		Param	d;	// Derivative term
	};


/**
 * Proportional-Integral-Derivative controller.
 *
 * \param	pInput
 *			Type of setpoint and measured quantity.
 * \param	pProcessVariable
 *			Type of output value used to control the measured quantity. Default - same as pInput.
 * \param	pParam
 *			Floating-point type for internal computations, default is double.
 *
 * TODO safety functions: limit derivative or something so it's not 0/nan/inf and the result is limited to certain range.
 * TODO protect from infs and nans
 */
template<class pInput, class pProcessVariable = pInput, class pParam = double>
	class PIDController
	{
	  public:
		using Input				= pInput;
		using ProcessVariable	= pProcessVariable;
		using Param				= pParam;

		using Settings			= PIDSettings<pParam>;

		using Integral		= decltype (std::declval<Input>() * std::declval<si::Time>());
		using Derivative	= decltype (std::declval<Input>() / std::declval<si::Time>());

	  public:
		// Ctor
		PIDController() = default;

		// Ctor
		explicit
		PIDController (Settings const& settings, Input setpoint);

		// Ctor
		explicit
		PIDController (Param p, Param i, Param d, Input setpoint);

		/**
		 * Set winding. That is value -1.0 is equal to 1.0.
		 * When using winding, the measured value is expected to
		 * be winded up, too.
		 */
		void
		set_winding (bool winding)
			{ _winding = winding; }

		/**
		 * Get P parameter.
		 */
		Param
		p() const noexcept
			{ return _p; }

		/**
		 * Set P parameter.
		 */
		void
		set_p (Param p) noexcept
			{ _p = p; }

		/**
		 * Get I parameter.
		 */
		Param
		i() const noexcept
			{ return _i; }

		/**
		 * Set I parameter.
		 */
		void
		set_i (Param i) noexcept
			{ _i = i; }

		/**
		 * Get D parameter.
		 */
		Param
		d() const noexcept
			{ return _d; }

		/**
		 * Set D parameter.
		 */
		void
		set_d (Param d) noexcept
			{ _d = d; }

		/**
		 * Set P, I and D at once.
		 */
		void
		set_pid (Settings const& settings) noexcept;

		/**
		 * Return overall gain of all three parameters.
		 */
		Param
		gain() const noexcept
			{ return _gain; }

		/**
		 * Set overall gain for all three paramters.
		 */
		void
		set_gain (Param gain) noexcept
			{ _gain = gain; }

		/**
		 * I (integral) parameter limit.
		 */
		std::optional<Range<Integral>>
		integral_limit() const noexcept
			{ return _integral_limit; }

		/**
		 * Set I (integral) parameter limit.
		 */
		void
		set_integral_limit (Range<Integral> limit) noexcept
			{ _integral_limit = limit; }

		/**
		 * Set I (integral) parameter limit.
		 */
		void
		set_integral_limit (std::optional<Range<Integral>> limit) noexcept
			{ _integral_limit = limit; }

		/**
		 * Output limit.
		 */
		Range<ProcessVariable>
		output_limit() const noexcept
			{ return _output_limit; }

		/**
		 * Set output limit.
		 */
		void
		set_output_limit (Range<ProcessVariable> limit) noexcept
			{ _output_limit = limit; }

		/**
		 * Set setpoint value. If winding is enabled,
		 * then the setpoint should be normalized to [-1..1].
		 */
		void
		set_setpoint (Input setpoint) noexcept
			{ _setpoint = setpoint; }

		/**
		 * Process value for given dt (timespan) and return new value.
		 * Input value should be normalized to [-1..1].
		 */
		ProcessVariable
		process (Input measured, si::Time dt) noexcept;

		/**
		 * Same as process (Input, Time), but also provide setpoint value to be set.
		 */
		ProcessVariable
		process (Input setpoint, Input measured, si::Time dt) noexcept;

		/**
		 * Alias for process().
		 */
		ProcessVariable
		operator() (Input measured, si::Time dt) noexcept
			{ return process (measured, dt); }

		/**
		 * Alias for process().
		 */
		ProcessVariable
		operator() (Input setpoint, Input measured, si::Time dt) noexcept
			{ return process (setpoint, measured, dt); }

		/**
		 * Return current controller output value.
		 * TODO change to value() to be compatible with Smoother/RangeSmoother
		 */
		ProcessVariable
		output() const noexcept
			{ return _output; }

		/**
		 * Return error value.
		 */
		Input
		error() const noexcept
			{ return _previous_error; }

		/**
		 * Reset to default state.
		 */
		void
		reset() noexcept;

	  private:
		bool							_winding					= false;
		Input							_setpoint					{ };
		Input							_previous_error				{ };
		Integral						_integrated_error			{ };
		Derivative						_error_derivative			{ };
		Param							_p							= 0.0;
		Param							_i							= 0.0;
		std::optional<Range<Integral>>	_integral_limit;
		Param							_d							= 0.0;
		Param							_gain						= 1.0;
		ProcessVariable					_output						{ };
		Range<ProcessVariable>			_output_limit				{ -std::numeric_limits<ProcessVariable>::max(), std::numeric_limits<ProcessVariable>::max() };
	};


template<class V, class C, class P>
	inline
	PIDController<V, C, P>::PIDController (Settings const& settings, Input setpoint):
		_setpoint (setpoint),
		_p (settings.p),
		_i (settings.i),
		_d (settings.d)
	{
		reset();
	}


template<class V, class C, class P>
	inline
	PIDController<V, C, P>::PIDController (Param p, Param i, Param d, Input setpoint):
		_setpoint (setpoint),
		_p (p),
		_i (i),
		_d (d)
	{
		reset();
	}


template<class V, class C, class P>
	inline void
	PIDController<V, C, P>::set_pid (Settings const& settings) noexcept
	{
		_p = settings.p;
		_i = settings.i;
		_d = settings.d;
	}


template<class V, class C, class P>
	inline typename PIDController<V, C, P>::ProcessVariable
	PIDController<V, C, P>::process (Input measured, si::Time dt) noexcept
	{
		using si::isfinite;
		using si::abs;
		using std::isfinite;
		using std::abs;

		Input error;

		if (_winding)
		{
			// TODO do it better
			error = clamped<Input> (measured - _setpoint, Input (-2.0), Input (+2.0));

			if (abs (error) > Input (1.0))
				error = error - sgn (error) * Input (2.0);
		}
		else
			error = measured - _setpoint;

		// I:
		_integrated_error += error * dt;

		if (_integral_limit)
			clamp (_integrated_error, *_integral_limit);

		// D:
		_error_derivative = (error - _previous_error) / dt;

		if (!isfinite (_error_derivative))
			_error_derivative = Derivative();

		// P and the rest:
		auto anti_error_action = -_gain * (_p * error + _i * _integrated_error / 1_s + _d * _error_derivative * 1_s);
		_output = clamped<ProcessVariable> (ProcessVariable (si::quantity (anti_error_action)), _output_limit);
		_previous_error = error;

		return _output;
	}


template<class V, class C, class P>
	inline typename PIDController<V, C, P>::ProcessVariable
	PIDController<V, C, P>::process (Input setpoint, Input measured, si::Time dt) noexcept
	{
		set_setpoint (setpoint);
		return process (measured, dt);
	}


template<class V, class C, class P>
	inline void
	PIDController<V, C, P>::reset() noexcept
	{
		_output = ProcessVariable();
		_previous_error = Input();
		_integrated_error = Integral();
		_error_derivative = Derivative();
	}

} // namespace xf

#endif

