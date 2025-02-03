/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__PRANDTL_TUBE_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__PRANDTL_TUBE_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/observation_widget.h>

// Standard:
#include <cstddef>


namespace xf::sim {

class PrandtlTube;


class PrandtlTubeWidget: public ObservationWidget
{
  public:
	// Ctor
	explicit
	PrandtlTubeWidget (PrandtlTube&);

	// ObservationWidget API
	void
	update_observed_values (rigid_body::Body const* planet_body) override;

  private:
	PrandtlTube&		_prandtl_tube;
	std::string			_static_pressure;
	std::string			_dynamic_pressure;
	std::string			_total_pressure;
};

} // namespace xf::sim

#endif

