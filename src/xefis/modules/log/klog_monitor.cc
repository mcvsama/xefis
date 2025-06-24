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
#include "klog_monitor.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qdom.h>

// System:
#include <sys/klog.h>

// Standard:
#include <cstddef>
#include <algorithm>
#include <cctype>


KLogMonitor::KLogMonitor (xf::ProcessingLoop& loop, std::string_view const instance):
	KLogMonitorIO (loop, instance)
{
	_timer = new QTimer (this);
	_timer->setInterval (100);
	_timer->setSingleShot (false);
	QObject::connect (_timer, &QTimer::timeout, this, &KLogMonitor::check_klog);
	_timer->start();

	_io.flag_oom = true;
	_io.flag_io = true;
	_io.flag_oops = true;
	_io.flag_bug = true;
}


void
KLogMonitor::check_klog()
{
	const int read_all_command = 3;

	if (auto len = klogctl (read_all_command, _buffer.data(), _buffer.size()); len > 0)
	{
		std::string buffer (_buffer.data(), nu::to_unsigned (len));
		std::transform (buffer.begin(), buffer.end(), buffer.begin(), tolower);

		// Search for OOMs:
		if (buffer.contains ("oom-killer"))
			_io.flag_oom = true;

		// Search for I/O errors:
		if (buffer.contains ("I/O error"))
			_io.flag_io = true;

		// Search for Oopses:
		if (buffer.contains (" oops"))
			_io.flag_oops = true;

		// Search for BUGs:
		if (buffer.contains (" bug"))
			_io.flag_bug = true;
	}
}

