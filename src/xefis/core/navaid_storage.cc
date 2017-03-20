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

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QFile>
#include <QtCore/QTextStream>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/navigation/earth.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/qzdevice.h>

// Local:
#include "navaid_storage.h"


namespace xf {

class GzDataFileIterator
{
  public:
	// Ctor
	explicit
	GzDataFileIterator (QString const& path);

	/**
	 * Return true if pointer doesn't point to the end of the line.
	 */
	operator bool() const;

	/**
	 * Go to the next line.
	 */
	void
	operator++();

	/**
	 * Return QTextStream made of the line content.
	 */
	QTextStream&
	operator*();

  private:
	QFile				_file;
	Unique<QZDevice>	_decompressor;
	Unique<QTextStream>	_decompressed_stream;
	Unique<QTextStream>	_line_stream;
	QString				_line;
};


inline
GzDataFileIterator::GzDataFileIterator (QString const& path):
	_file (path)
{
	_file.open (QFile::ReadOnly);
	_decompressor = std::make_unique<QZDevice> (&_file);
	_decompressor->open (QZDevice::ReadOnly);
	_decompressed_stream = std::make_unique<QTextStream> (_decompressor.get());
	// Skip two first lines (file origin and copyrights):
	operator++();
	operator++();
}


inline
GzDataFileIterator::operator bool() const
{
	return !_line.simplified().isEmpty() || !_decompressed_stream->atEnd();
}


void
GzDataFileIterator::operator++()
{
	_line.clear();
	while (_line.simplified().isEmpty() && !_decompressed_stream->atEnd())
		_line = _decompressed_stream->readLine();
	_line_stream = std::make_unique<QTextStream> (&_line);
}


QTextStream&
GzDataFileIterator::operator*()
{
	return *_line_stream;
}


NavaidStorage::NavaidStorage():
	_navaids_tree (access_position)
{
	_logger.set_prefix ("<navaid storage>");
	_logger << "Creating NavaidStorage" << std::endl;
}


NavaidStorage::~NavaidStorage()
{
	_logger << "Destroying NavaidStorage" << std::endl;
}


void
NavaidStorage::load()
{
	parse_nav_dat();
	parse_fix_dat();
	parse_apt_dat();

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
		if (xf::haversine_earth (position, navaid.position()) <= radius)
		{
			set.push_back (navaid);
			return false;
		}
		return true;
	};

	Navaid navaid_at_position (Navaid::OTHER, position, "", "", 0_nmi);
	_navaids_tree.find_nearest_if (navaid_at_position, std::numeric_limits<Length::Value>::max(), inserter_and_predicate);

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
		return xf::haversine (position, a.position()) < xf::haversine (position, b.position());
	});

	return result;
}


void
NavaidStorage::parse_nav_dat()
{
	_logger << "Loading navaids" << std::endl;

	for (GzDataFileIterator line (_nav_dat_file); line; ++line)
	{
		auto& line_ts = *line;

		int type_int;
		Nav type;
		LonLat pos;
		double elevation_ft;
		double khz;
		QString identifier;
		double range;
		QString name;
		double pos_lon;
		double pos_lat;

		line_ts >> type_int >> pos_lat >> pos_lon;

		if (type_int == 99) // EOF sentinel
			break;

		pos = LonLat (1_deg * pos_lon, 1_deg * pos_lat);
		type = static_cast<Nav> (type_int);

		switch (type)
		{
			case Nav::NDB:
			{
				int unused_int;
				double unused_float;
				line_ts >> unused_int >> unused_int >> khz >> range >> unused_float >> identifier;
				// Rest of the line is the name:
				name = line_ts.readLine();
				Navaid navaid (Navaid::NDB, pos, identifier, name, 1_nmi * range);
				navaid.set_frequency (khz * 10_kHz);
				_navaids_tree.insert (navaid);
				break;
			}

			case Nav::VOR:
			{
				double slaved_variation_deg;
				line_ts >> elevation_ft >> khz >> range >> slaved_variation_deg >> identifier;
				// Rest of the line is the name:
				name = line_ts.readLine();
				khz *= 10.f;

				Navaid navaid (Navaid::VOR, pos, identifier, name, 1_nmi * range);
				navaid.set_frequency (khz * 10_kHz);
				navaid.set_slaved_variation (1_deg * slaved_variation_deg);
				navaid.set_elevation (1_ft * elevation_ft);
				if (name.endsWith ("VOR-DME"))
					navaid.set_vor_type (Navaid::VOR_DME);
				else if (name.endsWith ("VORTAC"))
					navaid.set_vor_type (Navaid::VORTAC);
				else
					navaid.set_vor_type (Navaid::VOROnly);
				_navaids_tree.insert (navaid);
				break;
			}

			case Nav::LOC:		// ILS localizer
			case Nav::LOCSA:	// Stand-alone localizer
			{
				double true_bearing_deg;
				QString icao;
				QString runway_id;
				line_ts >> elevation_ft >> khz >> range >> true_bearing_deg >> identifier >> icao >> runway_id;
				// Rest of the line is the name:
				name = line_ts.readLine();
				khz *= 10.f;

				Navaid navaid (Navaid::LOC, pos, identifier, name, 1_nmi * range);
				navaid.set_frequency (khz * 10_kHz);
				navaid.set_true_bearing (1_deg * true_bearing_deg);
				navaid.set_elevation (1_ft * elevation_ft);
				navaid.set_icao (icao);
				navaid.set_runway_id (runway_id);
				_navaids_tree.insert (navaid);
				break;
			}

			case Nav::GS:
				break;

			case Nav::OM:
			case Nav::MM:
			case Nav::IM:
				break;

			case Nav::DMESF:	// Suppress frequency
			case Nav::DME:		// Display frequency
				break;

			default:
				break;
		}
	}

	_logger << "Loading navaids: done" << std::endl;
}


