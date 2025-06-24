/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__UI__DEBUG_VALUE_H__INCLUDED
#define XEFIS__SUPPORT__UI__DEBUG_VALUE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/qt/qstring.h>
#include <neutrino/range.h>

// Qt:
#include <QSlider>
#include <QLabel>
#include <QLayout>

// Standard:
#include <cstddef>
#include <memory>


namespace xf {

/**
 * Get layout for the debug window, so that another debug-widget can be added.
 * If debug window doesn't exist, it's created.
 */
QVBoxLayout&
get_debug_window_layout();


[[nodiscard]]
bool
debug_bool (std::string const& name, bool default_value = false, std::function<void()> const callback = nullptr);


template<class Value>
	[[nodiscard]]
	inline Value
	debug_slider (std::string const& name,
				  neutrino::Range<Value> const range,
				  std::optional<Value> default_value = std::nullopt,
				  Value const step = Value (1),
				  std::function<void()> const callback = nullptr)
	{
		struct Details
		{
			QSlider*				slider;
			neutrino::Range<Value>	range;
			Value					value;
		};

		static std::map<std::string, Details> sliders;

		if (auto const found = sliders.find (name);
			found != sliders.end())
		{
			return found->second.value;
		}
		else
		{
			if (!default_value)
				default_value = range.min();

			auto& details = sliders.emplace(
				std::make_pair (name, Details { new QSlider (Qt::Horizontal), range, *default_value })
			).first->second;

			auto& slider = *details.slider;
			slider.setTickPosition (QSlider::TicksAbove);
			slider.setTracking (true);
			auto const page_step = 10 * step;
			slider.setTickInterval (range.extent() / page_step);
			slider.setPageStep (range.extent() / page_step);
			slider.setRange (range.min() / step, range.max() / step);
			slider.setValue (details.value / step);

			auto ph = PaintHelper (slider);
			slider.setMinimumWidth (ph.em_pixels_int (25));

			auto* min_label = new QLabel (neutrino::to_qstring (std::format ("{}", range.min())));
			min_label->setAlignment (Qt::AlignLeft);

			auto* title_label = new QLabel (neutrino::to_qstring (std::format ("{}: {}", name, details.value)));
			title_label->setAlignment (Qt::AlignCenter);

			auto* max_label = new QLabel (neutrino::to_qstring (std::format ("{}", range.max())));
			max_label->setAlignment (Qt::AlignRight);

			QObject::connect (&slider, &QSlider::valueChanged, [&details, name, step, title_label, callback] (int value) {
				details.value = value * step;
				title_label->setText (neutrino::to_qstring (std::format ("{}: {}", name, details.value)));

				if (callback)
					callback();
			});

			auto* layout = new QGridLayout();
			layout->addWidget (min_label, 0, 0);
			layout->addWidget (title_label, 0, 1);
			layout->addWidget (max_label, 0, 2);
			layout->addWidget (&slider, 1, 0, 1, 3);

			auto& window_layout = get_debug_window_layout();
			window_layout.addWidget (PaintHelper::new_hline());
			window_layout.addLayout (layout);

			return details.value;
		}
	}

} // namespace xf

#endif

