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

#ifndef XEFIS__CORE__NAV_H__INCLUDED
#define XEFIS__CORE__NAV_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QString>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/lonlat.h>


namespace Xefis {

class Navaid
{
  public:
	enum Type {
		OTHER	= 0,
		NDB		= 2,	// NDB
		VOR		= 3,	// VOR, VOR-DME, VORTAC
		LOC		= 4,	// ILS localizer component
		LOCSA	= 5,	// Stand-alone localiser
		GS		= 6,	// ILS glideslope component
		OM		= 7,	// ILS outer marker
		MM		= 8,	// ILS middle marker
		IM		= 9,	// ILS inner marker
		DMESF	= 12,	// Standalone DME or a component of NDB-DME (suppressed frequency)
		DME		= 13,	// Like DMESF, but frequency is displayed
		Fix		= 50,	// Fix
	};

	enum VorType {
		VOROnly	= 1,	// Just VOR
		VOR_DME	= 2,	// VOR and DME
		VORTAC	= 3,	// VOR/TACAN
	};

  public:
	Navaid (Type, LonLat const&, QString const& identifier, QString const& name, Miles range);

	bool
	operator< (Navaid const& other) const;

	Type
	type() const;

	LonLat const&
	position() const;

	QString const&
	identifier() const;

	QString const&
	name() const;

	Miles
	range() const;

	void
	set_frequency (float khz);

	float
	frequency() const;

	void
	set_slaved_variation (Degrees);

	Degrees
	slaved_variation() const;

	void
	set_amsl (Feet);

	Feet
	amsl() const;

	void
	set_true_bearing (Degrees);

	Degrees
	true_bearing() const;

	void
	set_icao (QString const& icao);

	QString const&
	icao() const;

	void
	set_runway (QString const& runway);

	QString const&
	runway() const;

	/**
	 * Return appropriate identifier for displaying on HSI.
	 * This will be the identifier for VORs, DMEs, etc.
	 * and ICAO code for localisers.
	 */
	QString const&
	identifier_for_hsi();

	/**
	 * Return VOR subtype, if this navaid is VOR.
	 * Undefined foro non-VOR navaids.
	 */
	VorType
	vor_type() const;

	void
	set_vor_type (VorType);

  private:
	Type	_type;
	LonLat	_position;
	QString	_identifier;
	QString	_name;
	Miles	_range;
	float	_frequency			= 0.f;
	Degrees	_slaved_variation	= 0.f; // VOR only
	Feet	_amsl				= 0.f;
	Degrees	_true_bearing		= 0.f; // LOC* only
	QString	_icao;
	QString	_runway;
	VorType	_vor_type;
};


inline
Navaid::Navaid (Type type, LonLat const& position, QString const& identifier, QString const& name, Miles range):
	_type (type),
	_position (position),
	_identifier (identifier),
	_name (name),
	_range (range)
{ }


inline bool
Navaid::operator< (Navaid const& other) const
{
	return std::make_tuple (_position.lat(), _position.lon())
		 < std::make_tuple (other._position.lat(), other._position.lon());
}


inline Navaid::Type
Navaid::type() const
{
	return _type;
}


inline LonLat const&
Navaid::position() const
{
	return _position;
}


inline QString const&
Navaid::identifier() const
{
	return _identifier;
}


inline QString const&
Navaid::name() const
{
	return _name;
}


inline Miles
Navaid::range() const
{
	return _range;
}


inline void
Navaid::set_frequency (float khz)
{
	_frequency = khz;
}


inline float
Navaid::frequency() const
{
	return _frequency;
}


inline void
Navaid::set_slaved_variation (Degrees degrees)
{
	_slaved_variation = degrees;
}


inline Degrees
Navaid::slaved_variation() const
{
	return _slaved_variation;
}


inline void
Navaid::set_amsl (Feet amsl)
{
	_amsl = amsl;
}


inline Feet
Navaid::amsl() const
{
	return _amsl;
}


inline void
Navaid::set_true_bearing (Degrees bearing)
{
	_true_bearing = bearing;
}


inline Degrees
Navaid::true_bearing() const
{
	return _true_bearing;
}


inline void
Navaid::set_icao (QString const& icao)
{
	_icao = icao;
}


inline QString const&
Navaid::icao() const
{
	return _icao;
}


inline void
Navaid::set_runway (QString const& runway)
{
	_runway = runway;
}


inline QString const&
Navaid::runway() const
{
	return _runway;
}


inline QString const&
Navaid::identifier_for_hsi()
{
	if (_type == LOC || _type == LOCSA)
		return icao();
	return identifier();
}


inline Navaid::VorType
Navaid::vor_type() const
{
	return _vor_type;
}


inline void
Navaid::set_vor_type (VorType vor_type)
{
	_vor_type = vor_type;
}

} // namespace Xefis

#endif

