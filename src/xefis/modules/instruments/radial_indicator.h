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

#ifndef XEFIS__MODULES__INSTRUMENTS__RADIAL_INDICATOR_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__RADIAL_INDICATOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <optional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/instrument.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_digitizer.h>
#include <xefis/core/v2/property_observer.h>

// Local:
#include "linear_indicator.h"


class RadialIndicator: BasicIndicator
{
  private:
	struct PointInfo
	{
		PointInfo (float angle, QPen const& pen, float tick_len):
			angle (angle), pen (pen), tick_len (tick_len)
		{ }

		float angle; QPen pen; float tick_len;
	};

  public:
	// Ctor
	explicit
	RadialIndicator (v2::PropertyDigitizer value_digitizer,
					 v2::PropertyDigitizer value_target_digitizer,
					 v2::PropertyDigitizer value_reference_digitizer,
					 v2::PropertyDigitizer value_automatic_digitizer,
					 std::string const& instance = {});

	// Module API
	void
	process (v2::Cycle const&) override;

  protected:
	// QWidget API
	void
	paintEvent (QPaintEvent*) override;

	void
	paint_text (float q, float r);

	void
	paint_indicator (float q, float r);

  private:
	v2::PropertyDigitizer	_value_digitizer;
	v2::PropertyDigitizer	_value_target_digitizer;
	v2::PropertyDigitizer	_value_reference_digitizer;
	v2::PropertyDigitizer	_value_automatic_digitizer;
	v2::PropertyObserver	_inputs_observer;
	std::vector<PointInfo>	_points;
};

#endif
