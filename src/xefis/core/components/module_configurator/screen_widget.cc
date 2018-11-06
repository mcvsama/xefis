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

// Qt:
#include <QBoxLayout>
#include <QGridLayout>
#include <QTabWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/module_configurator/config_widget.h>
#include <xefis/utility/histogram.h>

// Local:
#include "screen_widget.h"


namespace xf::configurator {

ScreenWidget::ScreenWidget (Screen& screen, QWidget* parent):
	ConfigWidget (parent),
	_screen (screen)
{
	auto* name_label = create_colored_strip_label (QString::fromStdString (_screen.instance()).toHtmlEscaped(), QColor (0xff, 0xaa, 0x00), Qt::AlignBottom, this);

	auto tabs = new QTabWidget (this);
	tabs->addTab (create_performance_tab(), "Performance");

	auto* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (name_label);
	layout->addItem (new QSpacerItem (0, em_pixels (0.15f), QSizePolicy::Fixed, QSizePolicy::Fixed));
	layout->addWidget (tabs);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setInterval (1000_Hz / ConfigWidget::kDataRefreshRate);
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &ScreenWidget::refresh);
	_refresh_timer->start();

	refresh();
}


void
ScreenWidget::refresh()
{
	using Milliseconds = si::Quantity<si::Millisecond>;

	// TODO multiple painting threads
}


QWidget*
ScreenWidget::create_performance_tab()
{
	auto* widget = new QWidget (this);

	// TODO multiple painting threads

	return widget;
}

} // namespace xf::configurator

