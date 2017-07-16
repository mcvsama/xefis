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

#ifndef XEFIS__MODULES__INSTRUMENTS__DATATABLE_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__DATATABLE_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/instrument.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/v1/property.h>


class Datatable:
	public v1::Instrument,
	protected xf::InstrumentAids
{
	class LabelValue
	{
	  public:
		// Ctor
		LabelValue (QDomElement const& config, QColor default_label_color, QColor default_value_color);

		/**
		 * Return true if value changed.
		 */
		bool
		fresh() const;

		/**
		 * Return value to be painted.
		 */
		QString
		stringify() const;

	  public:
		QString				label;
		QColor				label_color;
		v1::GenericProperty	value;
		QColor				value_color;
		QString				nil_value;
		std::string			unit;
		std::string			format;
	};

  public:
	// Ctor
	Datatable (v1::ModuleManager*, QDomElement const& config);

	void
	data_updated() override;

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

  private:
	double					_label_font_size		= 16.0;
	double					_value_font_size		= 18.0;
	QColor					_default_label_color	= { 0xff, 0xff, 0xff };
	QColor					_default_value_color	= { 0xff, 0xff, 0xff };
	Qt::Alignment			_alignment				= Qt::AlignTop;
	std::vector<LabelValue>	_list;
};


bool
Datatable::LabelValue::fresh() const
{
	return value.fresh();
}

#endif
