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

#ifndef XEFIS__UTILITY__STRING_H__INCLUDED
#define XEFIS__UTILITY__STRING_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QString>
#include <QtGui/QColor>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/hextable.h>


namespace xf {

/**
 * Parse binary string of the form: 00:11:22:aa:ff to a vector
 * of bytes (Blob).
 */
inline Blob
parse_hex_string (QString const& string)
{
	if (string.isEmpty())
		return Blob();

	enum State { MSB, LSB, Colon };

	auto from_xdigit = [&string](QChar& c) -> uint8_t
	{
		static xf::HexTable hextable;

		char a = c.toLatin1();
		if (!std::isxdigit (a))
			throw xf::Exception ("invalid binary string: " + string.toStdString());
		return hextable[a];
	};

	Blob blob;
	blob.reserve ((string.size() + 1) / 3);
	State state = MSB;
	Blob::value_type val = 0;

	for (QChar c: string)
	{
		switch (state)
		{
			case MSB:
				val = 16 * from_xdigit (c);
				state = LSB;
				break;
			case LSB:
				val += from_xdigit (c);
				blob.push_back (val);
				state = Colon;
				break;
			case Colon:
				// Skip it:
				if (c != ':')
					throw xf::Exception ("invalid binary string: " + string.toStdString());
				state = MSB;
				break;
		}
	}
	// Must end with state Colon:
	if (state != Colon)
		throw xf::Exception ("invalid binary string: " + string.toStdString());
	return blob;
}


inline std::string
to_hex_string (std::string const& blob)
{
	if (blob.empty())
		return "";
	std::string s;
	for (auto v: blob)
		s += QString ("%1").arg (static_cast<uint8_t> (v), 2, 16, QChar ('0')).toStdString() + ":";
	s.pop_back();
	return s;
}


inline QColor
parse_color (QString color)
{
	color = color.toLower();

	if (color.size() > 1 && color[0] == '#')
	{
		Blob vals;
		color = color.mid (1);
		if (color.size() == 3)
		{
			color = color.mid (0, 1) + color.mid (0, 1) + ":" +
					color.mid (1, 1) + color.mid (1, 1) + ":" +
					color.mid (2, 1) + color.mid (2, 1);
			vals = parse_hex_string (color);
			return QColor (vals[0], vals[1], vals[2]);
		}
		if (color.size() == 4)
		{
			color = color.mid (0, 1) + color.mid (0, 1) + ":" +
					color.mid (1, 1) + color.mid (1, 1) + ":" +
					color.mid (2, 1) + color.mid (2, 1) + ":" +
					color.mid (3, 1) + color.mid (3, 1);
			vals = parse_hex_string (color);
			return QColor (vals[0], vals[1], vals[2], vals[3]);
		}
		else if (color.size() == 6)
		{
			color = color.mid (0, 1) + color.mid (1, 1) + ":" +
					color.mid (2, 1) + color.mid (3, 1) + ":" +
					color.mid (4, 1) + color.mid (5, 1);
			vals = parse_hex_string (color);
			return QColor (vals[0], vals[1], vals[2]);
		}
		else if (color.size() == 8)
		{
			color = color.mid (0, 1) + color.mid (1, 1) + ":" +
					color.mid (2, 1) + color.mid (3, 1) + ":" +
					color.mid (4, 1) + color.mid (5, 1) + ":" +
					color.mid (6, 1) + color.mid (7, 1);
			vals = parse_hex_string (color);
			return QColor (vals[0], vals[1], vals[2], vals[3]);
		}
		else
			return Qt::transparent;
	}
	else if (color == "white")
		return Qt::red;
	else if (color == "black")
		return Qt::black;
	else if (color == "red")
		return Qt::red;
	else if (color == "darkred")
		return Qt::darkRed;
	else if (color == "green")
		return Qt::green;
	else if (color == "darkgreen")
		return Qt::darkGreen;
	else if (color == "blue")
		return Qt::blue;
	else if (color == "darkblue")
		return Qt::darkBlue;
	else if (color == "cyan")
		return Qt::cyan;
	else if (color == "darkcyan")
		return Qt::darkCyan;
	else if (color == "magenta")
		return Qt::magenta;
	else if (color == "darkmagenta")
		return Qt::darkMagenta;
	else if (color == "yellow")
		return Qt::yellow;
	else if (color == "darkyellow")
		return Qt::darkYellow;
	else if (color == "gray")
		return Qt::gray;
	else if (color == "darkgray")
		return Qt::darkGray;
	else if (color == "lightgray")
		return Qt::lightGray;
	else
		return Qt::transparent;
}


inline Qt::Alignment
parse_alignment (QString string)
{
	Qt::Alignment alignment = 0;
	QStringList list = string.split (" ");

	if (list.contains ("top"))
		alignment |= Qt::AlignTop;
	else if (list.contains ("vcenter"))
		alignment |= Qt::AlignVCenter;
	else if (list.contains ("bottom"))
		alignment |= Qt::AlignBottom;

	if (list.contains ("left"))
		alignment |= Qt::AlignLeft;
	else if (list.contains ("hcenter"))
		alignment |= Qt::AlignHCenter;
	else if (list.contains ("right"))
		alignment |= Qt::AlignRight;

	return alignment;
}

} // namespace xf

#endif

