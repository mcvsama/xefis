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

#ifndef XEFIS__CORE__PROPERTY_PATH_H__INCLUDED
#define XEFIS__CORE__PROPERTY_PATH_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Qt:
#include <QtCore/QString>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

/**
 * Encapsulates string used as path, for better type safety.
 */
class PropertyPath
{
  public:
	// Ctor
	PropertyPath() = default;

	// Ctor
	explicit
	PropertyPath (const char* path);

	// Ctor
	explicit
	PropertyPath (std::string const& path);

	// Ctor
	explicit
	PropertyPath (QString const& path);

	// Ctor
	PropertyPath (PropertyPath const&) = default;

	// Ctor
	PropertyPath (PropertyPath&&) = default;

	PropertyPath&
	operator= (PropertyPath const&) = default;

	PropertyPath&
	operator= (PropertyPath&&) = default;

	bool
	operator== (PropertyPath const& other) const noexcept;

	/**
	 * Return string reference.
	 */
	std::string const&
	string() const noexcept;

  private:
	std::string _path;
};


inline
PropertyPath::PropertyPath (const char* path):
	_path (path)
{ }


inline
PropertyPath::PropertyPath (std::string const& path):
	_path (path)
{ }


inline
PropertyPath::PropertyPath (QString const& path):
	PropertyPath (path.toStdString())
{ }


inline bool
PropertyPath::operator== (PropertyPath const& other) const noexcept
{
	return _path == other._path;
}


inline std::string const&
PropertyPath::string() const noexcept
{
	return _path;
}

} // namespace xf

#endif

