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

#ifndef XEFIS__CORE__MACHINE_H__INCLUDED
#define XEFIS__CORE__MACHINE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/screen.h>
#include <xefis/core/processing_loop.h>

// Neutrino:
#include <neutrino/noncopyable.h>
#include <neutrino/sequence.h>

// Standard:
#include <cstddef>
#include <vector>


namespace xf {

class ConfiguratorWidget;


class Machine: private Noncopyable
{
  private:
	using ProcessingLoops	= std::list<ProcessingLoop*>;
	using Screens			= std::list<Screen*>;

  public:
	// Ctor
	explicit
	Machine (Xefis&);

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
	 * A sequence of registered processing loops.
	 */
	Sequence<ProcessingLoops::iterator>
	processing_loops() noexcept
		{ return { _processing_loops.begin(), _processing_loops.end() }; }

	/**
	 * A sequence of registered processing loops.
	 */
	Sequence<ProcessingLoops::const_iterator>
	processing_loops() const noexcept
		{ return { _processing_loops.begin(), _processing_loops.end() }; }

	/**
	 * A sequence of registered screens.
	 */
	Sequence<Screens::iterator>
	screens() noexcept
		{ return { _screens.begin(), _screens.end() }; }

	/**
	 * A sequence of registered processing loops.
	 */
	Sequence<Screens::const_iterator>
	screens() const noexcept
		{ return { _screens.begin(), _screens.end() }; }

	/**
	 * Register a processing loop.
	 */
	void
	register_processing_loop (ProcessingLoop& processing_loop)
		{ _processing_loops.push_back (&processing_loop); }

	/**
	 * Register a screen.
	 */
	void
	register_screen (Screen& screen)
		{ _screens.push_back (&screen); }

	/**
	 * Show configurator widget.
	 */
	void
	show_configurator();

  private:
	Xefis&								_xefis;
	ProcessingLoops						_processing_loops;
	Screens								_screens;
	std::unique_ptr<ConfiguratorWidget>	_configurator_widget;
};

} // namespace xf

#endif

