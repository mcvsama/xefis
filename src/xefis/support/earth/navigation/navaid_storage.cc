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

// Local:
#include "navaid_storage.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/earth.h>

// Neutrino:
#include <neutrino/exception.h>
#include <neutrino/numeric.h>
#include <neutrino/qt/qzdevice.h>

// Qt:
#include <QFile>
#include <QTextStream>

// Standard:
#include <cstddef>
#include <memory>
#include <thread>


namespace xf {

class GzDataFileIteratorException: public Exception
{
  public:
	// Ctor
	explicit
	GzDataFileIteratorException(std::string const& message):
		Exception (message)
	{ }
};


class GzDataFileIterator
{
  public:
	// Ctor
	explicit
	GzDataFileIterator (std::string_view const path);

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
	QFile							_file;
	std::unique_ptr<QZDevice>		_decompressor;
	std::unique_ptr<QTextStream>	_decompressed_stream;
	std::unique_ptr<QTextStream>	_line_stream;
	QString							_line;
};


inline
GzDataFileIterator::GzDataFileIterator (std::string_view const path):
	_file (QString::fromStdString (std::string (path)))
{
	using namespace std::string_literals;

	if (_file.open (QFile::ReadOnly))
	{
		_decompressor = std::make_unique<QZDevice> (&_file);

		if (_decompressor->open (QZDevice::ReadOnly))
		{
			_decompressed_stream = std::make_unique<QTextStream> (_decompressor.get());
			// Skip two first lines (file origin and copyrights):
			operator++();
			operator++();
		}
		else
			throw GzDataFileIteratorException("could not open decompressor for file: "s + path);
	}
	else
		throw GzDataFileIteratorException("could not open file: "s + path);
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


NavaidStorage::NavaidStorage (Logger const& logger,
							  std::string_view const nav_file,
							  std::string_view const fix_file,
							  std::string_view const apt_file):
	_logger (logger.with_context ("<navaid storage>")),
	_nav_dat_file (nav_file),
	_fix_dat_file (fix_file),
	_apt_dat_file (apt_file),
	_navaids_tree (access_position)
{
	_logger << "Creating NavaidStorage" << std::endl;
}


NavaidStorage::~NavaidStorage()
{
	_destroying = true;

	if (_async_requested && !_loaded)
	{
		_logger << "Requested async load; waiting for completion." << std::endl;

		using namespace std::literals;

		while (!_loaded)
			std::this_thread::sleep_for (0.1s);
	}

	_logger << "Destroying NavaidStorage" << std::endl;
}


void
NavaidStorage::load()
{
	if (_loaded)
		return;

	parse_nav_dat();
	parse_fix_dat();
	parse_apt_dat();

	if (destroying())
		return;

	_navaids_tree.optimize();

	if (destroying())
		return;

	for (Navaid const& navaid: _navaids_tree)
	{
		auto g = _navaids_by_type.insert (std::make_pair (navaid.type(), Group())).first;
		g->second.by_identifier[navaid.identifier()] = &navaid;
		g->second.by_frequency.insert (std::make_pair (navaid.frequency(), &navaid));
	}
}


std::packaged_task<void()>
NavaidStorage::async_loader()
{
	_async_requested = true;
	return std::packaged_task<void()> ([this] {
		Exception::catch_and_log (_logger, std::bind (&NavaidStorage::load, this));
		_loaded = true;
	});
}


NavaidStorage::Navaids
NavaidStorage::get_navs (si::LonLat const& position, si::Length const radius) const
{
	if (!_loaded)
		return {};

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
	_navaids_tree.find_nearest_if (navaid_at_position, std::numeric_limits<si::Length::Value>::max(), inserter_and_predicate);

	return set;
}


Navaid const*
NavaidStorage::find_by_id (Navaid::Type type, QString const& identifier) const
{
	if (!_loaded)
		return {};

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
NavaidStorage::find_by_frequency (si::LonLat const& position, Navaid::Type type, si::Frequency const frequency) const
{
	if (!_loaded)
		return {};

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
		si::LonLat pos;
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

		pos = si::LonLat (1_deg * pos_lon, 1_deg * pos_lat);
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

		if (destroying())
			return;
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

		si::LonLat pos;
		QString identifier;
		double pos_lon;
		double pos_lat;

		line_ts >> pos_lat >> pos_lon >> identifier;

		if (pos_lat == 99.0) // EOF sentinel
			break;

		pos = si::LonLat (1_deg * pos_lon, 1_deg * pos_lat);

		_navaids_tree.insert (Navaid (Navaid::FIX, pos, identifier, identifier, 0_nmi));

		if (destroying())
			return;
	}

	_logger << "Loading fixes: done" << std::endl;
}


void
NavaidStorage::parse_apt_dat()
{
	_logger << "Loading airports" << std::endl;

	std::unique_ptr<Navaid> cur_land_airport;
	Navaid::Runways runways;
	std::size_t loaded_airports = 0;

	auto push_navaid = [&] {
		if (cur_land_airport && !runways.empty())
		{
			// Compute position:
			si::LonLat min_position = runways[0].pos_1();
			si::LonLat max_position = min_position;

			for (auto const& rwy: runways)
			{
				for (auto const& point: { rwy.pos_1(), rwy.pos_2() })
				{
					min_position.lon() = std::min (min_position.lon(), point.lon());
					min_position.lat() = std::min (min_position.lat(), point.lat());
					max_position.lon() = std::max (max_position.lon(), point.lon());
					max_position.lat() = std::max (max_position.lat(), point.lat());
				}
			}

			si::LonLat mean_position (xf::mean (min_position.lon(), max_position.lon()),
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
										   si::LonLat (1_deg * lon_deg[0], 1_deg * lat_deg[0]),
										   identifier[1],
										   si::LonLat (1_deg * lon_deg[1], 1_deg * lat_deg[1]));
					runway.set_width (1_m * width_m);
					runways.push_back (runway);
				}
			}
		}

		if (destroying())
			return;
	}

	push_navaid();

	_logger << "Loading airports: done" << std::endl;
}


bool
NavaidStorage::destroying()
{
	if (_destroying)
	{
		if (!_logged_destroying)
			_logger << "Loading navaids: interrupted" << std::endl;

		_logged_destroying = true;
		return true;
	}
	else
		return false;
}

} // namespace xf

