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

#ifndef NEUTRINO__MATH__HISTOGRAM_H__INCLUDED
#define NEUTRINO__MATH__HISTOGRAM_H__INCLUDED

// Standard:
#include <algorithm>
#include <cstddef>
#include <vector>

// Neutrino:
#include <neutrino/numeric.h>


namespace neutrino {

template<class Value>
	class Histogram
	{
	  public:
		using Bins = std::vector<std::size_t>;

	  public:
		// Ctor
		template<class Iterator>
			explicit
			Histogram (Iterator begin, Iterator end, Value bin_width, std::optional<Value> x_min = {}, std::optional<Value> x_max = {});

		/**
		 * Return minimum value of the histogram (X-axis value).
		 * For minimum recorded value use min().
		 */
		Value
		x_min() const noexcept;

		/**
		 * Return maximum value of the histogram (X-axis value).
		 * For maximum recorded value use max().
		 */
		Value
		x_max() const noexcept;

		/**
		 * Return number of samples in the biggest bin.
		 */
		std::size_t
		y_max() const noexcept;

		/**
		 * Return reference to bins vector.
		 */
		Bins const&
		bins() const noexcept;

		/**
		 * Return number of samples in histogram.
		 */
		std::size_t
		n_samples() const noexcept;

		/**
		 * Return bin index for given value.
		 */
		std::size_t
		bin_index (Value const&) const;

		/**
		 * Minimum recorded value.
		 */
		Value
		min() const noexcept;

		/**
		 * Maximum recorded value.
		 */
		Value
		max() const noexcept;

		/**
		 * Mean value for the data.
		 */
		Value
		mean() const noexcept;

		/**
		 * Median.
		 */
		Value
		median() const noexcept;

		/**
		 * Standard deviation.
		 */
		Value
		stddev() const noexcept;

		/**
		 * Return "normalized" percentile (in range 0..1) of values above given argument.
		 * That is what fraction of all samples constitute values above given [equal_or_above] value.
		 *
		 * It counts data already put in bins, so it's not exact. Its precision depends on bin width.
		 */
		float
		normalized_percentile_for (Value equal_or_above) const;

	  private:
		Value const	_bin_width;
		Value		_x_min;
		Value		_x_max;
		std::size_t	_y_max		{ 0 };
		std::size_t	_n_samples	{ 0 };
		Bins		_bins;
		Value		_min;
		Value		_max;
		Value		_mean;
		Value		_median;
		Value		_stddev;
	};


template<class Value>
	template<class Iterator>
		inline
		Histogram<Value>::Histogram (Iterator begin, Iterator end, Value bin_width, std::optional<Value> x_min, std::optional<Value> x_max):
			_bin_width (bin_width)
		{
			if (begin == end)
				throw std::length_error ("can't compute histogram for zero-length sequence");

			auto [min_it, max_it] = std::minmax_element (begin, end);
			_min = *min_it;
			_max = *max_it;

			_x_min = x_min.value_or (_min);
			_x_max = x_max.value_or (_max);

			_bins.resize (static_cast<std::size_t> (std::ceil ((_x_max - _x_min) / _bin_width)), 0u);
			auto const bins = _bins.size();

			for (Iterator v = begin; v != end; ++v)
			{
				std::size_t nth_bin = bin_index (*v);
				++_n_samples;

				if (0 <= nth_bin && nth_bin < bins)
				{
					auto& count = _bins[nth_bin];
					count++;

					if (count > _y_max)
						_y_max = count;
				}
			}

			_mean = neutrino::mean (begin, end);
			_median = neutrino::median (begin, end);
			_stddev = neutrino::stddev (begin, end);
		}


template<class Value>
	inline Value
	Histogram<Value>::x_min() const noexcept
	{
		return _x_min;
	}


template<class Value>
	inline Value
	Histogram<Value>::x_max() const noexcept
	{
		return _x_max;
	}


template<class Value>
	inline std::size_t
	Histogram<Value>::y_max() const noexcept
	{
		return _y_max;
	}


template<class Value>
	inline auto
	Histogram<Value>::bins() const noexcept -> Bins const&
	{
		return _bins;
	}


template<class Value>
	inline std::size_t
	Histogram<Value>::n_samples() const noexcept
	{
		return _n_samples;
	}


template<class Value>
	inline std::size_t
	Histogram<Value>::bin_index (Value const& value) const
	{
		return (value - _x_min) / _bin_width;
	}


template<class Value>
	inline Value
	Histogram<Value>::min() const noexcept
	{
		return _min;
	}


template<class Value>
	inline Value
	Histogram<Value>::max() const noexcept
	{
		return _max;
	}


template<class Value>
	inline Value
	Histogram<Value>::mean() const noexcept
	{
		return _mean;
	}


template<class Value>
	inline Value
	Histogram<Value>::median() const noexcept
	{
		return _median;
	}


template<class Value>
	inline Value
	Histogram<Value>::stddev() const noexcept
	{
		return _stddev;
	}


template<class Value>
	inline float
	Histogram<Value>::normalized_percentile_for (Value equal_or_above) const
	{
		std::size_t count = 0;

		for (std::size_t b = bin_index (equal_or_above); b < _bins.size(); ++b)
			count += _bins[b];

		return 1.0f * count / n_samples();
	}

} // namespace neutrino

#endif

