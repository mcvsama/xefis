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

#ifndef XEFIS__CORE__COMPONENTS__SIMULATOR__CAMERA_CONTROLS_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SIMULATOR__CAMERA_CONTROLS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/rigid_body_viewer.h>

// Qt:
#include <QDoubleSpinBox>

// Standard:
#include <cstddef>


namespace xf {

class CameraControls: public QWidget
{
  private:
	struct EarthCoordinates
	{
		SpaceLength<ECEFSpace>	ecef	{ 0_m, 0_m, 0_m };
		si::LonLatRadius<>		polar	{ 0_deg, 0_deg, 0_m };
	};

  public:
	// Ctor
	CameraControls (RigidBodyViewer&, QWidget* parent = nullptr);

	void
	set_camera_position (SpaceLength<WorldSpace> const position);

  private:
	void
	update_polar_from_ecef();

	void
	update_ecef_from_polar();

	void
	update_coordinates_from_ecef();

  private:
	RigidBodyViewer&					_rigid_body_viewer;
	std::shared_ptr<EarthCoordinates>	_coordinates;
	QDoubleSpinBox*						_ecef_x;
	QDoubleSpinBox*						_ecef_y;
	QDoubleSpinBox*						_ecef_z;
	QDoubleSpinBox*						_polar_lat;
	QDoubleSpinBox*						_polar_lon;
	QDoubleSpinBox*						_polar_radius;
};

} // namespace xf

#endif

