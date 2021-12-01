/* vim:ts=4
 *
 * Copyleft 2021  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SOCKETS__MODULE_SOCKET_PATH_H__INCLUDED
#define XEFIS__CORE__SOCKETS__MODULE_SOCKET_PATH_H__INCLUDED

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
class ModuleSocketPath
{
  public:
	// Ctor
	ModuleSocketPath() = default;

	// Ctor
	explicit
	ModuleSocketPath (const char* path);

	// Ctor
	explicit
	ModuleSocketPath (std::string_view const& path);

	// Ctor
	explicit
	ModuleSocketPath (QString const& path);

	// Ctor
	ModuleSocketPath (ModuleSocketPath const&) = default;

	// Ctor
	ModuleSocketPath (ModuleSocketPath&&) = default;

	ModuleSocketPath&
	operator= (ModuleSocketPath const&) = default;

	ModuleSocketPath&
	operator= (ModuleSocketPath&&) = default;

	bool
	operator== (ModuleSocketPath const& other) const noexcept;

	/**
	 * Return string reference.
	 */
	std::string const&
	string() const noexcept;

  private:
	std::string _path;
};


inline
ModuleSocketPath::ModuleSocketPath (const char* path):
	_path (path)
{ }


inline
ModuleSocketPath::ModuleSocketPath (std::string_view const& path):
	_path (path)
{ }


inline
ModuleSocketPath::ModuleSocketPath (QString const& path):
	ModuleSocketPath (path.toStdString())
{ }


inline bool
ModuleSocketPath::operator== (ModuleSocketPath const& other) const noexcept
{
	return _path == other._path;
}


inline std::string const&
ModuleSocketPath::string() const noexcept
{
	return _path;
}

} // namespace xf

#endif

