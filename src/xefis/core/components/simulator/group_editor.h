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

#ifndef XEFIS__CORE__COMPONENTS__SIMULATOR__GROUP_EDITOR_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SIMULATOR__GROUP_EDITOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/group.h>
#include <xefis/support/ui/observation_widget.h>
#include <xefis/support/ui/paint_helper.h>
#include <xefis/support/ui/rigid_body_viewer.h>

// Qt:
#include <QLabel>

// Standard:
#include <cstddef>


namespace xf {

class GroupEditor: public QWidget
{
  public:
	// Ctor
	explicit
	GroupEditor (QWidget* parent, RigidBodyViewer&);

	/**
	 * Sets group to edit. Pass nullptr to disable.
	 */
	void
	edit (rigid_body::Group*);

	/**
	 * Update data about currently edited body.
	 */
	void
	refresh();

  private:
	RigidBodyViewer&	_rigid_body_viewer;
	rigid_body::Group*	_edited_group	{ nullptr };
	std::unique_ptr<ObservationWidget>
						_edited_group_widget;
	QVBoxLayout			_edited_group_widget_layout;
	QLabel*				_group_label;
};

} // namespace xf

#endif

