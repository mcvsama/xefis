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

#ifndef XEFIS__SUPPORT__NAVIGATION__NAVAID_H__INCLUDED
#define XEFIS__SUPPORT__NAVIGATION__NAVAID_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QString>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

class Navaid
{
  public:
	class Runway
	{
	  public:
		// Ctor
		explicit
		Runway (QString const& identifier_1, LonLat const& pos_1, QString const& identifier_2, LonLat const& pos_2) noexcept;

		/**
		 * Runway ID of the first end.
		 */
		QString const&
		identifier_1() const noexcept;

		/**
		 * Location of the first end.
		 */
		LonLat const&
		pos_1() const noexcept;

		/**
		 * Runway ID of the second end.
		 */
		QString const&
		identifier_2() const noexcept;

		/**
		 * Location of the second end.
		 */
		LonLat const&
		pos_2() const noexcept;

		/**
		 * Get width.
		 */
		Length
		width() const noexcept;

		/**
		 * Set width.
		 */
		void
		set_width (Length width) noexcept;

	  private:
		QString	_identifier_1;
		LonLat	_pos_1;
		QString	_identifier_2;
		LonLat	_pos_2;
		Length	_width;
	};

	enum Type
	{
		OTHER,
		NDB,		// NDB
		VOR,		// VOR, VOR-DME, VORTAC
		LOC,		// ILS localizer component, stand-alone localizer
		FIX,		// Fix
		DME,		// DME
		ARPT		// Land airport
	};

	enum VorType
	{
		VOROnly,	// Just VOR
		VOR_DME,	// VOR and DME
		VORTAC,		// VOR/TACAN
	};

	typedef std::vector<Runway> Runways;

  public:
	// Ctor
	explicit
	Navaid (Type);

	// Ctor
	explicit
	Navaid (Type, LonLat const&, QString const& identifier, QString const& name, Length range);

	bool
	operator< (Navaid const& other) const;

	Type
	type() const noexcept;

	LonLat const&
	position() const noexcept;

	void
	set_position (LonLat const& position) noexcept;

	QString const&
	identifier() const noexcept;

	void
	set_identifier (QString const& identifier) noexcept;

	QString const&
	name() const noexcept;

	void
	set_name (QString const& name) noexcept;

	Length
	range() const noexcept;

	void
	set_range (Length const& range) noexcept;

	void
	set_frequency (Frequency) noexcept;

	Frequency
	frequency() const noexcept;

	void
	set_slaved_variation (Angle) noexcept;

	Angle
	slaved_variation() const noexcept;

	void
	set_elevation (Length) noexcept;

	Length
	elevation() const noexcept;

	void
	set_true_bearing (Angle) noexcept;

	Angle
	true_bearing() const noexcept;

	void
	set_icao (QString const& icao);

	QString const&
	icao() const noexcept;

	void
	set_runway_id (QString const& runway_id);

	QString const&
	runway_id() const noexcept;

	/**
	 * Return appropriate identifier for displaying on HSI.
	 * This will be the identifier for VORs, DMEs, etc.
	 * and ICAO code for localisers.
	 */
	QString const&
	identifier_for_hsi() const noexcept;

	/**
	 * Return VOR subtype, if this navaid is VOR.
	 * Undefined foro non-VOR navaids.
	 */
	VorType
	vor_type() const noexcept;

	void
	set_vor_type (VorType) noexcept;

	/**
	 * Return list of runways.
	 */
	Runways const&
	runways() const noexcept;

	/**
	 * Set runways list.
	 */
	void
	set_runways (Runways const& runways);

  private:
	Type		_type;
	LonLat		_position			= { 0_deg, 0_deg };
	QString		_identifier;
	QString		_name;
	Length		_range				= 0_nmi;
	Frequency	_frequency			= 0_Hz;
	Angle		_slaved_variation	= 0_deg; // VOR only
	Length		_elevation			= 0_ft;
	Angle		_true_bearing		= 0_deg; // LOC* only
	QString		_icao;
	QString		_runway_id;
	VorType		_vor_type			= VOROnly;
	Runways		_runways; // ARPT only
};


inline
Navaid::Runway::Runway (QString const& identifier_1, LonLat const& pos_1, QString const& identifier_2, LonLat const& pos_2) noexcept:
	_identifier_1 (identifier_1),
	_pos_1 (pos_1),
	_identifier_2 (identifier_2),
	_pos_2 (pos_2)
{ }


inline QString const&
Navaid::Runway::identifier_1() const noexcept
{
	return _identifier_1;
}


inline LonLat const&
Navaid::Runway::pos_1() const noexcept
{
	return _pos_1;
}


inline QString const&
Navaid::Runway::identifier_2() const noexcept
{
	return _identifier_2;
}


inline LonLat const&
Navaid::Runway::pos_2() const noexcept
{
	return _pos_2;
}


inline Length
Navaid::Runway::width() const noexcept
{
	return _width;
}


inline void
Navaid::Runway::set_width (Length width) noexcept
{
	_width = width;
}


inline
Navaid::Navaid (Type type):
	_type (type)
{ }


inline
Navaid::Navaid (Type type, LonLat const& position, QString const& identifier, QString const& name, Length range):
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
Navaid::type() const noexcept
{
	return _type;
}


inline LonLat const&
Navaid::position() const noexcept
{
	return _position;
}


inline void
Navaid::set_position (LonLat const& position) noexcept
{
	_position = position;
}


inline QString const&
Navaid::identifier() const noexcept
{
	return _identifier;
}


inline void
Navaid::set_identifier (QString const& identifier) noexcept
{
	_identifier = identifier;
}


inline QString const&
Navaid::name() const noexcept
{
	return _name;
}


inline void
Navaid::set_name (QString const& name) noexcept
{
	_name = name;
}


inline Length
Navaid::range() const noexcept
{
	return _range;
}


inline void
Navaid::set_range (Length const& range) noexcept
{
	_range = range;
}


inline void
Navaid::set_frequency (Frequency frequency) noexcept
{
	_frequency = frequency;
}


inline Frequency
Navaid::frequency() const noexcept
{
	return _frequency;
}


inline void
Navaid::set_slaved_variation (Angle degrees) noexcept
{
	_slaved_variation = degrees;
}


inline Angle
Navaid::slaved_variation() const noexcept
{
	return _slaved_variation;
}


inline void
Navaid::set_elevation (Length elevation) noexcept
{
	_elevation = elevation;
}


inline Length
Navaid::elevation() const noexcept
{
	return _elevation;
}


inline void
Navaid::set_true_bearing (Angle bearing) noexcept
{
	_true_bearing = bearing;
}


inline Angle
Navaid::true_bearing() const noexcept
{
	return _true_bearing;
}


inline void
Navaid::set_icao (QString const& icao)
{
	_icao = icao;
}


inline QString const&
Navaid::icao() const noexcept
{
	return _icao;
}


inline void
Navaid::set_runway_id (QString const& runway_id)
{
	_runway_id = runway_id;
}


inline QString const&
Navaid::runway_id() const noexcept
{
	return _runway_id;
}


inline QString const&
Navaid::identifier_for_hsi() const noexcept
{
	if (_type == LOC)
		return icao();
	return identifier();
}


inline Navaid::VorType
Navaid::vor_type() const noexcept
{
	return _vor_type;
}


inline void
Navaid::set_vor_type (VorType vor_type) noexcept
{
	_vor_type = vor_type;
}


inline Navaid::Runways const&
Navaid::runways() const noexcept
{
	return _runways;
}


inline void
Navaid::set_runways (Runways const& runways)
{
	_runways = runways;
}

} // namespace xf

#endif

