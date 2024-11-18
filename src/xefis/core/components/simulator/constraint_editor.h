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

#ifndef XEFIS__CORE__COMPONENTS__SIMULATOR__CONSTRAINT_EDITOR_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SIMULATOR__CONSTRAINT_EDITOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/constraint.h>
#include <xefis/support/ui/observation_widget.h>

// Qt:
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QToolBox>
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf {

class ConstraintEditor: public QWidget
{
  public:
	// Ctor
	explicit
	ConstraintEditor (QWidget* parent);

	/**
	 * Sets constraint to edit. Pass nullptr to disable.
	 */
	void
	edit (rigid_body::Constraint* constraint_to_edit);

	/**
	 * Update data about currently edited constraint.
	 */
	void
	refresh();

  private:
	QWidget*
	create_basic_info_widget();

  private:
	rigid_body::Constraint*		_edited_constraint { nullptr };
	std::unique_ptr<ObservationWidget>
								_edited_constraint_widget;
	QLabel*						_constraint_label;
};

} // namespace xf

#endif

