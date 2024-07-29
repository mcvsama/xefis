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

// Qt:
#include <QLabel>
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf {

class BodyEditor: public QWidget
{
  public:
	// Ctor
	explicit
	BodyEditor (QWidget* parent);

	/**
	 * Sets body to edit. Pass nullptr to disable.
	 */
	void
	edit_body (rigid_body::Body* body_to_edit);

	/**
	 * Update data about currently edited body.
	 */
	void
	refresh();

  private:
	rigid_body::Body*	_edited_body { nullptr };
	QLabel*				_body_label;
};

} // namespace xf

#endif

