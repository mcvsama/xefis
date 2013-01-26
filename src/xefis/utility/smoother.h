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

#ifndef XEFIS__UTILITY__SMOOTHER_H__INCLUDED
#define XEFIS__UTILITY__SMOOTHER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>


namespace Xefis {

/**
 * Implementation of one-pole LPF.
 * See http://musicdsp.org/showone.php?id=257
 */
template<class tValueType>
	class Smoother
	{
	  public:
		typedef tValueType ValueType;

	  public:
		Smoother (ValueType samples = 1.f) noexcept;

		/**
		 * \param	samples is number of samples after which returned value reaches
		 * 			99.99% of target value.
		 */
		void
		set_samples (ValueType samples) noexcept;

		/**
		 * Resets smoother to initial state (or given value).
		 */
		void
		reset (float value = 0.0f) noexcept;

		/**
		 * Return smoothed sample from given input sample.
		 */
		ValueType
		process (ValueType s, unsigned int iterations = 1) noexcept;

		/**
		 * Smooth (low-pass) given sequence.
		 */
		template<class ForwardIterator>
			void
			process (ForwardIterator begin, ForwardIterator end) noexcept;

		/**
		 * Fill sequence with smoothed samples, where input sample is @value.
		 * \param	begin,end Sequence to be filled.
		 * \param	value Input data that is to be smoothed. It's used for each
		 * 			sample in [begin, end) sequence.
		 */
		template<class ForwardIterator>
			void
			fill (ForwardIterator begin, ForwardIterator end, ValueType value) noexcept;


		/**
		 * Multiply sequence samples by given value in a smooth-way.
		 * \param	begin,end Sequence to modify.
		 * \param	value Multiplying value.
		 */
		template<class ForwardIterator>
			void
			multiply (ForwardIterator begin, ForwardIterator end, ValueType value) noexcept;

	  private:
		ValueType
		process_single_sample (ValueType s) noexcept;

	  private:
		ValueType _time;
		ValueType _z;
	};


template<class V>
	inline
	Smoother<V>::Smoother (ValueType samples) noexcept
	{
		set_samples (samples);
		reset();
	}


template<class V>
	inline void
	Smoother<V>::set_samples (ValueType samples) noexcept
	{
		_time = std::pow (0.01f, 2.0f / samples);
	}


template<class V>
	inline void
	Smoother<V>::reset (float value) noexcept
	{
		_z = value;
	}


template<class V>
	inline typename Smoother<V>::ValueType
	Smoother<V>::process (ValueType s, unsigned int iterations) noexcept
	{
		for (unsigned int i = 0; i < iterations; ++i)
			process_single_sample (s);
		return _z;
	}


template<class V>
	template<class ForwardIterator>
		inline void
		Smoother<V>::process (ForwardIterator begin, ForwardIterator end) noexcept
		{
			for (ForwardIterator c = begin; c != end; ++c)
				*c = process_single_sample (*c);
		}


template<class V>
	template<class ForwardIterator>
		inline void
		Smoother<V>::fill (ForwardIterator begin, ForwardIterator end, ValueType value) noexcept
		{
			for (ForwardIterator c = begin; c != end; ++c)
				*c = process_single_sample (value);
		}


template<class V>
	template<class ForwardIterator>
		inline void
		Smoother<V>::multiply (ForwardIterator begin, ForwardIterator end, ValueType value) noexcept
		{
			for (ForwardIterator c = begin; c != end; ++c)
				*c *= process_single_sample (value);
		}


template<class V>
	inline typename Smoother<V>::ValueType
	Smoother<V>::process_single_sample (ValueType s) noexcept
	{
		return _z = _time * (_z - s) + s;
	}

} // namespace Xefis

#endif

