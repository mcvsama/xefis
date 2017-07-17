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

#ifndef XEFIS__SUPPORT__NAVIGATION__NAVAID_STORAGE_H__INCLUDED
#define XEFIS__SUPPORT__NAVIGATION__NAVAID_STORAGE_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>
#include <map>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/logger.h>
#include <kdtree++/kdtree.hpp>

// Local:
#include "navaid.h"


namespace xf {

class NavaidStorage
{
	struct Group
	{
		std::map<QString, Navaid const*>		by_identifier;
		std::multimap<Frequency, Navaid const*>	by_frequency;
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
	static Angle::Value
	access_position (Navaid const& navaid, std::size_t const dimension);

	typedef KDTree::KDTree<2, Navaid, std::function<Angle::Value (Navaid const&, std::size_t)>> NavaidsTree;

  public:
	// Ctor
	explicit
	NavaidStorage();

	// Dtor
	~NavaidStorage();

	/**
	 * Load navaids and fixes.
	 */
	void
	load();

	/**
	 * Return set of navaids withing the given @radius
	 * from a @position.
	 */
	Navaids
	get_navs (LonLat const& position, Length radius) const;

	/**
	 * Find navaid of given type by its @identifier.
	 * Return nullptr if not found.
	 */
	Navaid const*
	find_by_id (Navaid::Type, QString const& identifier) const;

	/**
	 * Return set of navaids, sorted by proximity to the @position
	 * (first is the nearest).
	 */
	Navaids
	find_by_frequency (LonLat const& position, Navaid::Type, Frequency frequency) const;

  private:
	void
	parse_nav_dat();

	void
	parse_fix_dat();

	void
	parse_apt_dat();

  private:
	Logger			_logger;
	NavaidsTree		_navaids_tree;
	const char*		_nav_dat_file	= "share/nav/nav.dat.gz"; // TODO make it configurable
	const char*		_fix_dat_file	= "share/nav/fix.dat.gz";
	const char*		_apt_dat_file	= "share/nav/apt.dat.gz";
	NavaidsByType	_navaids_by_type;
};


inline Angle::Value
NavaidStorage::access_position (Navaid const& navaid, std::size_t const dimension)
{
	return dimension == 0 ? navaid.position().lat().quantity<Degree>() : navaid.position().lon().quantity<Degree>();
}

} // namespace xf

#endif

