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

#ifndef NEUTRINO__MATH__NORMAL_DISTRIBUTION_H__INCLUDED
#define NEUTRINO__MATH__NORMAL_DISTRIBUTION_H__INCLUDED

// Standard:
#include <cstddef>
#include <random>

// Neutrino:
#include <neutrino/math/normal_variable.h>


namespace neutrino {

/**
 * A wrapper for std::normal_distribution that allows using non-fundamental types as values.
 */
template<class pValue>
	class NormalDistribution
	{
	  public:
		using Value = pValue;

	  public:
		// Ctor
		NormalDistribution (Value mean, Value stddev);

		// Ctor
		NormalDistribution (NormalVariable<Value> const&);

		// Ctor
		NormalDistribution (NormalDistribution const&) = default;

		// Copy operator
		NormalDistribution&
		operator= (NormalDistribution const&) = default;

		void
		reset()
			{ _dist.reset(); }

		Value
		mean() const noexcept
			{ return static_cast<Value> (1) * _dist.mean(); }

		Value
		stddev() const noexcept
			{ return static_cast<Value> (1) * _dist.stddev(); }

		template<class Generator>
			Value
			operator() (Generator& g)
				{ return static_cast<Value> (1) * _dist (g); }

	  private:
		std::normal_distribution<> _dist;
	};


template<class V>
	inline
	NormalDistribution<V>::NormalDistribution (Value mean, Value stddev):
		_dist (mean.value(), stddev.value())
	{ }


template<class V>
	inline
	NormalDistribution<V>::NormalDistribution (NormalVariable<Value> const& var):
		_dist (var.mean().value(), var.stddev().value())
	{ }

} // namespace neutrino

#endif

