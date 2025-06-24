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

#ifndef XEFIS__SUPPORT__CORE__SINGLE_LOOP_MACHINE_H__INCLUDED
#define XEFIS__SUPPORT__CORE__SINGLE_LOOP_MACHINE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>
#include <xefis/core/processing_loop.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Typical machine with one processing loop, logger and basic stuff.
 */
class SingleLoopMachine: public xf::Machine
{
  public:
	// Ctor
	explicit
	SingleLoopMachine (xf::Xefis&, nu::Logger const&, si::Frequency loop_frequency);

  protected:
	[[nodiscard]]
	nu::Logger&
	logger()
		{ return _logger; }

	[[nodiscard]]
	nu::Logger const&
	logger() const
		{ return _logger; }

	[[nodiscard]]
	xf::ProcessingLoop&
	loop()
		{ return _loop; }

	[[nodiscard]]
	xf::ProcessingLoop const&
	loop() const
		{ return _loop; }

	/**
	 * Prepare and start the machine.
	 */
	void
	start();

	/**
	 * Connect modules' sockets.
	 */
	virtual void
	connect_modules() = 0;

  private:
	nu::Logger			_logger;
	xf::ProcessingLoop	_loop;
};

} // namespace xf

#endif

