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

#ifndef XEFIS__CORE__COMPONENTS__SIMULATOR__STANDARD_EDITOR_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SIMULATOR__STANDARD_EDITOR_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/observation_widget.h>
#include <xefis/support/ui/paint_helper.h>
#include <xefis/support/ui/widget.h>

// Qt:
#include <QLabel>

// Standard:
#include <cstddef>


namespace xf {

class RigidBodyViewer;


template<class tObject>
	class StandardEditor: public QWidget
	{
	  public:
		using Object = tObject;

	  public:
		// Ctor
		explicit
		StandardEditor (QWidget* parent, RigidBodyViewer& viewer, QColor strip_color);

		/**
		 * Sets object to edit. Pass nullptr to disable.
		 */
		void
		edit (Object*);

		/**
		 * Update data about currently edited object.
		 */
		void
		refresh();

	  private:
		RigidBodyViewer&	_rigid_body_viewer;
		Object*				_edited_object { nullptr };
		std::unique_ptr<ObservationWidget>
							_edited_object_widget;
		QVBoxLayout			_edited_object_widget_layout;
		QLabel*				_object_label;
	};


template<class O>
	inline
	StandardEditor<O>::StandardEditor (QWidget* parent, RigidBodyViewer& viewer, QColor const strip_color):
		QWidget (parent),
		_rigid_body_viewer (viewer)
	{
		auto const ph = PaintHelper (*this);

		auto [top_strip, top_label] = Widget::create_colored_strip_label ("–", strip_color, Qt::AlignBottom, this);
		top_strip->setMinimumWidth (ph.em_pixels_int (25));
		_object_label = top_label;

		auto* layout = new QVBoxLayout (this);
		layout->addWidget (top_strip);
		layout->addLayout (&_edited_object_widget_layout);
		layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

		setEnabled (false);
		refresh();
	}


template<class O>
	inline void
	StandardEditor<O>::edit (Object* object_to_edit)
	{
		_edited_object = object_to_edit;
		_edited_object_widget.reset();

		if (_edited_object)
		{
			if constexpr (std::is_polymorphic_v<Object>)
				if (auto* has_observation_widget = dynamic_cast<HasObservationWidget*> (_edited_object))
					_edited_object_widget = has_observation_widget->create_observation_widget();

			if (!_edited_object_widget)
				_edited_object_widget = std::make_unique<ObservationWidget> (_edited_object);

			if (_edited_object_widget)
				_edited_object_widget_layout.addWidget (_edited_object_widget.get());
		}

		refresh();
	}


template<class O>
	inline void
	StandardEditor<O>::refresh()
	{
		if (_edited_object_widget)
			_edited_object_widget->update_observed_values (_rigid_body_viewer.planet());

		if (_edited_object)
		{
			setEnabled (true);
			_object_label->setText (QString::fromStdString (_edited_object->label()));
		}
		else
		{
			setEnabled (false);
			_object_label->setText ("–");
		}
	}

} // namespace xf

#endif

