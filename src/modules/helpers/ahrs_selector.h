/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__HELPERS__AHRS_SELECTOR_H__INCLUDED
#define XEFIS__MODULES__HELPERS__AHRS_SELECTOR_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


class AHRSSelector: public Xefis::Module
{
	static constexpr std::size_t MaxInputs = 2;

  public:
	// Ctor
	AHRSSelector (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	/**
	 * Set all output properties to nil.
	 */
	void
	reset_all();

	/**
	 * Copy property to the output property, if input property is fresh,
	 * or selector property is fresh (selector_fresh is true).
	 */
	template<class PropertyType>
		static void
		copy_if_different (PropertyType& input_property, PropertyType& output_property);

	static bool
	compute_disagree_flag (Xefis::PropertyAngle const& first, Xefis::PropertyAngle const& second, Angle threshold);

  private:
	Angle											_setting_pitch_disagree_threshold;
	Angle											_setting_roll_disagree_threshold;
	Angle											_setting_magnetic_heading_disagree_threshold;
	// Input:
	Xefis::PropertyInteger							_selected_input;
	std::array<Xefis::PropertyBoolean, MaxInputs>	_inputs_serviceable;
	std::array<Xefis::PropertyAngle, MaxInputs>		_inputs_orientation_pitch;
	std::array<Xefis::PropertyAngle, MaxInputs>		_inputs_orientation_roll;
	std::array<Xefis::PropertyAngle, MaxInputs>		_inputs_orientation_magnetic_heading;
	// Output:
	Xefis::PropertyBoolean							_output_serviceable;
	Xefis::PropertyAngle							_output_orientation_pitch;
	Xefis::PropertyAngle							_output_orientation_roll;
	Xefis::PropertyAngle							_output_orientation_magnetic_heading;
	Xefis::PropertyBoolean							_output_pitch_disagree_flag;
	Xefis::PropertyBoolean							_output_roll_disagree_flag;
	Xefis::PropertyBoolean							_output_magnetic_heading_disagree_flag;
	Xefis::PropertyBoolean							_output_failover_flag;
};

#endif
