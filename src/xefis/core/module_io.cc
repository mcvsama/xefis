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
#include <algorithm>

// Neutrino:
#include <neutrino/demangle.h>
#include <neutrino/numeric.h>
#include <neutrino/utility.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/core/setting.h>
#include <xefis/utility/types.h>

// Local:
#include "module.h"


namespace xf {

namespace module_io {

UninitializedSettings::UninitializedSettings (std::vector<BasicSetting*> settings):
	FastException (make_message (settings))
{ }


std::string
UninitializedSettings::make_message (std::vector<BasicSetting*> settings)
{
	if (settings.empty())
		return "uninitialized settings in a module";
	else
	{
		std::string result = "uninitialized setting(s) found for module-io " + identifier (*settings[0]->io()) + ": ";

		for (std::size_t i = 0; i < settings.size(); ++i)
		{
			result += settings[i]->name();

			if (i != settings.size() - 1)
				result += ", ";
		}

		return result;
	}
}

} // namespace module_io


void
ModuleIO::ProcessingLoopAPI::verify_settings()
{
	std::vector<BasicSetting*> uninitialized_settings;

	for (auto* setting: _io._registered_settings)
	{
		if (setting->required() && !*setting)
			uninitialized_settings.push_back (setting);
	}

	if (!uninitialized_settings.empty())
		throw module_io::UninitializedSettings (uninitialized_settings);

	_io.verify_settings();
}


void
ModuleIO::ProcessingLoopAPI::register_setting (BasicSetting& setting)
{
	_io._registered_settings.push_back (&setting);
}


void
ModuleIO::ProcessingLoopAPI::register_input_socket (BasicModuleIn& socket)
{
	_io._registered_input_sockets.push_back (&socket);
}


void
ModuleIO::ProcessingLoopAPI::unregister_input_socket (BasicModuleIn& socket)
{
	auto new_end = std::remove (_io._registered_input_sockets.begin(), _io._registered_input_sockets.end(), &socket);
	_io._registered_input_sockets.resize (neutrino::to_unsigned (std::distance (_io._registered_input_sockets.begin(), new_end)));
}


void
ModuleIO::ProcessingLoopAPI::register_output_socket (BasicModuleOut& socket)
{
	_io._registered_output_sockets.push_back (&socket);
}


void
ModuleIO::ProcessingLoopAPI::unregister_output_socket (BasicModuleOut& socket)
{
	auto new_end = std::remove (_io._registered_output_sockets.begin(), _io._registered_output_sockets.end(), &socket);
	_io._registered_output_sockets.resize (neutrino::to_unsigned (std::distance (_io._registered_output_sockets.begin(), new_end)));
}


Sequence<std::vector<BasicSetting*>::const_iterator>
ModuleIO::ProcessingLoopAPI::settings() const noexcept
{
	return { _io._registered_settings.begin(), _io._registered_settings.end() };
}


Sequence<std::vector<BasicModuleIn*>::const_iterator>
ModuleIO::ProcessingLoopAPI::input_sockets() const noexcept
{
	return { _io._registered_input_sockets.begin(), _io._registered_input_sockets.end() };
}


Sequence<std::vector<BasicModuleOut*>::const_iterator>
ModuleIO::ProcessingLoopAPI::output_sockets() const noexcept
{
	return { _io._registered_output_sockets.begin(), _io._registered_output_sockets.end() };
}


ModuleIO::~ModuleIO()
{
	// Operate on copies since the deregister() method modifies vectors in this class.
	for (auto* socket: clone (_registered_input_sockets))
		socket->deregister();

	for (auto* socket: clone (_registered_output_sockets))
		socket->deregister();
}


std::string
identifier (ModuleIO& io)
{
	return demangle (typeid (io)) + " of " + identifier (io.module());
}


std::string
identifier (ModuleIO* io)
{
	return io ? identifier (*io) : "(nullptr)";
}

} // namespace xf

