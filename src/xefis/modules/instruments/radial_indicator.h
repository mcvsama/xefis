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
#include <xefis/core/instrument.h>
#include <xefis/core/property.h>
#include <xefis/core/property_digitizer.h>
#include <xefis/core/property_observer.h>

// Local:
#include "linear_indicator.h"


class RadialIndicatorIO: public BasicIndicatorIO
{ };


class RadialIndicator: BasicIndicator<RadialIndicatorIO>
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
	RadialIndicator (std::unique_ptr<RadialIndicatorIO>,
					 xf::PropertyDigitizer value_digitizer,
					 xf::PropertyDigitizer value_target_digitizer,
					 xf::PropertyDigitizer value_reference_digitizer,
					 xf::PropertyDigitizer value_automatic_digitizer,
					 std::string const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

	// Instrument API
	void
	paint (xf::PaintRequest&) const override;

  protected:
	void
	paint_text (xf::InstrumentAids&, xf::InstrumentPainter&, float q, float r) const;

	void
	paint_indicator (xf::InstrumentAids&, xf::InstrumentPainter&, float q, float r) const;

  private:
	xf::PropertyDigitizer	_value_digitizer;
	xf::PropertyDigitizer	_value_target_digitizer;
	xf::PropertyDigitizer	_value_reference_digitizer;
	xf::PropertyDigitizer	_value_automatic_digitizer;
	xf::PropertyObserver	_inputs_observer;
};

#endif