void
NavaidStorage::parse_fix_dat()
{
	_logger << "Loading fixes" << std::endl;

	for (GzDataFileIterator line (_fix_dat_file); line; ++line)
	{
		auto& line_ts = *line;

		LonLat pos;
		QString identifier;
		double pos_lon;
		double pos_lat;

		line_ts >> pos_lat >> pos_lon >> identifier;

		if (pos_lat == 99.0) // EOF sentinel
			break;

		pos = LonLat (1_deg * pos_lon, 1_deg * pos_lat);

		_navaids_tree.insert (Navaid (Navaid::FIX, pos, identifier, identifier, 0_nmi));
	}

	_logger << "Loading fixes: done" << std::endl;
}


void
NavaidStorage::parse_apt_dat()
{
	_logger << "Loading airports" << std::endl;

	Unique<Navaid> cur_land_airport;
	Navaid::Runways runways;
	std::size_t loaded_airports = 0;

	auto push_navaid = [&] {
		if (cur_land_airport && !runways.empty())
		{
			// Compute position:
			LonLat min_position = runways[0].pos_1();
			LonLat max_position = min_position;
			for (auto rwy: runways)
			{
				for (auto point: { rwy.pos_1(), rwy.pos_2() })
				{
					min_position.lon() = std::min (min_position.lon(), point.lon());
					min_position.lat() = std::min (min_position.lat(), point.lat());
					max_position.lon() = std::max (max_position.lon(), point.lon());
					max_position.lat() = std::max (max_position.lat(), point.lat());
				}
			}
			LonLat mean_position (xf::mean (min_position.lon(), max_position.lon()),
								  xf::mean (min_position.lat(), max_position.lat()));
			cur_land_airport->set_position (mean_position);
			cur_land_airport->set_runways (runways);

			_navaids_tree.insert (*cur_land_airport);
			cur_land_airport.reset();
			runways.clear();

			++loaded_airports;
		}
	};

	for (GzDataFileIterator line (_apt_dat_file); line; ++line)
	{
		auto& line_ts = *line;

		int type;
		line_ts >> type;

		if (type == 99) // EOF sentinel
			break;

		switch (static_cast<Apt> (type))
		{
			case Apt::LandAirport:
			{
				push_navaid();

				int elevation_ft;
				int twr;
				int deprecated;
				QString identifier;
				QString name;

				line_ts >> elevation_ft >> twr >> deprecated >> identifier;
				name = line_ts.readAll();

				cur_land_airport = std::make_unique<Navaid> (Navaid::ARPT);
				cur_land_airport->set_identifier (identifier);
				cur_land_airport->set_name (name);
				cur_land_airport->set_elevation (1_ft * elevation_ft);
				break;
			}

			case Apt::Runway:
			{
				if (cur_land_airport)
				{
					double width_m;
					int runway_surface_type;
					int shoulder_surface_type;
					double smoothness;
					int center_line_lights;
					int edge_lights;
					int distance_remaining_lights;
					QString identifier[2];
					double lat_deg[2];
					double lon_deg[2];
					double displaced_threshold_m[2];
					double blast_pad_length_m[2];
					int runway_markings[2]; // Visual, non-precision, precision
					int approach_lighting[2];
					int touchdown_zone_lighting[2]; // Flag (true/false)
					int runway_end_identifier_lights[2];

					line_ts >> width_m >> runway_surface_type >> shoulder_surface_type >> smoothness >> center_line_lights
							>> edge_lights >> distance_remaining_lights;
					for (int i = 0; i < 2; ++i)
						line_ts >> identifier[i] >> lat_deg[i] >> lon_deg[i] >> displaced_threshold_m[i] >> blast_pad_length_m[i]
								>> runway_markings[i] >> approach_lighting[i] >> touchdown_zone_lighting[i] >> runway_end_identifier_lights[i];

					Navaid::Runway runway (identifier[0],
										   LonLat (1_deg * lon_deg[0], 1_deg * lat_deg[0]),
										   identifier[1],
										   LonLat (1_deg * lon_deg[1], 1_deg * lat_deg[1]));
					runway.set_width (1_m * width_m);
					runways.push_back (runway);
				}
			}
		}
	}

	push_navaid();

	_logger << "Loading airports: done" << std::endl;
}

} // namespace xf

