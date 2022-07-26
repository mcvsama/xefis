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
#include "module_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/module_configurator/config_widget.h>
#include <xefis/support/qt/ownership_breaker.h>
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/math/histogram.h>
#include <neutrino/numeric.h>

// Qt:
#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>

// Standard:
#include <cstddef>
#include <optional>


namespace xf::configurator {

ModuleWidget::ModuleWidget (Module& module, QWidget* parent):
	ConfigWidget (parent),
	_module (module),
	_instrument (dynamic_cast<Instrument*> (&_module))
{
	auto full_name_str = QString::fromStdString (identifier (module));
	auto const ph = PaintHelper (*this, palette(), font());

	QString module_type = _instrument ? "Instrument " : "Module ";
	QColor color = _instrument ? QColor (0xff, 0x66, 0xff) : QColor (0x50, 0x79, 0xff);
	QWidget* name_label = create_colored_strip_label (module_type + full_name_str.toHtmlEscaped(), color, Qt::AlignBottom, this);

	auto tabs = new QTabWidget (this);
	tabs->addTab (create_performance_tab(), "Performance");

	_inputs_socket_tree = new SocketTree (this);
	_inputs_socket_tree->populate (Module::ModuleSocketAPI (_module).input_sockets());

	_outputs_socket_tree = new SocketTree (this);
	_outputs_socket_tree->populate (Module::ModuleSocketAPI (_module).output_sockets());

	tabs->addTab (_inputs_socket_tree, "Data inputs");
	tabs->addTab (_outputs_socket_tree, "Data outputs");

	if (auto module_with_config_widget = dynamic_cast<Module::HasConfiguratorWidget*> (&_module))
	{
		auto module_config_widget = module_with_config_widget->configurator_widget();
		tabs->addTab (new OwnershipBreaker (module_config_widget, this), "Module config");
	}

	auto* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->addWidget (name_label);
	layout->addItem (new QSpacerItem (0, ph.em_pixels (0.15f), QSizePolicy::Fixed, QSizePolicy::Fixed));
	layout->addWidget (tabs);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setInterval (1000_Hz / ConfigWidget::kDataRefreshRate);
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &ModuleWidget::refresh);
	_refresh_timer->start();

	refresh();
}


void
ModuleWidget::refresh()
{
	using Milliseconds = si::Quantity<si::Millisecond>;

	auto const processing_loop_api = Module::ProcessingLoopAPI (_module);
	auto const accounting_api = Module::AccountingAPI (_module);

	{
		auto const& samples = accounting_api.communication_times();
		bool const enabled = processing_loop_api.implements_communicate_method();

		_communication_time_group->setEnabled (enabled);

		if (!samples.empty())
		{
			auto const [range, grid_lines] = get_max_for_axis<Milliseconds> (*std::max_element (samples.begin(), samples.end()));
			xf::Histogram<Milliseconds> histogram (samples.begin(), samples.end(), range / 100, 0.0_ms, range);

			_communication_time_histogram->set_data (histogram, { accounting_api.cycle_time() });
			_communication_time_histogram->set_grid_lines (grid_lines);
			_communication_time_stats->set_data (histogram, std::make_optional<Milliseconds> (accounting_api.cycle_time()));
		}
	}

	{
		auto const& samples = accounting_api.processing_times();
		bool const enabled = processing_loop_api.implements_process_method();

		_processing_time_group->setEnabled (enabled);

		if (!samples.empty())
		{
			auto const [range, grid_lines] = get_max_for_axis<Milliseconds> (*std::max_element (samples.begin(), samples.end()));
			xf::Histogram<Milliseconds> histogram (samples.begin(), samples.end(), range / 100, 0.0_ms, range);

			_processing_time_histogram->set_data (histogram, { accounting_api.cycle_time() });
			_processing_time_histogram->set_grid_lines (grid_lines);
			_processing_time_stats->set_data (histogram, std::make_optional<Milliseconds> (accounting_api.cycle_time()));
		}
	}

	if (_painting_time_histogram)
	{
		auto const accounting_api = Instrument::AccountingAPI (*_instrument);
		auto const& samples = accounting_api.painting_times();
		auto const [range, grid_lines] = get_max_for_axis<Milliseconds> (*std::max_element (samples.begin(), samples.end()));
		xf::Histogram<Milliseconds> histogram (samples.begin(), samples.end(), range / 100, 0.0_ms, range);

		_painting_time_histogram->set_data (histogram, { accounting_api.frame_time() });
		_painting_time_histogram->set_grid_lines (grid_lines);
		_painting_time_stats->set_data (histogram, std::make_optional<Milliseconds> (accounting_api.frame_time()));
	}
}


QWidget*
ModuleWidget::create_performance_tab()
{
	auto* widget = new QWidget (this);
	QWidget* painting_time_group {};

	std::tie (_communication_time_histogram, _communication_time_stats, _communication_time_group) = create_performance_widget (widget, "HW communication time");
	std::tie (_processing_time_histogram, _processing_time_stats, _processing_time_group) = create_performance_widget (widget, "Processing time");

	if (_instrument)
		std::tie (_painting_time_histogram, _painting_time_stats, painting_time_group) = create_performance_widget (widget, "Painting time");

	auto layout = new QGridLayout (widget);
	layout->setMargin (0);
	layout->addWidget (_communication_time_group, 0, 0);
	layout->addWidget (_processing_time_group, 1, 0);

	if (painting_time_group)
		layout->addWidget (painting_time_group, 2, 0);

	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 3, 0);

	return widget;
}

} // namespace xf::configurator

