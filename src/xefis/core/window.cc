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
#include <array>

// Qt:
#include <QtWidgets/QShortcut>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QLabel>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/configurator/configurator_widget.h>
#include <xefis/core/module.h>
#include <xefis/core/panel.h>
#include <xefis/core/config_reader.h>
#include <xefis/core/stdexcept.h>
#include <xefis/support/ui/widgets/panel_button.h>
#include <xefis/support/ui/widgets/panel_rotary_encoder.h>
#include <xefis/support/ui/widgets/panel_numeric_display.h>
#include <xefis/support/ui/widgets/group_box.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "window.h"


namespace xf {

Window::Stack::Stack (Time delay)
{
	_black_widget = std::make_unique<QWidget>();

	layout = std::make_unique<QStackedLayout>();
	layout->addWidget (_black_widget.get());

	// This delay is here, because it just looks good:
	timer = std::make_unique<QTimer>();
	timer->setSingleShot (true);
	timer->setInterval (delay.quantity<Millisecond>());

	QObject::connect (timer.get(), &QTimer::timeout, [&] {
		layout->setCurrentIndex (*property + 1);
	});
}


Window::InstrumentDecorator::InstrumentDecorator (QWidget* parent):
	QWidget (parent)
{
	_layout = new QHBoxLayout (this);
	_layout->setMargin (0);
	_layout->setSpacing (0);
}


void
Window::InstrumentDecorator::set_instrument (Instrument * instrument)
{
	_instrument = instrument;
	_layout->addWidget (instrument);
}


Window::Window (Xefis* xefis, ConfigReader* config_reader, QDomElement const& element):
	_xefis (xefis),
	_config_reader (config_reader)
{
	try {
		setWindowTitle ("XEFIS");
		float ws = config_reader->windows_scale();
		resize (clamped<int> (ws * element.attribute ("width").toFloat(), 40, 10000),
				clamped<int> (ws * element.attribute ("height").toFloat(), 30, 10000));
		setMouseTracking (true);
		setAttribute (Qt::WA_TransparentForMouseEvents);

		if (element.attribute ("full-screen") == "true")
			setWindowState (windowState() | Qt::WindowFullScreen);

		_stack = new QStackedWidget (this);

		_instruments_panel = new QWidget (_stack);
		_instruments_panel->setBackgroundRole (QPalette::Shadow);
		_instruments_panel->setAutoFillBackground (true);
		// Black background:
		QPalette p = palette();
		p.setColor (QPalette::Shadow, Qt::black);
		p.setColor (QPalette::Dark, Qt::gray);
		_instruments_panel->setPalette (p);

		_configurator_panel = new QWidget (this);

		QLayout* configurator_layout = new QVBoxLayout (_configurator_panel);
		configurator_layout->setMargin (WidgetMargin);
		configurator_layout->setSpacing (0);

		QBoxLayout* layout = new QVBoxLayout (this);
		layout->setMargin (0);
		layout->setSpacing (0);
		layout->addWidget (_stack);

		_stack->addWidget (_instruments_panel);
		_stack->addWidget (_configurator_panel);
		_stack->setCurrentWidget (_instruments_panel);

		new QShortcut (Qt::Key_Escape, this, SLOT (show_configurator()));

		process_window_element (element);
	}
	catch (...)
	{
		unparent_modules();
		throw;
	}
}


Window::~Window()
{
	unparent_modules();
}


void
Window::data_updated (Time const&)
{
	for (auto stack: _stacks)
	{
		if (stack->property.valid() && stack->property.fresh())
		{
			// Unfresh it:
			stack->property.read();
			// First, set black empty widget (the first one).
			// Then start a timer to show requested layout after a delay.
			stack->layout->setCurrentIndex (0);
			stack->timer->start();
		}
	}
}


float
Window::pen_scale() const
{
	return _xefis->config_reader()->pen_scale();
}


float
Window::font_scale() const
{
	return _xefis->config_reader()->font_scale();
}


Window::InstrumentDecorator*
Window::get_decorator_for (Module::Pointer const& ptr) const
{
	auto decorator = _decorators.find (ptr);
	if (decorator != _decorators.end())
		return decorator->second;
	return nullptr;
}


void
Window::unparent_modules()
{
	// Since children of type Module are managed by the ModuleManager, we must not delete them.
	// Find them and reparent to 0.
	for (auto child: findChildren<QWidget*>())
		if (dynamic_cast<Module*> (child))
			child->setParent (nullptr);
}


void
Window::process_window_element (QDomElement const& window_element)
{
	for (QDomElement& e: window_element)
	{
		if (e == "layout")
		{
			QLayout* layout = process_layout_element (e, _instruments_panel, nullptr);
			if (_instruments_panel->layout())
				throw BadConfiguration ("a window can only have one layout");
			_instruments_panel->setLayout (layout);
		}
		else
			throw BadDomElement (e);
	}
}


Panel*
Window::process_panel_element (QDomElement const& panel_element, QWidget* parent_widget)
{
	Panel* panel = new Panel (parent_widget, _xefis);

	for (QDomElement& e: panel_element)
	{
		if (e == "layout")
		{
			QLayout* layout = process_layout_element (e, panel, panel);
			layout->setMargin (5);
			if (panel->layout())
				throw BadConfiguration ("a panel can only have one layout");
			panel->setLayout (layout);
		}
		else
			throw BadDomElement (e);
	}

	return panel;
}


QWidget*
Window::process_group_element (QDomElement const& group_element, QWidget* parent_widget, Panel* panel)
{
	QWidget* group = nullptr;
	QString label = group_element.attribute ("label");
	bool is_q = false;

	// @padding
	std::array<int, 4> padding = parse_padding (group_element, "padding");

	// Two types of groups: different for instruments and for panels:
	if (panel)
	{
		is_q = true;
		group = new QGroupBox (label.replace ("&", "&&"), parent_widget);
	}
	else
	{
		GroupBox* group_box = new GroupBox (label, parent_widget);
		group_box->set_padding (padding);
		group = group_box;
	}

	// Children of <group>:
	for (QDomElement& e: group_element)
	{
		if (e == "layout")
		{
			QLayout* layout = process_layout_element (e, group, panel);

			// Multiple layouts?
			if (group->layout())
				throw BadConfiguration ("a group can only have one layout");

			// Configure layout:
			if (is_q)
				layout->setMargin (5);
			else
			{
				layout->setMargin (0);
			}
			group->setLayout (layout);
		}
		else
			throw BadDomElement (e);
	}

	return group;
}


QWidget*
Window::process_widget_element (QDomElement const& widget_element, QWidget* parent_widget, Panel* panel)
{
	if (!panel)
		throw BadDomElement (widget_element, "<widget> can only be used as descendant of <panel>");

	QString type = widget_element.attribute ("type");
	QWidget* widget = nullptr;
	QWidget* label_wrapper = nullptr;
	QLabel* top_label = nullptr;
	QLabel* bottom_label = nullptr;

	QString top_label_str = widget_element.attribute ("top-label").toHtmlEscaped();
	QString bottom_label_str = widget_element.attribute ("bottom-label").toHtmlEscaped();

	if (!top_label_str.isEmpty() || !bottom_label_str.isEmpty())
	{
		label_wrapper = new PanelWidget (parent_widget, panel);

		if (!top_label_str.isEmpty())
		{
			top_label = new QLabel (top_label_str, label_wrapper);
			top_label->setTextFormat (Qt::PlainText);
			top_label->setAlignment (Qt::AlignHCenter | Qt::AlignBottom);
			top_label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		}

		if (!bottom_label_str.isEmpty())
		{
			bottom_label = new QLabel (bottom_label_str, label_wrapper);
			bottom_label->setTextFormat (Qt::PlainText);
			bottom_label->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
			bottom_label->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		}
	}

	if (type == "button")
	{
		PanelButton::LEDColor color = PanelButton::Green;
		QString color_attr = widget_element.attribute ("led-color");
		if (color_attr == "amber")
			color = PanelButton::Amber;
		else if (color_attr == "red")
			color = PanelButton::Red;
		else if (color_attr == "white")
			color = PanelButton::White;
		else if (color_attr == "blue")
			color = PanelButton::Blue;

		widget = new PanelButton (parent_widget, panel, color,
								  PropertyBoolean (PropertyPath (widget_element.attribute ("click-property"))),
								  PropertyBoolean (PropertyPath (widget_element.attribute ("toggle-property"))),
								  PropertyBoolean (PropertyPath (widget_element.attribute ("led-property"))));
	}
	else if (type == "rotary-encoder")
	{
		widget = new PanelRotaryEncoder (parent_widget, panel,
										 widget_element.attribute ("click-label"),
										 PropertyInteger (PropertyPath (widget_element.attribute ("value-property"))),
										 PropertyBoolean (PropertyPath (widget_element.attribute ("click-property"))));
	}
	else if (type == "numeric-display")
	{
		if (widget_element.hasAttribute ("format-property"))
			widget = new PanelNumericDisplay (parent_widget, panel,
											  widget_element.attribute ("digits").toUInt(),
											  widget_element.attribute ("unit").toStdString(),
											  PropertyPath (widget_element.attribute ("value-property")),
											  PropertyString (PropertyPath (widget_element.attribute ("format-property"))));
		else
			widget = new PanelNumericDisplay (parent_widget, panel,
											  widget_element.attribute ("digits").toUInt(),
											  widget_element.attribute ("unit").toStdString(),
											  PropertyPath (widget_element.attribute ("value-property")),
											  widget_element.attribute ("format").toStdString());
	}
	else
		throw BadDomAttribute (widget_element, "type");

	// Is widget disabled?
	widget->setEnabled (widget_element.tagName() != "disabled-widget");

	// Comment?
	if (widget_element.hasAttribute ("comment"))
		widget->setToolTip (widget_element.attribute ("comment").toHtmlEscaped());

	if (label_wrapper)
	{
		QVBoxLayout* layout = new QVBoxLayout (label_wrapper);
		layout->setMargin (0);
		layout->setSpacing (2);
		layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
		if (top_label)
			layout->addWidget (top_label, 0, Qt::AlignCenter);
		layout->addWidget (widget, 0, Qt::AlignCenter);
		if (bottom_label)
			layout->addWidget (bottom_label, 0, Qt::AlignCenter);
		layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
		return label_wrapper;
	}
	else
		return widget;
}


QLayout*
Window::process_layout_element (QDomElement const& layout_element, QWidget* parent_widget, Panel* panel)
{
	QLayout* new_layout = nullptr;
	QBoxLayout* box_new_layout = nullptr;
	Shared<Stack> stack;

	QString type = layout_element.attribute ("type");
	unsigned int spacing = layout_element.attribute ("spacing").toUInt();

	if (type == "horizontal")
		new_layout = box_new_layout = new QHBoxLayout();
	else if (type == "vertical")
		new_layout = box_new_layout = new QVBoxLayout();
	else if (type == "stack")
	{
		if (!layout_element.hasAttribute ("path"))
			throw MissingDomAttribute (layout_element, "path");

		Time switch_delay = 0_ms;
		if (layout_element.hasAttribute ("switch-delay"))
			parse (layout_element.attribute ("switch-delay").toStdString(), switch_delay);

		stack = std::make_shared<Stack> (switch_delay);
		stack->property.set_path (PropertyPath (layout_element.attribute ("path")));
		stack->property.set_default (0);
		new_layout = stack->layout.get();
		_stacks.insert (stack);
	}
	else
		throw BadDomAttribute (layout_element, "type", "must be 'vertical', 'horizontal' or 'stack'");

	new_layout->setSpacing (0);
	new_layout->setMargin (0);

	if (box_new_layout)
		box_new_layout->setSpacing (spacing);

	for (QDomElement& e: layout_element)
	{
		if (e == "item")
			process_item_element (e, new_layout, parent_widget, panel, stack);
		else if (e == "stretch")
		{
			if (!panel)
				throw BadDomElement (e, "<stretch> can only be used as descendant of <panel>");

			if (box_new_layout)
				box_new_layout->addStretch (10000);
		}
		else if (e == "separator")
		{
			if (type == "stack")
				throw BadDomElement (e, "<separator> not allowed in stack-type layout");

			QWidget* separator = new QWidget (parent_widget);
			separator->setMinimumSize (2, 2);
			separator->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			separator->setBackgroundRole (QPalette::Dark);
			separator->setAutoFillBackground (true);
			separator->setCursor (QCursor (Qt::CrossCursor));
			new_layout->addWidget (separator);
		}
		else
			throw BadDomElement (e);
	}

	return new_layout;
}


void
Window::process_item_element (QDomElement const& item_element, QLayout* layout, QWidget* parent_widget, Panel* panel, Shared<Stack> stack)
{
	QBoxLayout* box_layout = dynamic_cast<QBoxLayout*> (layout);
	QStackedLayout* stacked_layout = dynamic_cast<QStackedLayout*> (layout);

	assert (stacked_layout && stack);

	// @stretch-factor
	if (stacked_layout && item_element.hasAttribute ("stretch-factor"))
		throw BadDomAttribute (item_element, "stretch-factor", "'stretch-factor' not allowed on <item> of stack-type layout");

	// @id
	if (box_layout && item_element.hasAttribute ("id"))
		throw BadDomAttribute (item_element, "id", "'id' not allowed on <item> of non-stack-type layout");

	// @margin
	std::array<int, 4> margins = parse_margin (item_element, "margin");

	bool has_child = false;
	int stretch = clamped (item_element.attribute ("stretch-factor").toInt(), 1, std::numeric_limits<int>::max());

	// <item>'s children:
	for (QDomElement& e: item_element)
	{
		if (has_child)
			throw BadConfiguration ("only one child element per <item> allowed");

		has_child = true;

		if (e == "layout")
		{
			if (box_layout)
			{
				QLayout* sub_layout = process_layout_element (e, parent_widget, panel);
				sub_layout->setContentsMargins (margins[0], margins[1], margins[2], margins[3]);
				box_layout->addLayout (sub_layout, stretch);
			}
			else if (stacked_layout)
			{
				QLayout* sub_layout = process_layout_element (e, parent_widget, panel);
				QWidget* proxy_widget = new QWidget (parent_widget);
				proxy_widget->setLayout (sub_layout);
				stacked_layout->addWidget (proxy_widget);
				stacked_layout->setCurrentWidget (proxy_widget);
			}
		}
		else if (e == "instrument" || e == "panel" || e == "group" || e == "widget" || e == "disabled-widget")
		{
			QWidget* widget = nullptr;

			if (e == "instrument")
			{
				Module* module = _config_reader->process_module_element (e, parent_widget);
				if (module)
				{
					Instrument* instrument = dynamic_cast<Instrument*> (module);
					if (instrument)
					{
						// Put instrument widget into a decorator:
						auto decorated_instrument = std::make_unique<InstrumentDecorator> (parent_widget);
						decorated_instrument->set_instrument (instrument);
						widget = decorated_instrument.get();
						_decorators[module->get_pointer()] = decorated_instrument.get();
						decorated_instrument.release();
					}
				}
			}
			else if (e == "panel")
				widget = process_panel_element (e, parent_widget);
			else if (e == "group")
				widget = process_group_element (e, parent_widget, panel);
			else if (e == "widget" || e == "disabled-widget")
				widget = process_widget_element (e, parent_widget, panel);

			// If it's a widget, add it to layout:
			if (widget)
			{
				if (box_layout)
				{
					// Another proxy layout is here so that we can add margins around the widget.
					// That is because setContentsMargins() doesn't work at all on QWidgets,
					// but it does on QLayout.
					QVBoxLayout* proxy_layout = new QVBoxLayout();
					proxy_layout->setContentsMargins (margins[0], margins[1], margins[2], margins[3]);
					proxy_layout->addWidget (widget);
					box_layout->addLayout (proxy_layout, stretch);
				}
				else if (stacked_layout)
				{
					stacked_layout->addWidget (widget);
					stacked_layout->setCurrentWidget (widget);
				}
			}
		}
		else
			throw BadDomElement (e);
	}

	if (!has_child)
	{
		if (box_layout)
		{
			box_layout->addStretch (stretch);
			QWidget* p = box_layout->parentWidget();
			if (p)
				p->setCursor (QCursor (Qt::CrossCursor));
		}
		else if (stacked_layout)
		{
			// Empty widget:
			QWidget* empty_widget = new QWidget (parent_widget);
			empty_widget->setCursor (QCursor (Qt::CrossCursor));
			stacked_layout->addWidget (empty_widget);
			stacked_layout->setCurrentWidget (empty_widget);
		}
	}
}


void
Window::show_configurator()
{
	if (_stack->currentWidget() == _instruments_panel)
	{
		ConfiguratorWidget* configurator_widget = _xefis->configurator_widget();
		if (!configurator_widget)
			return;
		if (configurator_widget->owning_window())
			configurator_widget->owning_window()->configurator_taken();
		_configurator_panel->layout()->addWidget (configurator_widget);
		_stack->setCurrentWidget (_configurator_panel);
		configurator_widget->set_owning_window (this);
	}
	else
		_stack->setCurrentWidget (_instruments_panel);
}


void
Window::configurator_taken()
{
	_stack->setCurrentWidget (_instruments_panel);
}


std::array<int, 4>
Window::parse_margin (QDomElement const& element, QString const& attribute_name)
{
	std::array<int, 4> margins = { { 0, 0, 0, 0 } };
	QString string = element.attribute (attribute_name);

	if (!string.isEmpty())
	{
		// Margin can be 1, 2, 3 or 4 numbers of pixels. Order: same as in CSS.
		// No units allowed - just numbers.
		QStringList numbers = string.simplified().split (" ", QString::SkipEmptyParts);
		switch (numbers.size())
		{
			case 1:
			{
				int a = numbers[0].toInt();
				margins = { { a, a, a, a } };
				break;
			}

			case 2:
			{
				int v = numbers[0].toInt();
				int h = numbers[1].toInt();
				margins = { { h, v, h, v } };
				break;
			}

			case 3:
			{
				int t = numbers[0].toInt();
				int h = numbers[1].toInt();
				int b = numbers[2].toInt();
				margins = { { h, t, h, b } };
				break;
			}

			case 4:
			{
				int t = numbers[0].toInt();
				int r = numbers[1].toInt();
				int b = numbers[2].toInt();
				int l = numbers[3].toInt();
				margins = { { l, t, r, b } };
				break;
			}

			default:
				throw BadDomAttribute (element, attribute_name, "bad format");
		}
	}

	return margins;
}


std::array<int, 4>
Window::parse_padding (QDomElement const& element, QString const& attribute_name)
{
	return parse_margin (element, attribute_name);
}

} // namespace xf

