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

#ifndef XEFIS__UTILITY__HEXTABLE_H__INCLUDED
#define XEFIS__UTILITY__HEXTABLE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * This is a helper for decoding hex-encoded values.
 * It assumes source files are compiled using UTF-8/ASCII encoding.
 */
class HexTable
{
  public:
	HexTable();

	/**
	 * Return integer for given character c.
	 * c can be '0'..'9', 'a'..'f', 'A'..'F'.
	 */
	int operator[] (char c) const;

  private:
	int _table[256];
};


inline
HexTable::HexTable()
{
	std::fill (std::begin (_table), std::end (_table), 0);

	for (char c = '0'; c <= '9'; ++c)
		_table[static_cast<uint8_t> (c)] = c - '0';
	for (char c = 'a'; c <= 'f'; ++c)
		_table[static_cast<uint8_t> (c)] = 10 + c - 'a';
	for (char c = 'A'; c <= 'F'; ++c)
		_table[static_cast<uint8_t> (c)] = 10 + c - 'A';
}


inline int
HexTable::operator[] (char c) const
{
	return _table[static_cast<uint8_t> (c)];
}

} // namespace xf

#endif

