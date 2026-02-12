/* vim:ts=4
 *
 * Copyleft 2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__MACHINE_H__INCLUDED
#define XEFIS__CORE__MACHINE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/clock.h>
#include <xefis/core/processing_loop.h>
#include <xefis/core/screen.h>

// Neutrino:
#include <neutrino/noncopyable.h>

// Standard:
#include <cstddef>
#include <list>
#include <ranges>
#include <string>
#include <string_view>


namespace xf {

class ConfiguratorWidget;


class Machine: private nu::Noncopyable
{
  private:
	// Must be lists to make sure that iterators returned by clocks(), processing_loops(), etc
	// won't get invalidated when new object is added.
	using Clocks			= std::list<Clock*>;
	using ProcessingLoops	= std::list<ProcessingLoop*>;
	using Screens			= std::list<Screen*>;

  public:
	// Ctor
	explicit
	Machine (Xefis&, std::u8string_view instance = u8"");

	/**
	 * Dtor
	 * Has to be virtual since we'll possibly refer to machines thorugh base class
	 * (that is Machine).
	 */
	virtual
	~Machine();

	/**
	 * Return main Xefis object.
	 */
	Xefis&
	xefis() const noexcept
		{ return _xefis; }

	/**
	 * Return machine instance name.
	 */
	[[nodiscard]]
	std::u8string_view
	instance() const
		{ return _instance; }

	/**
	 * A sequence of registered clocks.
	 */
	std::ranges::subrange<Clocks::iterator>
	clocks() noexcept
		{ return { _clocks.begin(), _clocks.end() }; }

	/**
	 * A sequence of registered processing loops.
	 */
	std::ranges::subrange<Clocks::const_iterator>
	clocks() const noexcept
		{ return { _clocks.begin(), _clocks.end() }; }

	/**
	 * A sequence of registered processing loops.
	 */
	std::ranges::subrange<ProcessingLoops::iterator>
	processing_loops() noexcept
		{ return { _processing_loops.begin(), _processing_loops.end() }; }

	/**
	 * A sequence of registered processing loops.
	 */
	std::ranges::subrange<ProcessingLoops::const_iterator>
	processing_loops() const noexcept
		{ return { _processing_loops.begin(), _processing_loops.end() }; }

	/**
	 * A sequence of registered screens.
	 */
	std::ranges::subrange<Screens::iterator>
	screens() noexcept
		{ return { _screens.begin(), _screens.end() }; }

	/**
	 * A sequence of registered processing loops.
	 */
	std::ranges::subrange<Screens::const_iterator>
	screens() const noexcept
		{ return { _screens.begin(), _screens.end() }; }

	/**
	 * Register a clock.
	 * They will be accessible in the associated ConfiguratorWidget.
	 * They will not be advanced in any way when advancing processin loop.
	 */
	void
	register_clock (Clock& clock)
		{ _clocks.push_back (&clock); }

	/**
	 * Register a processing loop.
	 * Registered loops' time will be advanced when advance_loop() is called.
	 * They will also be accessible in the associated ConfiguratorWidget.
	 */
	void
	register_processing_loop (ProcessingLoop& processing_loop)
		{ _processing_loops.push_back (&processing_loop); }

	/**
	 * Register a screen.
	 * It will be accessible in the associated ConfiguratorWidget.
	 */
	void
	register_screen (Screen& screen)
		{ _screens.push_back (&screen); }

	/**
	 * Advance machine's processing loops.
	 */
	void
	advance_loops (si::Time);

	/**
	 * Show configurator widget.
	 */
	void
	show_configurator();

	/**
	 * Pause/unpause the machine.
	 * This is forwarded to all machine's processing loops.
	 */
	void
	set_paused (bool paused);

	[[nodiscard]]
	bool
	paused() const noexcept
		{ return _paused; }

  private:
	Xefis&								_xefis;
	std::u8string						_instance;
	Clocks								_clocks;
	ProcessingLoops						_processing_loops;
	Screens								_screens;
	std::unique_ptr<ConfiguratorWidget>	_configurator_widget;
	bool								_paused { false };
};

} // namespace xf

#endif

