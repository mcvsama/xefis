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

#ifndef XEFIS__SUPPORT__EARTH__NAVIGATION__NAVAID_STORAGE_H__INCLUDED
#define XEFIS__SUPPORT__EARTH__NAVIGATION__NAVAID_STORAGE_H__INCLUDED

// Standard:
#include <cstddef>
#include <future>
#include <set>
#include <string_view>
#include <map>

// Lib:
#include <kdtree++/kdtree.hpp>

// Neutrino:
#include <neutrino/logger.h>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "navaid.h"


namespace xf {

class NavaidStorage
{
	struct Group
	{
		std::map<QString, Navaid const*>			by_identifier;
		std::multimap<si::Frequency, Navaid const*>	by_frequency;
	};

	typedef std::map<Navaid::Type, Group> NavaidsByType;

	enum class Fix
	{
		FIX				= 50,	// Fix
	};

	enum class Nav
	{
		OTHER			= 0,
		NDB				= 2,	// NDB
		VOR				= 3,	// VOR, VOR-DME, VORTAC
		LOC				= 4,	// ILS localizer component
		LOCSA			= 5,	// Stand-alone localiser
		GS				= 6,	// ILS glideslope component
		OM				= 7,	// ILS outer marker
		MM				= 8,	// ILS middle marker
		IM				= 9,	// ILS inner marker
		DMESF			= 12,	// Standalone DME or a component of NDB-DME (suppressed frequency)
		DME				= 13,	// Like DMESF, but frequency is displayed
	};

	enum class Apt
	{
		LandAirport		= 1,
		Runway			= 100,
	};

  public:
	typedef std::vector<Navaid> Navaids;

  private:
	static si::Angle::Value
	access_position (Navaid const& navaid, std::size_t const dimension);

	typedef KDTree::KDTree<2, Navaid, std::function<si::Angle::Value (Navaid const&, std::size_t)>> NavaidsTree;

  public:
	// Ctor
	explicit
	NavaidStorage (Logger const&,
				   std::string_view const& nav_file,
				   std::string_view const& fix_file,
				   std::string_view const& apt_file);

	// Dtor
	~NavaidStorage();

	/**
	 * Load navaids and fixes.
	 * Either use load() or async_loader().
	 */
	void
	load();

	/**
	 * Return a task to be run asynchronously (thread-safe) that loads the data.
	 * Either use load() or async_loader().
	 * You may use get_navs(), find_by_id() and find_by_frequency() even before async_loader() task finishes.
	 */
	std::packaged_task<void()>
	async_loader();

	/**
	 * Interrupts loading. After calling this, you can only destroy the navaid storage.
	 */
	void
	interrupt_loading()
		{ _destroying = true; }

	/**
	 * Return set of navaids withing the given @radius
	 * from a @position.
	 * \threadsafe
	 */
	Navaids
	get_navs (si::LonLat const& position, si::Length radius) const;

	/**
	 * Find navaid of given type by its @identifier.
	 * Return nullptr if not found.
	 * \threadsafe
	 */
	Navaid const*
	find_by_id (Navaid::Type, QString const& identifier) const;

	/**
	 * Return set of navaids, sorted by proximity to the @position
	 * (first is the nearest).
	 * \threadsafe
	 */
	Navaids
	find_by_frequency (si::LonLat const& position, Navaid::Type, si::Frequency frequency) const;

  private:
	void
	parse_nav_dat();

	void
	parse_fix_dat();

	void
	parse_apt_dat();

	bool
	destroying();

  private:
	std::atomic<bool>	_async_requested	{ false };
	std::atomic<bool>	_loaded				{ false };
	std::atomic<bool>	_destroying			{ false };
	std::atomic<bool>	_logged_destroying	{ false };
	Logger				_logger;
	std::string			_nav_dat_file;
	std::string			_fix_dat_file;
	std::string			_apt_dat_file;
	NavaidsTree			_navaids_tree;
	NavaidsByType		_navaids_by_type;
};


inline si::Angle::Value
NavaidStorage::access_position (Navaid const& navaid, std::size_t const dimension)
{
	return dimension == 0 ? navaid.position().lat().in<si::Degree>() : navaid.position().lon().in<si::Degree>();
}

} // namespace xf

#endif

