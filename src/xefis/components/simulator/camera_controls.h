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
  public:
	// Ctor
	CameraControls (RigidBodyViewer&, QWidget* parent = nullptr);

  private:
	RigidBodyViewer& _rigid_body_viewer;
};

} // namespace xf

#endif

