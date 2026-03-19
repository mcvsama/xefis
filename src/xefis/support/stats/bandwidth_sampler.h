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

#ifndef XEFIS__SUPPORT__STATS__BANDWIDTH_SAMPLER_H__INCLUDED
#define XEFIS__SUPPORT__STATS__BANDWIDTH_SAMPLER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/format.h>

// Boost:
#include <boost/circular_buffer.hpp>

// Standard:
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <vector>


namespace xf {

/**
 * Collects a rolling history of fixed-width bandwidth samples from recorded byte counts.
 */
class BandwidthSampler
{
  public:
	using Bandwidth		= decltype (1.0 / si::Time (1.0));
	using SamplesBuffer	= boost::circular_buffer<Bandwidth>;

	struct Parameters
	{
		si::Time	bandwidth_measurement_interval	{ 1_s };
		std::size_t	bandwidth_history_size			{ 300u };
	};

  public:
	// Ctor
	BandwidthSampler();

	// Ctor
	explicit
	BandwidthSampler (Parameters parameters);

	/**
	 * Add bytes observed at the provided time to the corresponding open time bucket.
	 */
	void
	record_bytes (std::size_t bytes, si::Time at_time);

	/**
	 * Add bytes accumulated since the previous observation and close any buckets ending at the provided time.
	 * If no prior bucket exists, assume the bytes cover the interval immediately preceding @at_time.
	 */
	void
	record_bytes_up_to (std::size_t bytes, si::Time at_time);

	/**
	 * Advance histogram buckets up to the provided time and emit completed bandwidth samples.
	 */
	void
	flush (si::Time now);

	[[nodiscard]]
	std::ranges::subrange<SamplesBuffer::const_iterator>
	samples() const
		{ return { _samples.begin(), _samples.end() }; }

	[[nodiscard]]
	si::Time
	bandwidth_measurement_interval() const noexcept;

	[[nodiscard]]
	static std::string
	format_bandwidth (Bandwidth const bandwidth, uint8_t const width_excl_dot = 3)
		{ return nu::format_unit (8 * bandwidth.in<si::Hertz>(), width_excl_dot, "b/s"); }

  private:
	Parameters				_parameters;
	std::optional<si::Time>	_bucket_start_time;
	std::size_t				_bucket_bytes		{ 0u };
	SamplesBuffer			_samples;
};

} // namespace xf

#endif
