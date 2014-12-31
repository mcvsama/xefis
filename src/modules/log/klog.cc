/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
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
#include <cctype>

// System:
#include <sys/klog.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>

// Local:
#include "klog.h"


XEFIS_REGISTER_MODULE_CLASS ("log/klog", KLog);


KLog::KLog (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_properties (config, {
		// Output:
		{ "flag.oom", _flag_oom, true },
		{ "flag.io", _flag_io, true },
		{ "flag.oops", _flag_oops, true },
		{ "flag.bug", _flag_bug, true },
	});

	_timer = new QTimer (this);
	_timer->setInterval (100);
	_timer->setSingleShot (false);
	QObject::connect (_timer, SIGNAL (timeout()), this, SLOT (check_klog()));
	_timer->start();

	_flag_oom.set_default (false);
	_flag_io.set_default (false);
	_flag_oops.set_default (false);
	_flag_bug.set_default (false);
}


void
KLog::check_klog()
{
	const int read_all_command = 3;
	std::size_t len = klogctl (read_all_command, _buffer.data(), _buffer.size());
	std::string buffer (_buffer.data(), len);
	std::transform (buffer.begin(), buffer.end(), buffer.begin(), tolower);
	// Search for OOMs:
	if (buffer.find ("oom-killer") != std::string::npos)
		_flag_oom.write (true);
	// Search for I/O errors:
	if (buffer.find ("i/o error") != std::string::npos)
		_flag_io.write (true);
	// Search for Oopses:
	if (buffer.find (" oops") != std::string::npos)
		_flag_oops.write (true);
	// Search for BUGs:
	if (buffer.find (" bug") != std::string::npos)
		_flag_bug.write (true);
}

