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

// Local:
#include "debug_value.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QCheckBox>
#include <QLabel>
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf {

QVBoxLayout&
get_debug_window_layout()
{
	static thread_local std::unique_ptr<QWidget> debug_widget;

	if (!debug_widget)
	{
		debug_widget = std::make_unique<QWidget>();

		auto* label = new QLabel ("Debug controls");
		auto font = label->font();
		font.setPointSize (font.pointSize() * 1.2);
		label->setFont (font);

		auto* layout = new QVBoxLayout (&*debug_widget);
		layout->addWidget (label);

		debug_widget->show();
	}

	return dynamic_cast<QVBoxLayout&> (*debug_widget->layout());
}


bool
debug_bool (std::string const& name, bool default_value, std::function<void()> const callback)
{
	struct Details
	{
		QCheckBox*	check_box;
		bool		value;
	};

	static std::map<std::string, Details> check_boxes;

	if (auto const found = check_boxes.find (name);
		found != check_boxes.end())
	{
		return found->second.value;
	}
	else
	{
		auto& details = check_boxes.emplace(
			std::make_pair (name, Details { new QCheckBox (nu::to_qstring (name)), default_value })
		).first->second;

		auto& check_box = *details.check_box;
		check_box.setChecked (details.value);

		QObject::connect (&check_box, &QCheckBox::checkStateChanged, [&details, callback] (Qt::CheckState const state) {
			details.value = state != Qt::Unchecked;

			if (callback)
				callback();
		});

		auto& window_layout = get_debug_window_layout();
		window_layout.addWidget (PaintHelper::new_hline());
		window_layout.addWidget (&check_box);

		return details.value;
	}
}

} // namespace xf

