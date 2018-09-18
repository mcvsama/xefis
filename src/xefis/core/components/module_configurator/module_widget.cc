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
#include <QGroupBox>
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
	font.setPointSize (1.4 * font.pointSize());
	name_label->setFont (font);

	auto tabs = new QTabWidget (this);

	tabs->addTab (create_performance_tab(), "Performance");

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
		xf::Histogram<si::Quantity<si::Millisecond>> histogram (processing_times.begin(), processing_times.end(), 0.01_ms, 0.0_ms, 1_ms);

		_processing_time_histogram->set_data (histogram, { accounting_api.cycle_time() });
		_processing_time_stats->set_data (histogram);
	}

	if (_painting_time_histogram)
	{
		auto const accounting_api = BasicInstrument::AccountingAPI (*_instrument);
		auto const& painting_times = accounting_api.painting_times();
		xf::Histogram<si::Quantity<si::Millisecond>> histogram (painting_times.begin(), painting_times.end(), 1_ms, 0.0_ms, 100_ms);

		_painting_time_histogram->set_data (histogram, { accounting_api.frame_time() });
		_painting_time_stats->set_data (histogram);
	}
}


QWidget*
ModuleWidget::create_performance_tab()
{
	auto* widget = new QWidget (this);
	QWidget* processing_time_group {};
	QWidget* painting_time_group {};

	std::tie (_processing_time_histogram, _processing_time_stats, processing_time_group) = create_performance_widget (widget, "Processing time");

	if (_instrument)
		std::tie (_painting_time_histogram, _painting_time_stats, painting_time_group) = create_performance_widget (widget, "Painting time");

	auto layout = new QVBoxLayout (widget);
	auto histograms_layout = new QGridLayout();
	histograms_layout->addWidget (processing_time_group, 0, 0);

	if (painting_time_group)
		histograms_layout->addWidget (painting_time_group, 1, 0);

	histograms_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
	layout->addLayout (histograms_layout);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	return widget;
}


std::tuple<xf::HistogramWidget*, xf::HistogramStatsWidget*, QWidget*>
ModuleWidget::create_performance_widget (QWidget* parent, QString const& title)
{
	QMargins const margins (em_pixels (0.5f), em_pixels (0.25f), em_pixels (0.5f), em_pixels (0.25f));

	auto* group_box = new QGroupBox (title, parent);
	group_box->setFixedSize (em_pixels (40.0), em_pixels (15.0));

	auto* histogram_widget = new xf::HistogramWidget (group_box);
	histogram_widget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

	auto* stats_widget = new xf::HistogramStatsWidget (group_box);

	auto* group_layout = new QVBoxLayout (group_box);
	group_layout->addWidget (histogram_widget);
	group_layout->addWidget (stats_widget);
	group_layout->setContentsMargins (margins);

	return { histogram_widget, stats_widget, group_box };
}

} // namespace xf

