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

#ifndef XEFIS__UTILITY__TRANSISTOR_H__INCLUDED
#define XEFIS__UTILITY__TRANSISTOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <algorithm>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/smoother.h>


namespace xf {

/**
 * Transitions output value between two input values
 * over configured period of time.
 */
template<class tValueType>
	class Transistor
	{
	  public:
		typedef tValueType ValueType;

	  public:
		/**
		 * Ctor
		 * At the beginning both input values are constructed with their
		 * default operator. The first one is selected as output.
		 */
		explicit
		Transistor (Time smoothing_time, Time precision = 1_ms) noexcept;

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
		 * Switch output.
		 * \param	Input
		 *			Input number - 0 or 1.
		 */
		template<uint8_t Input>
			void
			select_input() noexcept;

		/**
		 * Switch output.
		 * \param	enable_second If true, enable second input, otherwise - use first input.
		 */
		void
		select_second_input (bool enable_second) noexcept;

		/**
		 * Resets output immediately, without doind long, smoothed transition.
		 */
		void
		reset() noexcept;

		/**
		 * Return smoothed sample from given input sample
		 * and time from last update.
		 * \param	s0 Value of the first input.
		 * \param	s1 Value of the second input.
		 * \param	dt Time delta from last update.
		 * \return	Computed output value.
		 */
		ValueType
		process (ValueType s0, ValueType s1, Time dt) noexcept;

		/**
		 * Return last computed value.
		 */
		ValueType
		value() const noexcept;

		/**
		 * Return internal smoother.
		 */
		Smoother<ValueType>&
		smoother() noexcept;

		/**
		 * Return internal smoother.
		 */
		Smoother<ValueType> const&
		smoother() const noexcept;

	  private:
		Smoother<ValueType>	_smoother;
		bool				_selected_second	= false;
		ValueType			_output				= ValueType();
	};


template<class T>
	inline
	Transistor<T>::Transistor (Time smoothing_time, Time precision) noexcept:
		_smoother (smoothing_time, precision)
	{ }


template<class T>
	inline Time
	Transistor<T>::smoothing_time() const noexcept
	{
		return _smoother.smoothing_time();
	}


template<class T>
	inline void
	Transistor<T>::set_smoothing_time (Time smoothing_time) noexcept
	{
		_smoother.set_smoothing_time (smoothing_time);
	}


template<class T>
	inline Time
	Transistor<T>::precision() const noexcept
	{
		return _smoother.precision();
	}


template<class T>
	inline void
	Transistor<T>::set_precision (Time precision) noexcept
	{
		_smoother.set_precision (precision);
	}


template<class T>
	template<uint8_t Input>
		inline void
		Transistor<T>::select_input() noexcept
		{
			static_assert (Input == 0 || Input == 1, "Input must be 0 or 1");
			_selected_second = Input == 1;
		}


template<class T>
	inline void
	Transistor<T>::select_second_input (bool select_second) noexcept
	{
		_selected_second = select_second;
	}


template<class T>
	inline void
	Transistor<T>::reset() noexcept
	{
		_smoother.reset (_selected_second ? 1.0 : 0.0);
	}


template<class T>
	inline typename Transistor<T>::ValueType
	Transistor<T>::process (ValueType s0, ValueType s1, Time dt) noexcept
	{
		ValueType f = _smoother.process (_selected_second ? 1.0 : 0.0, dt);
		return _output = xf::renormalize (f, Range<double> (0.0, 1.0), Range<ValueType> (s0, s1));
	}


template<class T>
	inline typename Transistor<T>::ValueType
	Transistor<T>::value() const noexcept
	{
		return _output;
	}


template<class T>
	inline Smoother<typename Transistor<T>::ValueType>&
	Transistor<T>::smoother() noexcept
	{
		return _smoother;
	}


template<class T>
	inline Smoother<typename Transistor<T>::ValueType> const&
	Transistor<T>::smoother() const noexcept
	{
		return _smoother;
	}

} // namespace xf

#endif

