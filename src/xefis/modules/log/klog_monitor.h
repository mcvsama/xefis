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

#ifndef XEFIS__MODULES__LOG__KLOG_MONITOR_H__INCLUDED
#define XEFIS__MODULES__LOG__KLOG_MONITOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>

// Qt:
#include <QtCore/QTimer>

// Standard:
#include <cstddef>
#include <array>


class KLogMonitorIO: public xf::Module
{
  public:
	/*
	 * Output
	 */

	xf::ModuleOut<bool>	flag_oom	{ this, "flags/oom" };
	xf::ModuleOut<bool>	flag_io		{ this, "flags/io-error" };
	xf::ModuleOut<bool>	flag_oops	{ this, "flags/oops" };
	xf::ModuleOut<bool>	flag_bug	{ this, "flags/bug" };

  public:
	using xf::Module::Module;
};


class KLogMonitor:
	public QObject,
	public KLogMonitorIO
{
	Q_OBJECT

  private:
	static constexpr std::size_t	kBufferSize = 1024 * 1024;

  public:
	// Ctor
	explicit
	KLogMonitor (std::string_view const& instance = {});

  private slots:
	void
	check_klog();

  private:
	KLogMonitorIO&					_io			{ *this };
	QTimer*							_timer		{ nullptr };
	bool							_open		{ false };
	std::array<char, kBufferSize>	_buffer;
};

#endif
