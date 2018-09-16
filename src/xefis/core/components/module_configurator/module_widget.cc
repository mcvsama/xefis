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
#include <QTabWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/qt/ownership_breaker.h>
#include <xefis/utility/histogram.h>

// Local:
#include "module_widget.h"


namespace xf {

ModuleWidget::ModuleWidget (BasicModule& module, QWidget* parent):
	xf::Widget (parent),
	_module (module),
	_instrument (dynamic_cast<BasicInstrument*> (&_module))
{
	auto full_name_str = QString::fromStdString (identifier (module));

	QLabel* name_label = new QLabel (full_name_str.toHtmlEscaped());
	name_label->setAlignment (Qt::AlignLeft);
	QFont font = name_label->font();
	font.setPointSize (2.0 * font.pointSize());
	name_label->setFont (font);

	auto tabs = new QTabWidget (this);

	tabs->addTab (create_performance_widget(), "Performance");

	if (auto* io_base = _module.io_base())
	{
		_inputs_property_tree = new PropertyTree (this);
		_inputs_property_tree->populate (ModuleIO::ProcessingLoopAPI (*io_base).input_properties());

		_outputs_property_tree = new PropertyTree (this);
		_outputs_property_tree->populate (ModuleIO::ProcessingLoopAPI (*io_base).output_properties());

		tabs->addTab (_inputs_property_tree, "Data inputs");
		tabs->addTab (_outputs_property_tree, "Data outputs");
	}

	if (auto module_with_config_widget = dynamic_cast<BasicModule::HasConfiguratorWidget*> (&_module))
	{
		auto module_config_widget = module_with_config_widget->configurator_widget();
		tabs->addTab (new OwnershipBreaker (module_config_widget, this), "Module config");
	}

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (WidgetSpacing);
	layout->addWidget (name_label);
	layout->addWidget (tabs);

	_refresh_timer = new QTimer (this);
	_refresh_timer->setSingleShot (false);
	_refresh_timer->setInterval ((0.25_s).in<si::Millisecond>());
	QObject::connect (_refresh_timer, &QTimer::timeout, this, &ModuleWidget::refresh);
	_refresh_timer->start();
}


void
ModuleWidget::refresh()
{
	if (_processing_time_histogram)
	{
		auto const accounting_api = BasicModule::AccountingAPI (_module);
		auto const& processing_times = accounting_api.processing_times();

		_processing_time_histogram->set_data (xf::Histogram<si::Quantity<si::Millisecond>> (processing_times.begin(), processing_times.end(), 0.01_ms, 0.0_ms, 1_ms));
	}

	if (_painting_time_histogram)
	{
		auto const accounting_api = BasicInstrument::AccountingAPI (*_instrument);
		auto const& painting_times = accounting_api.painting_times();

		_painting_time_histogram->set_data (xf::Histogram<si::Quantity<si::Millisecond>> (painting_times.begin(), painting_times.end(), 1_ms, 0.0_ms, 100_ms));
	}
}


QWidget*
ModuleWidget::create_performance_widget()
{
	auto* widget = new QWidget (this);

	_processing_time_histogram = new xf::HistogramWidget (widget);
	_processing_time_histogram->setFixedSize (em_pixels (30.0), em_pixels (10.0));

	if (_instrument)
	{
		_painting_time_histogram = new xf::HistogramWidget (widget);
		_painting_time_histogram->setFixedSize (em_pixels (30.0), em_pixels (10.0));
	}

	auto layout = new QVBoxLayout (widget);
	auto histograms_layout = new QHBoxLayout();

	histograms_layout->addWidget (_processing_time_histogram);

	if (_painting_time_histogram)
		histograms_layout->addWidget (_painting_time_histogram);

	histograms_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	layout->addLayout (histograms_layout);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	return widget;
}

} // namespace xf

