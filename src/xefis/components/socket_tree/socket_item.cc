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

// Local:
#include "socket_item.h"
#include "socket_tree.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/base/icons.h>
#include <xefis/core/sockets/socket_converter.h>

// Neutrino:
#include <neutrino/qt/qstring.h>

// Qt:
#include <QtWidgets/QTreeWidgetItem>
#include <QFontDatabase>

// Standard:
#include <cstddef>
#include <format>


namespace xf {

SocketItem::SocketItem (BasicSocket* socket, QTreeWidgetItem& parent):
	QTreeWidgetItem (&parent),
	_socket (socket)
{
	if (_socket)
		setText (SocketTree::UseCountColumn, QString::number (_socket->readers_count()));

	QFont const monospace_font = QFontDatabase::systemFont (QFontDatabase::FixedFont);

	for (auto const column: { 2, 3, 4 })
		setFont (column, monospace_font);
}


void
SocketItem::setup_appereance()
{
	setIcon (0, is_dir()
		? icons::socket_dir()
		: icons::socket_value());
}


void
SocketItem::read()
{
	if (_socket)
	{
		SocketConversionSettings conv_settings;
		conv_settings.numeric_format_double = "{:.12f}";
		conv_settings.preferred_units = {
			si::Celsius::dynamic_unit(),
			si::Degree::dynamic_unit(),
		};

		using neutrino::to_qstring;

		QString actual_value = to_qstring (_socket->to_string (conv_settings));
		neutrino::filter_printable_string (actual_value);

		QString set_value = to_qstring (_socket->to_string (conv_settings));
		neutrino::filter_printable_string (set_value);

		setTextAlignment (SocketTree::ActualValueColumn, Qt::AlignRight);
		setText (SocketTree::ActualValueColumn, actual_value);

		setTextAlignment (SocketTree::SetValueColumn, Qt::AlignRight);
		setText (SocketTree::SetValueColumn, set_value);

		setTextAlignment (SocketTree::FallbackValueColumn, Qt::AlignRight);
		setText (SocketTree::FallbackValueColumn, "x"); // TODO get the actual fallback value
	}
}


bool
SocketItem::operator< (QTreeWidgetItem const& other) const
{
	if (auto const* other_socket = dynamic_cast<SocketItem const*> (&other))
	{
		if (is_dir() && other_socket->is_dir())
			return text (SocketTree::NameColumn) < other.text (SocketTree::NameColumn);
		else
			return is_dir();
	}
	else
		return QTreeWidgetItem::operator< (other);
}

} // namespace xf

