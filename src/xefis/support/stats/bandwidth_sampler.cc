/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Local:
#include "bandwidth_sampler.h"

// Neutrino:
#include <neutrino/format.h>

// Standard:
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>


namespace xf {

BandwidthSampler::BandwidthSampler():
	BandwidthSampler (Parameters())
{ }


BandwidthSampler::BandwidthSampler (Parameters const parameters):
	_parameters (parameters),
	_samples (std::max<std::size_t> (1u, parameters.bandwidth_history_size))
{ }


void
BandwidthSampler::record_bytes (std::size_t const bytes, si::Time const at_time)
{
	if (!_bucket_start_time)
		_bucket_start_time = at_time;

	flush (at_time);
	_bucket_bytes += bytes;
}


void
BandwidthSampler::record_bytes_up_to (std::size_t const bytes, si::Time const at_time)
{
	auto const interval = bandwidth_measurement_interval();

	if (!_bucket_start_time)
		_bucket_start_time = at_time - interval;

	_bucket_bytes += bytes;
	flush (at_time);
}


void
BandwidthSampler::flush (si::Time const now)
{
	auto const interval = bandwidth_measurement_interval();

	if (_bucket_start_time)
	{
		if (*_bucket_start_time + interval <= now)
		{
			auto const elapsed_intervals = static_cast<std::size_t> (std::floor ((now - *_bucket_start_time) / interval));

			if (elapsed_intervals > _samples.capacity())
			{
				_samples.clear();

				for (std::size_t i = 0u; i < _samples.capacity(); ++i)
					_samples.push_back (Bandwidth());
			}
			else
			{
				_samples.push_back (_bucket_bytes / interval);

				for (std::size_t i = 1u; i < elapsed_intervals; ++i)
					_samples.push_back (Bandwidth());
			}

			*_bucket_start_time += elapsed_intervals * interval;
			_bucket_bytes = 0u;
		}
	}
	else
		_bucket_start_time = now;
}


si::Time
BandwidthSampler::bandwidth_measurement_interval() const noexcept
{
	auto const interval = _parameters.bandwidth_measurement_interval;
	return interval > 0_s && isfinite (interval)
		? interval
		: 1_s;
}

} // namespace xf
