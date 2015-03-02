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

#ifndef XEFIS__MODULES__LOG__KLOG_H__INCLUDED
#define XEFIS__MODULES__LOG__KLOG_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>


class KLog:
	public QObject,
	public xf::Module
{
	Q_OBJECT

	static constexpr std::size_t	BufferSize = 1024 * 1024;

  public:
	// Ctor
	KLog (xf::ModuleManager*, QDomElement const& config);

  private slots:
	void
	check_klog();

  private:
	QTimer*							_timer		= nullptr;
	bool							_open		= false;
	std::array<char, BufferSize>	_buffer;
	// Output:
	xf::PropertyBoolean				_flag_oom;
	xf::PropertyBoolean				_flag_io;
	xf::PropertyBoolean				_flag_oops;
	xf::PropertyBoolean				_flag_bug;
};

#endif
