/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__NAV_STORAGE_H__INCLUDED
#define XEFIS__CORE__NAV_STORAGE_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>
#include <map>

// Xefis:
#include <xefis/config/all.h>
#include <kdtree++/kdtree.hpp>

// Local:
#include "navaid.h"


namespace Xefis {

class NavaidStorage
{
  public:
	typedef std::set<Navaid> Navaids;
	typedef std::map<Navaid::Type, std::map<QString, Navaid const*>> NavaidsByIdentifier;

  private:
	static LonLat::ValueType::ValueType
	access_position (Navaid const& navaid, std::size_t const dimension);

	typedef KDTree::KDTree<2, Navaid, std::function<LonLat::ValueType::ValueType (Navaid const&, std::size_t)>> NavaidsTree;

  public:
	NavaidStorage();

	Navaids
	get_navs (LonLat const& position, Length radius) const;

	Navaid const*
	find_by_id (Navaid::Type, QString const& identifier) const;

  private:
	void
	parse_nav_dat();

	void
	parse_fix_dat();

	void
	parse_awy_dat();

  private:
	NavaidsTree			_navaids_tree;
	const char*			_nav_dat_file	= "share/nav/nav.dat";
	const char*			_fix_dat_file	= "share/nav/fix.dat";
	const char*			_awy_dat_file	= "share/nav/awy.dat";
	NavaidsByIdentifier	_navaids_by_identifier;
};


inline LonLat::ValueType::ValueType
NavaidStorage::access_position (Navaid const& navaid, std::size_t const dimension)
{
	return dimension == 0 ? navaid.position().lat().deg() : navaid.position().lon().deg();
}

} // namespace Xefis

#endif

