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

#ifndef XEFIS__UTILITY__CONVERGENCE_H__INCLUDED
#define XEFIS__UTILITY__CONVERGENCE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

/**
 * Executes a formula function iteratively, until result converges to a value,
 * or iterations limit is reached.
 */
template<class tValueType>
	class Convergence
	{
	  public:
		typedef tValueType								ValueType;
		typedef std::function<ValueType (ValueType)>	FormulaFunction;

	  public:
		// Ctor
		explicit
		Convergence (ValueType delta, unsigned int max_iterations, FormulaFunction formula) noexcept;

		/**
		 * Run iterations until convergence or iterations limit.
		 * Return true, if value has converged (difference between two last computations is less than
		 * configured delta).
		 * \param	initial_value is the initial value to be passed to the formula function.
		 */
		bool
		converge (ValueType initial_value);

		/**
		 * Return most recently pushed sample.
		 */
		ValueType
		result() const noexcept;

		/**
		 * Return number of iterations that was needed
		 * to compute result.
		 */
		unsigned int
		iterations() const noexcept;

	  private:
		ValueType		_delta;
		unsigned int	_max_iterations;
		unsigned int	_actual_iterations = 0;
		FormulaFunction	_formula;
		ValueType		_result;
	};


template<class V>
	inline
	Convergence<V>::Convergence (ValueType delta, unsigned int max_iterations, FormulaFunction formula) noexcept:
		_delta (delta),
		_max_iterations (max_iterations),
		_formula (formula)
	{ }


template<class V>
	inline bool
	Convergence<V>::converge (ValueType initial_value)
	{
		unsigned int i = 0;
		ValueType rp = _formula (initial_value);
		ValueType rn;
		ValueType delta;

		do {
			rn = _formula (rp);
			delta = std::abs (rn - rp);
			rp = rn;
		}
		while (delta > _delta && i++ < _max_iterations);

		_result = rn;
		_actual_iterations = i;

		return delta <= _delta;
	}


template<class V>
	inline typename Convergence<V>::ValueType
	Convergence<V>::result() const noexcept
	{
		return _result;
	}


template<class V>
	inline unsigned int
	Convergence<V>::iterations() const noexcept
	{
		return _actual_iterations;
	}


/**
 * Simpler API for convergence.
 */
template<class ValueType>
	inline std::optional<ValueType>
	converge (ValueType initial_value, ValueType delta, unsigned int max_iterations, std::function<ValueType (ValueType)> formula) noexcept
	{
		Convergence<ValueType> comp (delta, max_iterations, formula);
		if (comp.converge (initial_value))
			return comp.result();
		return { };
	}

} // namespace xf

#endif

