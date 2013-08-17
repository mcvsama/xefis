/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/hextable.h>


namespace Xefis {

/**
 * Parse binary string of the form: 00:11:22:aa:ff to a vector
 * of bytes.
 */
inline std::vector<uint8_t>
parse_binary_string (QString const& string)
{
	typedef std::vector<uint8_t> Blob;
	enum State { MSB, LSB, Colon };

	auto from_xdigit = [&string](QChar& c) -> uint8_t
	{
		static Xefis::HexTable hextable;

		char a = c.toLatin1();
		if (!std::isxdigit (a))
			throw Xefis::Exception ("invalid binary string: " + string.toStdString());
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
					throw Xefis::Exception ("invalid binary string: " + string.toStdString());
				state = MSB;
				break;
		}
	}
	// Must end with state Colon:
	if (state != Colon)
		throw Xefis::Exception ("invalid binary string: " + string.toStdString());
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

} // namespace Xefis

#endif

