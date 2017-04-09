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

#ifndef XEFIS__MODULES__LOG__KLOG_H__INCLUDED
#define XEFIS__MODULES__LOG__KLOG_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>


class KLogMonitor:
	public QObject,
	public v2::Module
{
	Q_OBJECT

  public:
	/*
	 * Output
	 */

	v2::PropertyOut<bool>	output_flag_oom		{ this, "/flags/oom" };
	v2::PropertyOut<bool>	output_flag_io		{ this, "/flags/io-error" };
	v2::PropertyOut<bool>	output_flag_oops	{ this, "/flags/oops" };
	v2::PropertyOut<bool>	output_flag_bug		{ this, "/flags/bug" };

  private:
	static constexpr std::size_t	BufferSize = 1024 * 1024;

  public:
	// Ctor
	explicit
	KLogMonitor (std::string const& instance = {});

  private slots:
	void
	check_klog();

  private:
	QTimer*							_timer		{ nullptr };
	bool							_open		{ false };
	std::array<char, BufferSize>	_buffer;
};

#endif
