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

#ifndef XEFIS__TEST__TEST_PROCESSING_LOOP_H__INCLUDED
#define XEFIS__TEST__TEST_PROCESSING_LOOP_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/processing_loop.h>

// Neutrino:
#include <neutrino/logger.h>

// Standard:
#include <cstddef>
#include <sstream>


namespace xf {

class TestProcessingLoop: public ProcessingLoop
{
  public:
	static thread_local inline std::ostringstream	log_buffer;
	static thread_local inline nu::LoggerOutput		logger_output	{ log_buffer };
	static thread_local inline nu::Logger			logger			{ logger_output };

  public:
	// Ctor
	explicit
	TestProcessingLoop (si::Time const cycle_dt):
		ProcessingLoop ("test loop", 1 / cycle_dt, logger),
		_cycle_dt (cycle_dt)
	{ }

	void
	next_cycle()
	{
		_now += _cycle_dt;
		execute_cycle (_now);
	}

	void
	next_cycles (size_t cycles)
	{
		for (size_t i = 0; i < cycles; ++i)
			next_cycle();
	}

  private:
	si::Time	_cycle_dt;
	si::Time	_now { 0_s };
};

} // namespace xf

#endif

