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

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QFile>
#include <QtCore/QTextStream>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>

// Local:
#include "navaid_storage.h"


namespace Xefis {

NavaidStorage::NavaidStorage():
	_navaids_tree (access_position)
{
	parse_nav_dat();
	parse_fix_dat();
	parse_awy_dat();

	_navaids_tree.optimize();

	for (Navaid const& navaid: _navaids_tree)
	{
		auto g = _navaids_by_type.insert (std::make_pair (navaid.type(), Group())).first;
		g->second.by_identifier[navaid.identifier()] = &navaid;
		g->second.by_frequency.insert (std::make_pair (navaid.frequency(), &navaid));
	}
}


NavaidStorage::Navaids
NavaidStorage::get_navs (LonLat const& position, Length radius) const
{
	Navaids set;

	auto inserter_and_predicate = [&] (Navaid const& navaid) -> bool
	{
		if (position.haversine_earth (navaid.position()) <= radius)
		{
			set.push_back (navaid);
			return false;
		}
		return true;
	};

	Navaid navaid_at_position (Navaid::OTHER, position, "", "", 0_nm);
	_navaids_tree.find_nearest_if (navaid_at_position, std::numeric_limits<Length::ValueType>::max(), inserter_and_predicate);

	return set;
}


Navaid const*
NavaidStorage::find_by_id (Navaid::Type type, QString const& identifier) const
{
	auto g = _navaids_by_type.find (type);
	if (g != _navaids_by_type.end())
	{
		auto navaid = g->second.by_identifier.find (identifier);
		if (navaid != g->second.by_identifier.end())
			return navaid->second;
	}
	return nullptr;
}


NavaidStorage::Navaids
NavaidStorage::find_by_frequency (LonLat const& position, Navaid::Type type, Frequency frequency) const
{
	Navaids result;

	auto g = _navaids_by_type.find (type);
	if (g != _navaids_by_type.end())
	{
		auto r0 = g->second.by_frequency.lower_bound (frequency - 5_kHz);
		auto r1 = g->second.by_frequency.lower_bound (frequency + 5_kHz);
		for (auto r = r0; r != r1; ++r)
			result.push_back (*r->second);
	}

	std::sort (result.begin(), result.end(), [&](Navaid const& a, Navaid const& b) -> bool {
		return position.haversine (a.position()) < position.haversine (b.position());
	});

	return result;
}


void
NavaidStorage::parse_nav_dat()
{
	QFile file (_nav_dat_file);
	file.open (QFile::ReadOnly);
	QTextStream ts (&file);

	// Skip two first lines (file origin and Copyrights):
	ts.readLine();
	ts.readLine();

	while (!ts.atEnd())
	{
		QString line = ts.readLine();
		QTextStream line_ts (&line);

		int unused_int;
		float unused_float;

		int type_int;
		Navaid::Type type;
		LonLat pos;
		float amsl;
		float khz;
		QString identifier;
		float range;
		QString name;
		float pos_lon;
		float pos_lat;

		line_ts >> type_int >> pos_lat >> pos_lon;

		if (type_int == 99) // EOF sentinel
			break;

		pos = LonLat (1_deg * pos_lon, 1_deg * pos_lat);
		type = static_cast<Navaid::Type> (type_int);

		switch (type)
		{
			case Navaid::NDB:
			{
				line_ts >> unused_int >> unused_int >> khz >> range >> unused_float >> identifier;
				// Rest of the line is the name:
				name = line_ts.readLine();
				Navaid navaid (type, pos, identifier, name, 1_nm * range);
				navaid.set_frequency (khz * 10_kHz);
				_navaids_tree.insert (navaid);
				break;
			}

			case Navaid::VOR:
			{
				float slaved_variation_deg;
				line_ts >> amsl >> khz >> range >> slaved_variation_deg >> identifier;
				// Rest of the line is the name:
				name = line_ts.readLine();
				khz *= 10.f;

				Navaid navaid (type, pos, identifier, name, 1_nm * range);
				navaid.set_frequency (khz * 10_kHz);
				navaid.set_slaved_variation (1_deg * slaved_variation_deg);
				navaid.set_amsl (1_ft * amsl);
				if (name.endsWith ("VOR-DME"))
					navaid.set_vor_type (Navaid::VOR_DME);
				else if (name.endsWith ("VORTAC"))
					navaid.set_vor_type (Navaid::VORTAC);
				else
					navaid.set_vor_type (Navaid::VOROnly);
				_navaids_tree.insert (navaid);
				break;
			}

			case Navaid::LOC:		// ILS localizer
			case Navaid::LOCSA:		// Stand-alone localizer
			{
				float true_bearing_deg;
				QString icao;
				QString runway;
				line_ts >> amsl >> khz >> range >> true_bearing_deg >> identifier >> icao >> runway;
				// Rest of the line is the name:
				name = line_ts.readLine();
				khz *= 10.f;

				Navaid navaid (type, pos, identifier, name, 1_nm * range);
				navaid.set_frequency (khz * 10_kHz);
				navaid.set_true_bearing (1_deg * true_bearing_deg);
				navaid.set_amsl (1_ft * amsl);
				navaid.set_icao (icao);
				navaid.set_runway (runway);
				_navaids_tree.insert (navaid);
				break;
			}

			case Navaid::GS:
				break;

			case Navaid::OM:
			case Navaid::MM:
			case Navaid::IM:
				break;

			case Navaid::DMESF:		// Suppress frequency
			case Navaid::DME:		// Display frequency
				break;

			case Navaid::Fix:
				// Not in this file.
				break;

			default:
				break;
		}
	}
}


void
NavaidStorage::parse_fix_dat()
{
	QFile file (_fix_dat_file);
	file.open (QFile::ReadOnly);
	QTextStream ts (&file);

	// Skip two first lines (file origin and Copyrights):
	ts.readLine();
	ts.readLine();

	while (!ts.atEnd())
	{
		QString line = ts.readLine();
		QTextStream line_ts (&line);

		LonLat pos;
		QString identifier;
		float pos_lon;
		float pos_lat;

		line_ts >> pos_lat >> pos_lon >> identifier;

		if (pos_lat == 99.0) // EOF sentinel
			break;

		pos = LonLat (1_deg * pos_lon, 1_deg * pos_lat);

		_navaids_tree.insert (Navaid (Navaid::Fix, pos, identifier, identifier, 0_nm));
	}
}


void
NavaidStorage::parse_awy_dat()
{
}

} // namespace Xefis

