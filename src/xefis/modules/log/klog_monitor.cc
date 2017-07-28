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
#include <cctype>

// System:
#include <sys/klog.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>

// Local:
#include "klog_monitor.h"


KLogMonitor::KLogMonitor (std::unique_ptr<KLogMonitorIO> module_io, std::string const& instance):
	Module (std::move (module_io), instance)
{
	_timer = new QTimer (this);
	_timer->setInterval (100);
	_timer->setSingleShot (false);
	QObject::connect (_timer, SIGNAL (timeout()), this, SLOT (check_klog()));
	_timer->start();

	io.output_flag_oom = true;
	io.output_flag_io = true;
	io.output_flag_oops = true;
	io.output_flag_bug = true;
}


void
KLogMonitor::check_klog()
{
	const int read_all_command = 3;
	std::size_t len = klogctl (read_all_command, _buffer.data(), _buffer.size());
	std::string buffer (_buffer.data(), len);
	std::transform (buffer.begin(), buffer.end(), buffer.begin(), tolower);

	// Search for OOMs:
	if (buffer.find ("oom-killer") != std::string::npos)
		io.output_flag_oom = true;

	// Search for I/O errors:
	if (buffer.find ("i/o error") != std::string::npos)
		io.output_flag_io = true;

	// Search for Oopses:
	if (buffer.find (" oops") != std::string::npos)
		io.output_flag_oops = true;

	// Search for BUGs:
	if (buffer.find (" bug") != std::string::npos)
		io.output_flag_bug = true;
}

