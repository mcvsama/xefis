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

#ifndef XEFIS__UTILITY__HISTOGRAM_H__INCLUDED
#define XEFIS__UTILITY__HISTOGRAM_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>


namespace xf {

template<class Value>
	class Histogram
	{
	  public:
		using Bins = std::vector<std::size_t>;

	  public:
		// Ctor
		template<class Iterator>
			explicit
			Histogram (Iterator begin, Iterator end, Value bin_width, std::optional<Value> min = {}, std::optional<Value> max = {});

		Value
		x_min() const noexcept;

		Value
		x_max() const noexcept;

		std::size_t
		y_max() const noexcept;

		Bins const&
		bins() const noexcept;

		std::size_t
		samples() const noexcept;

	  private:
		Value		_x_min;
		Value		_x_max;
		std::size_t	_y_max		{ 0 };
		std::size_t	_samples	{ 0 };
		Bins		_bins;
	};


template<class Value>
	template<class Iterator>
		inline
		Histogram<Value>::Histogram (Iterator begin, Iterator end, Value bin_width, std::optional<Value> min, std::optional<Value> max)
		{
			if (!min)
				for (Iterator v = begin; v != end; ++v)
					if (!min || *min > *v)
						min = *v;

			if (!max)
				for (Iterator v = begin; v != end; ++v)
					if (!max || *max < *v)
						max = *v;

			_x_min = *min;
			_x_max = *max;
			_bins.resize (static_cast<std::size_t> (std::ceil ((*max - *min) / bin_width)), 0u);
			auto const bins = _bins.size();

			for (Iterator v = begin; v != end; ++v)
			{
				std::size_t nth_bin = (*v - *min) / bin_width;
				++_samples;

				if (0 <= nth_bin && nth_bin < bins)
				{
					auto& count = _bins[nth_bin];
					count++;

					if (count > _y_max)
						_y_max = count;
				}
			}
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

} // namespace xf

#endif

