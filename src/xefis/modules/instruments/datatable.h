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
#include <optional>

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/instrument.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_stringifier.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/xefis.h>


class Datatable:
	public v2::Instrument,
	protected xf::InstrumentAids
{
	class Line
	{
	  public:
		// Ctor
		Line (std::string const& label, v2::PropertyStringifier const&);

		// Ctor
		Line (std::string const& label, v2::PropertyStringifier const&,
			  QColor label_and_value_color);

		// Ctor
		Line (std::string const& label, v2::PropertyStringifier const&,
			  std::optional<QColor> label_color,
			  std::optional<QColor> value_color);

		/**
		 * Return value to be painted.
		 */
		QString
		stringify() const;

	  public:
		std::string				label;
		QColor					label_color	{ Qt::white };
		v2::PropertyStringifier	value;
		QColor					value_color	{ Qt::white };
	};

  public:
	// Ctor
	Datatable (xf::Xefis*, std::string const& instance = {});

	/**
	 * Set font size for all labels.
	 */
	void
	set_label_font_size (xf::FontSize);

	/**
	 * Set font size for all values.
	 */
	void
	set_value_font_size (xf::FontSize);

	/**
	 * Set table alignment within widget.
	 */
	void
	set_alignment (Qt::Alignment);

	/**
	 * Add text line to the table.
	 * Forwards arguments to the Line constructor.
	 */
	template<class ...Arg>
		void
		add_line (Arg&& ...args);

	// Module API
	void
	process (v2::Cycle const&) override;

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

  private:
	xf::FontSize			_label_font_size	{ 16.0 };
	xf::FontSize			_value_font_size	{ 18.0 };
	Qt::Alignment			_alignment			{ Qt::AlignTop };
	std::vector<Line>		_list;
	v2::PropertyObserver	_change_observer;
};


template<class ...Arg>
	void
	Datatable::add_line (Arg&& ...args)
	{
		_list.emplace_back (std::forward<Arg> (args)...);
	}

#endif
