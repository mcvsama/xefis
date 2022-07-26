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
#include <xefis/core/sockets/socket_converter.h>

// Qt:
#include <QtWidgets/QTreeWidgetItem>

// Standard:
#include <cstddef>


namespace xf {

SocketItem::SocketItem (BasicSocket* socket, QTreeWidgetItem& parent):
	QTreeWidgetItem (&parent),
	_socket (socket)
{
	if (_socket)
		setText (SocketTree::UseCountColumn, QString::number (_socket->use_count()));
}


void
SocketItem::setup_appereance()
{
	setIcon (0, is_dir()
		? resources::icons16::socket_dir()
		: resources::icons16::socket_value());
}


void
SocketItem::read()
{
	if (_socket)
	{
		SocketConversionSettings conv_settings;
		conv_settings.numeric_format = boost::format ("%.12f");
		conv_settings.preferred_units = {
			si::Celsius::dynamic_unit(),
			si::Degree::dynamic_unit(),
		};

		std::string const actual_value = _socket->to_string (conv_settings);
		std::string const set_value = _socket->to_string (conv_settings);

		setTextAlignment (SocketTree::ActualValueColumn, Qt::AlignRight);
		setText (SocketTree::ActualValueColumn, QString::fromStdString (actual_value));

		setTextAlignment (SocketTree::SetValueColumn, Qt::AlignRight);
		setText (SocketTree::SetValueColumn, QString::fromStdString (set_value));

		setTextAlignment (SocketTree::FallbackValueColumn, Qt::AlignRight);
		setText (SocketTree::FallbackValueColumn, "x");
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

