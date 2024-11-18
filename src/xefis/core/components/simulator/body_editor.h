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
#include <xefis/support/ui/observation_widget.h>
#include <xefis/support/ui/paint_helper.h>

// Qt:
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBox>
#include <QWidget>

// Standard:
#include <cstddef>


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
	create_position_widget();

  private:
	QVBoxLayout					_layout				{ this };
	RigidBodyViewer&			_rigid_body_viewer;
	rigid_body::Body*			_edited_body		{ nullptr };
	std::unique_ptr<ObservationWidget>
								_edited_body_widget;
	QVBoxLayout					_edited_body_widget_layout;
	QLabel*						_body_label;
	QToolBox					_tool_box;
	// Position on the planet:
	QLabel						_latitude;
	QLabel						_longitude;
	QLabel						_altitude_amsl;
	// Velocities relative to the planet:
	QLabel						_velocity;
	QLabel						_angular_velocity;
};

} // namespace xf

#endif

