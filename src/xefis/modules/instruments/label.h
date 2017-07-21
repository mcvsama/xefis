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

#ifndef XEFIS__MODULES__INSTRUMENTS__LABEL_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__LABEL_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/instrument.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/types.h>


class Label:
	public v2::Instrument,
	protected xf::InstrumentAids
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<xf::FontSize>	font_size	{ this, xf::FontSize (10.0) };
	v2::Setting<QString>		label		{ this };
	v2::Setting<QColor>			color		{ this, Qt::white };
	v2::Setting<Qt::Alignment>	alignment	{ this, Qt::AlignVCenter | Qt::AlignHCenter };

  public:
	// Ctor
	explicit
	Label (std::string const& instance = {});

  protected:
	// QWidget API
	void
	resizeEvent (QResizeEvent*) override;

	// QWidget API
	void
	paintEvent (QPaintEvent*) override;
};

#endif
