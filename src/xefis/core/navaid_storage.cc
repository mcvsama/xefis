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


NavaidStorage::NavaidStorage()
{
	parse_nav_dat();
	parse_fix_dat();
	parse_awy_dat();
}


NavaidStorage::Navaids
NavaidStorage::get_navs (LatLng const& position, Miles radius) const
{
	Navaids set;

	for (Navaid const& navaid: _navaids)
		if (haversine_nm (navaid.position(), position) <= radius)
			set.insert (navaid);

	return set;
}


void
NavaidStorage::parse_nav_dat()
{
	QFile file (_nav_dat_file);
	file.open (QFile::ReadOnly);
	QTextStream ts (&file);

	while (!ts.atEnd())
	{
		QString line = ts.readLine();
		QTextStream line_ts (&line);

		int unused_int;
		float unused_float;

		int type_int;
		Navaid::Type type;
		LatLng pos;
		Feet amsl;
		float khz;
		QString identifier;
		Miles range;
		QString name;

		line_ts >> type_int >> pos.lat() >> pos.lng();

		if (type_int == 99) // EOF sentinel
			break;

		type = static_cast<Navaid::Type> (type_int);

		switch (type)
		{
			case Navaid::NDB:
				line_ts >> unused_int >> unused_int >> khz >> range >> unused_float >> identifier >> name;
				_navaids.insert (Navaid (type, pos, identifier, name, range));
				break;

			case Navaid::VOR:
			{
				Degrees slaved_variation;
				line_ts >> amsl >> khz >> range >> slaved_variation >> identifier >> name;
				khz *= 10.f;

				Navaid navaid (type, pos, identifier, name, range);
				navaid.set_frequency (khz * 10.f);
				navaid.set_slaved_variation (slaved_variation);
				navaid.set_amsl (amsl);
				_navaids.insert (navaid);
				break;
			}

			case Navaid::LOC:		// ILS localizer
			case Navaid::LOCSA:		// Stand-alone localizer
			{
				Degrees true_bearing;
				QString icao;
				QString runway;
				line_ts >> amsl >> khz >> range >> true_bearing >> identifier >> icao >> runway >> name;
				khz *= 10.f;

				Navaid navaid (type, pos, identifier, name, range);
				navaid.set_frequency (khz * 10.f);
				navaid.set_true_bearing (true_bearing);
				navaid.set_amsl (amsl);
				navaid.set_icao (icao);
				navaid.set_runway (runway);
				_navaids.insert (navaid);
				break;
			}

			case Navaid::GS:
				// TODO
				break;

			case Navaid::OM:
			case Navaid::MM:
			case Navaid::IM:
				// TODO
				break;

			case Navaid::DMESF:		// Suppress frequency
			case Navaid::DME:		// Display frequency
				// TODO
				break;

			case Navaid::Fix:
				// Not in this file.
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

	while (!ts.atEnd())
	{
		QString line = ts.readLine();
		QTextStream line_ts (&line);

		LatLng pos;
		QString identifier;

		line_ts >> pos.lat() >> pos.lng() >> identifier;

		if (pos.lat() == 99.0) // EOF sentinel
			break;

		_navaids.insert (Navaid (Navaid::Fix, pos, identifier, identifier, 0.f));
	}
}


void
NavaidStorage::parse_awy_dat()
{
}

