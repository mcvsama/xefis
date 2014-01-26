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

#ifndef XEFIS__CORE__AIRFRAME__FLAPS_H__INCLUDED
#define XEFIS__CORE__AIRFRAME__FLAPS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/range.h>


namespace Xefis {

class Flaps
{
  public:
	class Setting
	{
		friend class Flaps;

	  public:
		// Ctor
		Setting (QDomElement const& config);

		/**
		 * Label for EFIS.
		 */
		QString const&
		label() const noexcept;

		/**
		 * Return real flap setting angle.
		 */
		Angle const&
		angle() const noexcept;

		/**
		 * Return range of allowed IAS speeds
		 * for this flap setting.
		 */
		Range<Speed> const&
		speed_range() const noexcept;

		/**
		 * Return AOA correction for given angle setting.
		 */
		Angle const&
		aoa_correction() const noexcept;

		/**
		 * Return prev (lower angle) flap setting or nullptr if this is the last.
		 */
		Setting const*
		prev() const noexcept;

		/**
		 * Return next (higher angle) flap setting or nullptr if this is the last.
		 */
		Setting const*
		next() const noexcept;

	  private:
		void
		link (Setting const* prev, Setting const* next);

	  private:
		QString			_label;
		Angle			_angle;
		Range<Speed>	_speed_range;
		Angle			_aoa_correction;
		Setting const*	_next	= nullptr;
		Setting const*	_prev	= nullptr;
	};

	typedef std::map<Angle, Setting> Settings;

  public:
	// Ctor
	Flaps (QDomElement const& config);

	/**
	 * Get list of configured flap settings.
	 */
	Settings const&
	settings() const noexcept;

	/**
	 * Get most appropriate flap Setting for given
	 * flap angle.
	 */
	Setting const&
	find_setting (Angle const& flaps_angle) const;

	/**
	 * Get next flap setting (more extended flap setting).
	 * Return nullptr if there's none.
	 */
	Setting const*
	next_setting (Angle const& flaps_angle) const;

	/**
	 * Get previous flap setting (more retracted one).
	 * Return nullptr if there's none.
	 */
	Setting const*
	prev_setting (Angle const& flaps_angle) const;

	/**
	 * Compute AOA correction for given flaps angle setting
	 * (how much absolute AOA a flap setting adds to the current AOA).
	 * This value is interpolated.
	 */
	Angle
	get_aoa_correction (Angle const& flaps_angle) const;

	/**
	 * Compute speeds range for given flaps angle.
	 * Value is interpolated.
	 */
	Range<Speed>
	get_speed_range (Angle const& flaps_angle) const;

	/**
	 * Return interator to a setting for given flaps angle.
	 */
	Settings::const_iterator
	find_setting_iterator (Angle const& flaps_angle) const;

  private:
	Settings	_settings;
};


inline QString const&
Flaps::Setting::label() const noexcept
{
	return _label;
}


inline Angle const&
Flaps::Setting::angle() const noexcept
{
	return _angle;
}


inline Range<Speed> const&
Flaps::Setting::speed_range() const noexcept
{
	return _speed_range;
}


inline Angle const&
Flaps::Setting::aoa_correction() const noexcept
{
	return _aoa_correction;
}


inline Flaps::Setting const*
Flaps::Setting::prev() const noexcept
{
	return _prev;
}


inline Flaps::Setting const*
Flaps::Setting::next() const noexcept
{
	return _next;
}


inline Flaps::Settings const&
Flaps::settings() const noexcept
{
	return _settings;
}

} // namespace Xefis

#endif

