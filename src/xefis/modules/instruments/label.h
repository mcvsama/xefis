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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/support/instrument/instrument_support.h>
#include <xefis/utility/types.h>

// Standard:
#include <cstddef>


// TODO handle nans
class LabelIO: public xf::Instrument
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<float>			font_scale	{ this, "font_scale", 1.0 };
	xf::Setting<QString>		label		{ this, "label" };
	xf::Setting<QColor>			color		{ this, "color", Qt::white };
	xf::Setting<Qt::Alignment>	alignment	{ this, "alignment", Qt::AlignVCenter | Qt::AlignHCenter };

  public:
	using xf::Instrument::Instrument;
};


class Label:
	public LabelIO,
	private xf::InstrumentSupport
{
  private:
	struct PaintingParams
	{
		float			font_scale;
		QString			label;
		QColor			color;
		Qt::Alignment	alignment;
	};

  public:
	// Ctor
	explicit
	Label (xf::Graphics const&, std::string_view const& instance = {});

	// Instrument API
	std::packaged_task<void()>
	paint (xf::PaintRequest) const override;

  private:
	void
	async_paint (xf::PaintRequest const&, PaintingParams const&) const;

  private:
	LabelIO& _io { *this };
};

#endif
