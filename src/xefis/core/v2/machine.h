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

#ifndef XEFIS__CORE__V2__MACHINE_H__INCLUDED
#define XEFIS__CORE__V2__MACHINE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/processing_loop.h>


namespace x2 {
using namespace xf; // XXX

class Machine
{
  public:
	// Ctor
	Machine (Xefis*);

	/**
	 * Return main Xefis object.
	 */
	Xefis*
	xefis() const noexcept;

  protected:
	/**
	 * Create new processing loop with given frequency.
	 */
	template<class pProcessingLoop = ProcessingLoop, class ...Arg>
		pProcessingLoop*
		make_processing_loop (Arg&& ...args);

  private:
	Xefis*								_xefis;
	std::vector<Unique<ProcessingLoop>>	_processing_loops;
};


template<class pProcessingLoop, class ...Arg>
	inline pProcessingLoop*
	Machine::make_processing_loop (Arg&& ...args)
	{
		_processing_loops.push_back (std::make_unique<pProcessingLoop> (this, args...));

		return static_cast<pProcessingLoop*> (_processing_loops.back().get());
	}


inline Xefis*
Machine::xefis() const noexcept
{
	return _xefis;
}

} // namespace x2

#endif

