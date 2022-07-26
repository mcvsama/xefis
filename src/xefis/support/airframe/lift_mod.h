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

#ifndef XEFIS__SUPPORT__AIRFRAME__LIFT_MOD_H__INCLUDED
#define XEFIS__SUPPORT__AIRFRAME__LIFT_MOD_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/range.h>

// Qt:
#include <QDomElement>

// Standard:
#include <cstddef>
#include <map>


namespace xf {

/**
 * Base class for Flaps and Spoilers, since they have identical API.
 * Contains a list of settings (different angles for flaps/spoilers)
 * that affect lift in some way.
 */
class LiftMod
{
  public:
	class Setting
	{
		friend class LiftMod;

	  public:
		// Ctor
		Setting();

		// Ctor
		explicit
		Setting (QDomElement const& config);

		/**
		 * Label for EFIS.
		 */
		QString const&
		label() const noexcept;

		/**
		 * Return real flaps or spoilers setting angle.
		 */
		si::Angle
		angle() const noexcept;

		/**
		 * Return range of allowed IAS speeds
		 * for this flaps or spoilers setting.
		 */
		Range<si::Speed> const&
		speed_range() const noexcept;

		/**
		 * Return AOA correction for given angle setting.
		 *
		 * Nominal critical AOA should be decreased by this amount
		 * to get critical AOA corrected for this flap or spoilers setting.
		 */
		si::Angle
		aoa_correction() const noexcept;

		/**
		 * Return lift coefficient correction for given angle setting.
		 *
		 * This lift should be added to the nominal C_L for flapless wing.
		 */
		double
		cl_correction() const noexcept;

		/**
		 * Return prev (lower angle) flaps/spoilers setting or nullptr if this is the last.
		 */
		Setting const*
		prev() const noexcept;

		/**
		 * Return next (higher angle) flaps/spoilers setting or nullptr if this is the last.
		 */
		Setting const*
		next() const noexcept;

	  private:
		void
		link (Setting const* prev, Setting const* next);

	  private:
		QString				_label				= "<none>";
		si::Angle			_angle				= 0_deg;
		Range<si::Speed>	_speed_range		= { 0_kt, 9999_kt };
		si::Angle			_aoa_correction		= 0_deg;
		double				_cl_correction		= 0.0;
		Setting const*		_next				= nullptr;
		Setting const*		_prev				= nullptr;
	};

	typedef std::map<si::Angle, Setting> Settings;

  public:
	// Ctor
	explicit
	LiftMod (QDomElement const& config);

	/**
	 * Get list of configured flaps/spoilers settings.
	 */
	Settings const&
	settings() const noexcept;

	/**
	 * Get most appropriate flaps/spoilers Setting for given
	 * flaps/spoilers angle.
	 */
	Setting const&
	find_setting (si::Angle surfaces_angle) const;

	/**
	 * Get next flaps/spoilers setting (more extended flaps/spoilers setting).
	 * Return nullptr if there's none.
	 */
	Setting const*
	next_setting (si::Angle surfaces_angle) const;

	/**
	 * Get previous flaps/spoilers setting (more retracted one).
	 * Return nullptr if there's none.
	 */
	Setting const*
	prev_setting (si::Angle surfaces_angle) const;

	/**
	 * Compute AOA correction for given surfaces angle setting
	 * (how much absolute AOA a flaps/spoilers setting adds to the current AOA).
	 * This value is interpolated.
	 */
	si::Angle
	get_aoa_correction (si::Angle surfaces_angle) const;

	/**
	 * Compute speeds range for given surfaces angle.
	 * Value is interpolated.
	 */
	Range<si::Speed>
	get_speed_range (si::Angle surfaces_angle) const;

	/**
	 * Return interator to a setting for given surfaces angle.
	 */
	Settings::const_iterator
	find_setting_iterator (si::Angle surfaces_angle) const;

  private:
	Settings	_settings;
};


inline
LiftMod::Setting::Setting()
{ }


inline QString const&
LiftMod::Setting::label() const noexcept
{
	return _label;
}


inline si::Angle
LiftMod::Setting::angle() const noexcept
{
	return _angle;
}


inline Range<si::Speed> const&
LiftMod::Setting::speed_range() const noexcept
{
	return _speed_range;
}


inline si::Angle
LiftMod::Setting::aoa_correction() const noexcept
{
	return _aoa_correction;
}


inline double
LiftMod::Setting::cl_correction() const noexcept
{
	return _cl_correction;
}


inline LiftMod::Setting const*
LiftMod::Setting::prev() const noexcept
{
	return _prev;
}


inline LiftMod::Setting const*
LiftMod::Setting::next() const noexcept
{
	return _next;
}


inline LiftMod::Settings const&
LiftMod::settings() const noexcept
{
	return _settings;
}

} // namespace xf

#endif

