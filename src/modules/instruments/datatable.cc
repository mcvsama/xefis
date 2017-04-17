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

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QLayout>

// Lib:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/window.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/string.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qdom_iterator.h>

// Local:
#include "datatable.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/datatable", Datatable)


Datatable::LabelValue::LabelValue (QDomElement const& config, QColor default_label_color, QColor default_value_color):
	label_color (default_label_color),
	value_color (default_value_color),
	format ("%d")
{
	for (QDomElement const& e: xf::iterate_sub_elements (config))
	{
		if (e == "label")
		{
			if (e.hasAttribute ("color"))
				label_color = xf::parse_color (e.attribute ("color"));
			label = e.text();
		}
		else if (e == "value")
		{
			if (!e.hasAttribute ("path"))
				throw xf::MissingDomAttribute (e, "path");

			if (e.hasAttribute ("color"))
				value_color = xf::parse_color (e.attribute ("color"));

			if (e.hasAttribute ("format"))
				format = e.attribute ("format").toStdString();

			if (e.hasAttribute ("nil"))
				nil_value = e.attribute ("nil");

			value.set_path (xf::PropertyPath (e.attribute ("path")));
			unit = e.attribute ("unit").toStdString();
		}
	}
}


QString
Datatable::LabelValue::stringify() const
{
	return QString::fromStdString (value.stringify (boost::format (format), unit, nil_value.toStdString()));
}


Datatable::Datatable (xf::ModuleManager* module_manager, QDomElement const& config):
	xf::Instrument (module_manager, config),
	InstrumentAids (0.5f)
{
	QString label_color_str;
	QString value_color_str;
	QString align_str;

	parse_settings (config, {
		{ "label-color", label_color_str, false },
		{ "value-color", value_color_str, false },
		{ "label-font-size", _label_font_size, false },
		{ "value-font-size", _value_font_size, false },
		{ "align", align_str, false },
	});

	_default_label_color = xf::parse_color (label_color_str);
	_default_value_color = xf::parse_color (value_color_str);
	_alignment = xf::parse_alignment (align_str);

	for (QDomElement const& e: xf::iterate_sub_elements (config))
		if (e == "table")
			for (QDomElement const& f: xf::iterate_sub_elements (e))
				if (f == "row")
					_list.emplace_back (f, _default_label_color, _default_value_color);
}


void
Datatable::data_updated()
{
	if (std::any_of (_list.begin(), _list.end(), std::mem_fn (&LabelValue::fresh)))
		update();
}


void
Datatable::resizeEvent (QResizeEvent*)
{
	auto xw = dynamic_cast<xf::Window*> (window());
	if (xw)
		set_scaling (xw->pen_scale(), xw->font_scale());

	InstrumentAids::update_sizes (size(), window()->size());
}


void
Datatable::paintEvent (QPaintEvent*)
{
	auto painting_token = get_token (this);
	clear_background();

	QFont label_font = _font_10;
	QFont value_font = _font_10;
	label_font.setPixelSize (_label_font_size * _master_font_scale);
	value_font.setPixelSize (_value_font_size * _master_font_scale);

	double line_height = std::max (QFontMetricsF (label_font).height(), QFontMetricsF (value_font).height());
	double empty_height = height() - line_height * _list.size();

	if (_alignment & Qt::AlignVCenter)
		painter().translate (QPointF (0.0, 0.5 * empty_height));
	else if (_alignment & Qt::AlignBottom)
		painter().translate (QPointF (0.0, empty_height));

	for (std::size_t i = 0; i < _list.size(); ++i)
	{
		LabelValue const& lv = _list[i];

		QPointF left (0.0, (i + 1) * line_height);
		QPointF right (rect().width(), left.y());

		// Label:
		painter().setFont (label_font);
		painter().setPen (get_pen (lv.label_color, 1.0));
		painter().fast_draw_text (left, Qt::AlignLeft | Qt::AlignBottom, lv.label);
		// Value:
		painter().setFont (value_font);
		painter().setPen (get_pen (lv.value_color, 1.0));
		QString lv_s;
		try {
			lv_s = lv.stringify();
		}
		catch (xf::StringifyError const& exception)
		{
			painter().setPen (get_pen (Qt::red, 1.0));
			lv_s = exception.what();
		}
		catch (boost::io::bad_format_string const&)
		{
			painter().setPen (get_pen (Qt::red, 1.0));
			lv_s = "format: ill formed";
		}
		painter().fast_draw_text (right, Qt::AlignRight | Qt::AlignBottom, lv_s);
	}
}

