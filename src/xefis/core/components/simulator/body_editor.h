/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__COMPONENTS__SIMULATOR__BODY_EDITOR_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SIMULATOR__BODY_EDITOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/ui/airfoil_spline_widget.h>

// Qt:
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBox>
#include <QWidget>

// Standard:
#include <cstddef>
#include <optional>


namespace xf {

class RigidBodyViewer;


class BodyEditor: public QWidget
{
  public:
	// Ctor
	explicit
	BodyEditor (QWidget* parent, RigidBodyViewer& viewer);

	/**
	 * Sets body to edit. Pass nullptr to disable.
	 */
	void
	edit (rigid_body::Body* body_to_edit);

	/**
	 * Update data about currently edited body.
	 */
	void
	refresh();

  private:
	[[nodiscard]]
	QWidget*
	create_basic_info_widget();

	QWidget*
	create_mass_moments_widget();

  private:
	RigidBodyViewer&					_rigid_body_viewer;
	rigid_body::Body*					_edited_body { nullptr };
	QLabel*								_body_label;
	std::optional<QFrame>				_airfoil_frame;
	std::optional<AirfoilSplineWidget>	_airfoil_spline_widget;
	std::optional<QToolBox>				_tool_box;
	std::optional<QLabel>				_translational_kinetic_energy;
	std::optional<QLabel>				_rotational_kinetic_energy;
	std::optional<QLineEdit>			_mass_value;
};

} // namespace xf

#endif

