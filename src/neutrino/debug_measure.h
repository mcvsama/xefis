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

#ifndef NEUTRINO__DEBUG_MEASURE_H__INCLUDED
#define NEUTRINO__DEBUG_MEASURE_H__INCLUDED

// Standard:
#include <cstddef>
#include <ostream>

// Neutrino:
#include <neutrino/time_helper.h>


#define MEASURE(code) do { \
	auto duration = neutrino::TimeHelper::measure ([&] { (code); }); \
	\
	if (duration > 0.001_ms) \
	{ \
		auto num_chars = duration / 1_ms; \
		std::cout << std::setprecision(6) << std::fixed << duration.in<si::Second>() << " s  " << std::string (num_chars, '#'); \
		std::cout << std::string (100 - num_chars, '_') << #code << "\n"; \
	} \
} while (false)


namespace neutrino::debug {

class Timer
{
  public:
	// Ctor
	explicit
	Timer();

	si::Time
	get();

	si::Time
	delta();

  private:
	si::Time	_start_timestamp;
	si::Time	_last_check			{ 0_s };
};


inline
Timer::Timer():
	_start_timestamp (TimeHelper::now())
{ }


inline si::Time
Timer::get()
{
	_last_check = TimeHelper::now();
	return _last_check - _start_timestamp;
}


inline si::Time
Timer::delta()
{
	auto now = TimeHelper::now();
	return now - std::exchange (_last_check, now);
}


inline std::ostream&
operator<< (std::ostream& os, Timer& t)
{
	return os << std::setprecision (6) << std::fixed << t.get();
}

} // namespace neutrino::debug

#endif

