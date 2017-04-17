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
#include <typeinfo>

// Qt:
#include <QtCore/QDateTime>
#include <QtCore/QTimeZone>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QLayout>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/window.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qdom_iterator.h>
#include <xefis/utility/text_layout.h>

// Local:
#include "cdu.h"


XEFIS_REGISTER_MODULE_CLASS ("instruments/cdu", CDU)


constexpr double CDU::kButtonWidthForHeight;


CDU::Strip::Strip (CDU& cdu, QString const& title, Column column):
	_cdu (cdu),
	_title (title),
	_column (column)
{ }


QString const&
CDU::Strip::title() const noexcept
{
	return _title;
}


CDU::Column
CDU::Strip::column() const noexcept
{
	return _column;
}


void
CDU::Strip::set_rect (QRectF const& rect)
{
	_rect = rect;
}


QRectF const&
CDU::Strip::rect() const noexcept
{
	return _rect;
}


CDU&
CDU::Strip::cdu() const noexcept
{
	return _cdu;
}


bool
CDU::Strip::fresh() const noexcept
{
	return false;
}


void
CDU::Strip::paint (QRectF const& rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column, bool focused)
{
	QPen focus_pen = aids._autopilot_pen_2;
	double fpw = 0.5 * focus_pen.width();
	double top_bottom_margin = 4.0 * fpw;
	QRectF inner_rect = rect.adjusted (fpw, fpw, -fpw, -fpw);

	// Compute rects:

	QRectF button_rect;
	QSizeF button_size (kButtonWidthForHeight * inner_rect.height(), inner_rect.height());
	switch (column)
	{
		case Column::Left:
			button_rect = QRectF (inner_rect.topLeft(), button_size);
			break;

		case Column::Right:
			button_rect = QRectF (inner_rect.topRight() - QPointF (button_size.width(), 0.0), button_size);
			break;
	}
	button_rect.adjust (0.0, top_bottom_margin, 0.0, -top_bottom_margin);

	double dw = button_rect.width() + aids.pen_width (10.0);
	double kw = rect.width() - dw;
	QRectF title_rect (QPointF (0.0, 0.0), QSizeF (kw, aids._font_16_digit_height));
	QRectF value_rect (QPointF (0.0, 0.0), QSizeF (kw, aids._font_20_digit_height));
	switch (column)
	{
		case Column::Left:
			value_rect.moveTopLeft (QPointF (rect.left() + dw, button_rect.center().y() - 0.5 * value_rect.height()));
			title_rect.moveBottomLeft (QPointF (value_rect.left() + aids.pen_width (10.0), value_rect.top() - aids.pen_width (5.0)));
			break;

		case Column::Right:
			value_rect.moveTopRight (QPointF (rect.right() - dw, button_rect.center().y() - 0.5 * value_rect.height()));
			title_rect.moveBottomRight (QPointF (value_rect.right() - aids.pen_width (10.0), value_rect.top() - aids.pen_width (5.0)));
			break;
	}

	// Draw parts:
	paint_button (button_rect, aids, painter, column, focused);
	paint_title (title_rect, aids, painter, column, focused);
	paint_value (value_rect, aids, painter, column, focused);

	// Focus:
	if (focused)
		paint_focus (rect, button_rect.adjusted (-fpw, -fpw, fpw, fpw), aids, painter, column);
}


void
CDU::Strip::paint_button_helper (QRectF const& rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column, ButtonState state)
{
	QRectF btn_rect (rect.topLeft(), QSizeF (0.6 * rect.width(), rect.height()));
	if (column == Column::Right)
		btn_rect.translate (rect.width() - btn_rect.width(), 0.0);

	double adj_2 = aids.pen_width (1.0);
	double adj_3 = aids.pen_width (2.25);
	double swh = std::min (btn_rect.width(), btn_rect.height());
	QRectF rect_2 = btn_rect.adjusted (adj_2, adj_2, -adj_2, -adj_2);
	QRectF rect_3 = btn_rect.adjusted (adj_3, adj_3, -adj_3, -adj_3);
	QPointF point_delta (0.5 * swh, -0.5 * swh);
	QPointF point_l = btn_rect.bottomLeft() + point_delta;
	QPointF point_r = btn_rect.topRight() - point_delta;
	// White line:
	QPointF pa, pb;
	switch (column)
	{
		case Column::Left:
			pa = QPointF (btn_rect.right(), btn_rect.center().y());
			pb = QPointF (btn_rect.right() + 0.35 * rect.width(), pa.y());
			break;

		case Column::Right:
			pa = QPointF (btn_rect.left(), btn_rect.center().y());
			pb = QPointF (btn_rect.left() - 0.35 * rect.width(), pa.y());
			break;
	}

	switch (state)
	{
		case ButtonState::Normal:
		case ButtonState::Pressed:
		{
			// White line:
			painter.setPen (aids.get_pen (Qt::white, 1.0));
			painter.add_shadow (2.0, [&] {
				painter.drawLine (pa, pb);
			});

			QColor highlight_color (0xcc, 0xcc, 0xcc);
			QColor shadow_color (0x55, 0x55, 0x55);
			QColor face_color (0x88, 0x88, 0x88);
			if (state == ButtonState::Pressed)
			{
				std::swap (highlight_color, shadow_color);
				shadow_color = shadow_color.darker (150);
				face_color = face_color.darker (125);
			}

			// Backgorund/frame:
			painter.setPen (Qt::NoPen);
			painter.fillRect (btn_rect, Qt::black);
			// Highlight:
			painter.setBrush (highlight_color);
			painter.drawPolygon (QPolygonF (QVector<QPointF> { rect_2.topLeft(), rect_2.topRight(), point_r, point_l, rect_2.bottomLeft() }));
			// Shadow:
			painter.setBrush (shadow_color);
			painter.drawPolygon (QPolygonF (QVector<QPointF> { rect_2.topRight(), rect_2.bottomRight(), rect_2.bottomLeft(), point_l, point_r }));
			// Face:
			painter.fillRect (rect_3, face_color);
			break;
		}

		case ButtonState::Disabled:
		{
			QColor cyan (0x22, 0xcc, 0xff);
			painter.setPen (aids.get_pen (cyan, 1.0));
			painter.setBrush (Qt::NoBrush);
			painter.add_shadow (2.0, [&] {
				painter.drawLine (pa, pb);
				painter.drawRect (rect_2);
			});
			break;
		}
	}
}


void
CDU::Strip::paint_title_helper (QRectF const& rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column, QString const& title, QColor color)
{
	Qt::Alignment title_alignment;

	switch (column)
	{
		case Column::Left:
			title_alignment = Qt::AlignVCenter | Qt::AlignLeft;
			break;

		case Column::Right:
			title_alignment = Qt::AlignVCenter | Qt::AlignRight;
			break;
	}

	painter.setFont (aids._font_13);
	painter.setPen (aids.get_pen (color, 1.0));
	painter.fast_draw_text (rect, title_alignment, title);
}


void
CDU::Strip::paint_value_helper (QRectF const& rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column, QString const& value, QColor color)
{
	Qt::Alignment value_alignment;

	switch (column)
	{
		case Column::Left:
			value_alignment = Qt::AlignVCenter | Qt::AlignLeft;
			break;

		case Column::Right:
			value_alignment = Qt::AlignVCenter | Qt::AlignRight;
			break;
	}

	painter.setFont (aids._font_20);
	painter.setPen (aids.get_pen (color, 1.0));
	painter.fast_draw_text (rect, value_alignment, value);
}


void
CDU::Strip::paint_focus_helper (QRectF const& rect, QRectF const& button_rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column)
{
	QRectF const& r = button_rect;
	QPolygonF polygon;

	double r_left = 0.0;
	double rect_right = 0.0;
	double r_width = 0.0;
	QPointF r_topLeft;
	QPointF r_bottomLeft;

	switch (column)
	{
		case Column::Left:
		{
			r_left = r.left();
			rect_right = rect.right();
			r_width = r.width();
			r_topLeft = r.topLeft();
			r_bottomLeft = r.bottomLeft();
			break;
		}

		case Column::Right:
		{
			r_left = r.right();
			rect_right = rect.left();
			r_width = -r.width();
			r_topLeft = r.topRight();
			r_bottomLeft = r.bottomRight();
			break;
		}
	}

	double rx = r_left + 0.61 * r_width;
	double ry1 = r.top() + 0.2 * r.height();
	double ry2 = r.top() + 0.8 * r.height();
	polygon
		<< r_topLeft
		<< QPointF (rx, r.top())
		<< QPointF (rx, ry1)
		<< QPointF (rect_right, ry1)
		<< QPointF (rect_right, ry2)
		<< QPointF (rx, ry2)
		<< QPointF (rx, r.bottom())
		<< r_bottomLeft;
	polygon << polygon[0];

	painter.setPen (aids._autopilot_pen_2);
	painter.setBrush (Qt::NoBrush);
	painter.drawPolyline (polygon);
}


CDU::EmptyStrip::EmptyStrip (CDU& cdu, Column column):
	Strip (cdu, "", column)
{ }


void
CDU::EmptyStrip::paint_button (QRectF const& rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column, bool)
{
	paint_button_helper (rect, aids, painter, column, ButtonState::Disabled);
}


CDU::SettingStrip::SettingStrip (CDU& cdu, QDomElement const& setting_element, Column column):
	Strip (cdu, setting_element.attribute ("title"), column)
{
	if (!setting_element.hasAttribute ("path"))
		throw xf::MissingDomAttribute (setting_element, "path");

	_nil_value = setting_element.attribute ("nil-value", "").toStdString();
	_format = setting_element.attribute ("format", "%1%").toStdString();
	_true_value = setting_element.attribute ("true-value", "ON").toStdString();
	_false_value = setting_element.attribute ("false-value", "OFF").toStdString();
	_read_only = setting_element.attribute ("read-only") == "true";
	_property.set_path (xf::PropertyPath (setting_element.attribute ("path")));
	_unit = setting_element.attribute ("unit").toStdString();

	if (si::units_map().find (_unit) == si::units_map().end())
		throw xf::BadConfiguration ("unsupported unit '" + _unit + "'");
}


bool
CDU::SettingStrip::fresh() const noexcept
{
	return _property.fresh();
}


void
CDU::SettingStrip::handle_mouse_press (QMouseEvent* event, CDU*)
{
	if (!_read_only)
		if (_button_rect.contains (event->pos()))
			_button_state = ButtonState::Pressed;
}


void
CDU::SettingStrip::handle_mouse_release (QMouseEvent* event, CDU* cdu)
{
	_button_state = ButtonState::Normal;

	if (!_read_only &&
		_button_rect.contains (event->pos()) &&
		_property.configured())
	{
		if (_property.is_type<bool>())
		{
			xf::PropertyBoolean& property = static_cast<xf::PropertyBoolean&> (_property);
			property = !*property;
		}
		else
		{
			std::string entry_value = cdu->entry_value().trimmed().toStdString();
			std::exception_ptr eptr;

			// Try first to parse the value just as is. If it fails, try to append default unit.
			try {
				_property.parse_existing (entry_value);
			}
			catch (...)
			{
				eptr = std::current_exception();

				try {
					_property.parse_existing (entry_value + " " + _unit);
					eptr = nullptr;
				}
				catch (...)
				{
					try {
						std::rethrow_exception (eptr);
					}
					catch (si::UnsupportedUnit const&)
					{
						cdu->post_message ("Unsupported unit");
					}
					catch (si::UnparsableValue const&)
					{
						cdu->post_message ("Invalid value");
					}
					catch (si::IncompatibleTypes const&)
					{
						cdu->post_message ("Incompatible unit");
					}
				}
			}

			if (!eptr)
				cdu->clear_entry_value();
		}
	}
}


void
CDU::SettingStrip::paint_button (QRectF const& rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column, bool)
{
	_button_rect = rect;
	ButtonState button_state = ButtonState::Normal;
	if (_read_only)
		button_state = ButtonState::Disabled;
	else if (_button_rect.contains (cdu().mapFromGlobal (QCursor::pos())))
		button_state = _button_state;
	paint_button_helper (rect, aids, painter, column, button_state);
}


void
CDU::SettingStrip::paint_title (QRectF const& rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column, bool)
{
	paint_title_helper (rect, aids, painter, column, title(), QColor (0xcc, 0xd7, 0xe7));
}


void
CDU::SettingStrip::paint_value (QRectF const& rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column, bool)
{
	if (_property.valid())
	{
		if (_property.is_type<bool>())
		{
			bool p = *static_cast<xf::PropertyBoolean&> (_property);
			std::string const act_val = p ? _true_value : _false_value;
			std::string const inact_val = p ? _false_value : _true_value;

			xf::TextLayout tl;
			tl.set_alignment (Qt::AlignCenter);
			tl.set_background (Qt::NoBrush);

			switch (column)
			{
				case Column::Left:
					tl.add_fragment (inact_val, aids._font_13, Qt::white);
					tl.add_fragment ("\u2008⬌\u2008", aids._font_20, Qt::white);
					tl.add_fragment (act_val, aids._font_20, Qt::green);
					tl.paint (QPointF (rect.left(), rect.center().y()), Qt::AlignLeft | Qt::AlignVCenter, painter);
					break;

				case Column::Right:
					tl.add_fragment (act_val, aids._font_20, Qt::green);
					tl.add_fragment ("\u2008⬌\u2008", aids._font_20, Qt::white);
					tl.add_fragment (inact_val, aids._font_13, Qt::white);
					tl.paint (QPointF (rect.right(), rect.center().y()), Qt::AlignRight | Qt::AlignVCenter, painter);
					break;
			}
		}
		else
		{
			QColor str_color = _read_only ? QColor (0x22, 0xcc, 0xff) : Qt::white;
			QString str_val;
			try {
				str_val = QString::fromStdString (_property.stringify (boost::format (_format), _unit, _nil_value));
			}
			catch (xf::StringifyError const& exception)
			{
				str_color = Qt::red;
				str_val = exception.what();
			}
			catch (boost::io::bad_format_string const&)
			{
				str_color = Qt::red;
				str_val = "format: ill formed";
			}

			paint_value_helper (rect, aids, painter, column, str_val, str_color);
		}
	}
	else if (!_unit.empty())
	{
		bool left = column == Column::Left;
		// At least paint information about units.
		auto horz_alignment = left ? Qt::AlignLeft : Qt::AlignRight;
		QPointF position {
			(column == Column::Left) ? rect.left() : rect.right(),
			rect.center().y()
		};
		std::string text = left ? ("― [" + _unit + "]") : ("[" + _unit + "] ―");

		xf::TextLayout tl;
		tl.set_alignment (Qt::AlignCenter);
		tl.set_background (Qt::NoBrush);
		tl.add_fragment (text, aids._font_13, Qt::gray);
		tl.paint (position, horz_alignment | Qt::AlignVCenter, painter);
	}
}


void
CDU::SettingStrip::paint_focus (QRectF const& rect, QRectF const& button_rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column)
{
	if (!_read_only)
		paint_focus_helper (rect, button_rect, aids, painter, column);
}


CDU::GotoStrip::GotoStrip (CDU& cdu, QDomElement const& goto_element, Column column):
	Strip (cdu, goto_element.attribute ("title"), column)
{
	if (!goto_element.hasAttribute ("page-id"))
		throw xf::MissingDomAttribute (goto_element, "page-id");

	_target_page_id = goto_element.attribute ("page-id");
}


inline QString const&
CDU::GotoStrip::target_page_id() const noexcept
{
	return _target_page_id;
}


void
CDU::GotoStrip::handle_mouse_press (QMouseEvent* event, CDU*)
{
	if (_button_rect.contains (event->pos()))
		_button_state = ButtonState::Pressed;
}


void
CDU::GotoStrip::handle_mouse_release (QMouseEvent* event, CDU* cdu)
{
	_button_state = ButtonState::Normal;

	if (_button_rect.contains (event->pos()))
		cdu->switch_page (_target_page_id);
}


void
CDU::GotoStrip::paint_button (QRectF const& rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column, bool)
{
	_button_rect = rect;
	bool over_button = _button_rect.contains (cdu().mapFromGlobal (QCursor::pos()));
	paint_button_helper (rect, aids, painter, column, over_button ? _button_state : ButtonState::Normal);
}


void
CDU::GotoStrip::paint_value (QRectF const& rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column, bool)
{
	paint_value_helper (rect, aids, painter, column, title(), Qt::white);
}


void
CDU::GotoStrip::paint_focus (QRectF const& rect, QRectF const& button_rect, xf::InstrumentAids& aids, xf::Painter& painter, Column column)
{
	paint_focus_helper (rect, button_rect, aids, painter, column);
}


CDU::Page::Page (CDU& cdu, QDomElement const& page_element, Config& config, xf::Logger const& logger)
{
	if (page_element.hasAttribute ("id"))
		_id = page_element.attribute ("id");
	else
	{
		// Auto-generated ID:
		_id = QString::fromStdString ((boost::format ("__page#%016x") % this).str());
	}

	_title = page_element.attribute ("title");

	Column current_column_dir = Column::Left;
	Strips* current_column = nullptr;

	auto forget_strip = [&] (Strip* ptr)
	{
		for (UniqueStrips::iterator it = _strips.begin(); it != _strips.end(); ++it)
		{
			if (it->get() == ptr)
			{
				_strips.erase (it);
				break;
			}
		}
	};

	auto parse_column = [&] (QDomElement const& column_element)
	{
		bool has_fill_element = false;

		for (QDomElement const& e: xf::iterate_sub_elements (column_element))
		{
			if (e == "setting")
			{
				_strips.push_back (std::make_unique<SettingStrip> (cdu, e, current_column_dir));
				current_column->push_back (_strips.back().get());
			}
			else if (e == "goto")
			{
				_strips.push_back (std::make_unique<GotoStrip> (cdu, e, current_column_dir));
				current_column->push_back (_strips.back().get());
			}
			else if (e == "empty")
			{
				_strips.push_back (std::make_unique<EmptyStrip> (cdu, current_column_dir));
				current_column->push_back (_strips.back().get());
			}
			else if (e == "fill")
			{
				if (has_fill_element)
					logger << "Warning: <fill> already defined in the column, ignoring others." << std::endl;
				else
				{
					_strips.push_back (std::make_unique<FillStrip> (cdu));
					current_column->push_back (_strips.back().get());
				}
			}
			else
				throw xf::BadDomElement (e);
		}

		// Handle FillStrips:
		for (std::size_t i = 0; i < current_column->size(); ++i)
		{
			if (dynamic_cast<FillStrip*> (current_column->at (i)))
			{
				// Replace with enough EmptyStrips so that the total size is equal to config.rows():
				Strips::iterator it = current_column->begin() + i;
				forget_strip (*it);
				current_column->erase (it);
				if (current_column->size() < config.rows())
				{
					for (std::size_t j = 0, n = config.rows() - current_column->size(); j < n; ++j)
					{
						_strips.push_back (std::make_unique<EmptyStrip> (cdu, current_column_dir));
						it = current_column->insert (it, _strips.back().get());
					}
				}
				break;
			}
		}

		// Make sure that the column has exactly [rows] elements.
		// Remove overflow:
		if (current_column->size() > config.rows())
		{
			logger << "Warning: page '" << _id.toStdString() << "': number of elements exceed rows setting (" << config.rows() << ")" << std::endl;
			for (auto it = current_column->begin() + config.rows(); it != current_column->end(); ++it)
				forget_strip (*it);
			current_column->resize (config.rows());
		}
		// Fill in underflow:
		else if (current_column->size() < config.rows())
		{
			for (std::size_t i = 0, n = config.rows() - current_column->size(); i < n; ++i)
			{
				_strips.push_back (std::make_unique<EmptyStrip> (cdu, current_column_dir));
				current_column->push_back (_strips.back().get());
			}
		}
	};

	for (QDomElement const& e: xf::iterate_sub_elements (page_element))
	{
		if (e == "left")
		{
			current_column_dir = Column::Left;
			current_column = &_strips_left;
			parse_column (e);
		}
		else if (e == "right")
		{
			current_column_dir = Column::Right;
			current_column = &_strips_right;
			parse_column (e);
		}
		else
			throw xf::BadDomElement (e);
	}
}


inline QString const&
CDU::Page::id() const noexcept
{
	return _id;
}


inline QString const&
CDU::Page::title() const noexcept
{
	return _title;
}


inline CDU::Page::UniqueStrips const&
CDU::Page::strips() const noexcept
{
	return _strips;
}


inline CDU::Page::Strips const&
CDU::Page::strips_left() const noexcept
{
	return _strips_left;
}


inline CDU::Page::Strips const&
CDU::Page::strips_right() const noexcept
{
	return _strips_right;
}


bool
CDU::Page::scan_properties() const noexcept
{
	for (auto const& strip: _strips)
		if (strip->fresh())
			return true;
	return false;
}


bool
CDU::Page::handle_mouse_move (QMouseEvent* event)
{
	Strip* old_focused_strip = _focused_strip;
	_focused_strip = nullptr;

	for (auto& strip: _strips)
	{
		if (strip->rect().contains (event->pos()))
		{
			_focused_strip = strip.get();
			break;
		}
	}

	return _focused_strip != old_focused_strip;
}


bool
CDU::Page::handle_mouse_press (QMouseEvent* event, CDU* cdu)
{
	if (_focused_strip)
	{
		_focused_strip->handle_mouse_press (event, cdu);
		_capture_strip = _focused_strip;
	}

	return !!_focused_strip;
}


bool
CDU::Page::handle_mouse_release (QMouseEvent* event, CDU* cdu)
{
	if (_capture_strip)
		_capture_strip->handle_mouse_release (event, cdu);
	_capture_strip = nullptr;

	return !!_capture_strip;
}


void
CDU::Page::paint (QRectF const& rect, xf::InstrumentAids& aids, xf::Painter& painter)
{
	auto paint_column = [&] (Column column, QRectF const& column_rect, Page::Strips const& strips)
	{
		std::size_t n = strips.size();
		QSizeF size (column_rect.width(), column_rect.height() / n);

		for (std::size_t i = 0; i < n; ++i)
		{
			auto const& strip = strips[i];
			QRectF strip_rect (QPointF (column_rect.left(), column_rect.top() + i * size.height()), size);
			strip->set_rect (strip_rect);
			strip->paint (strip_rect, aids, painter, column, _focused_strip == strip);
		}
	};

	double title_height = 2.25 * aids._font_20_digit_height;
	QRectF strips_rect = rect.adjusted (0.0, title_height, 0.0, 0.0);
	double strip_height = strips_rect.height() / std::max (strips_left().size(), strips_right().size());
	QRectF black_rect = rect.adjusted (kButtonWidthForHeight * strip_height, 0.0, -kButtonWidthForHeight * strip_height, 0.0);
	QSizeF half_size (0.5 * strips_rect.width(), strips_rect.height());
	_bb_margin = black_rect.left();

	// Black rect:
	painter.setFont (aids._font_20);
	painter.setPen (aids.get_pen (QColor (0xbb, 0xbb, 0xbb), 1.0));
	painter.setBrush (Qt::black);
	painter.drawRect (black_rect);

	// Page title:
	painter.setPen (aids.get_pen (Qt::white, 1.0));
	painter.fast_draw_text (QPointF (rect.center().x(), rect.top() + 0.35 * title_height),
							  Qt::AlignHCenter | Qt::AlignVCenter,
							  title());

	paint_column (Column::Left, QRectF (strips_rect.topLeft(), half_size), strips_left());
	paint_column (Column::Right, QRectF (QPointF (strips_rect.left() + 0.5 * strips_rect.width(), strips_rect.top()), half_size), strips_right());
}


double
CDU::Page::bb_margin() const noexcept
{
	return _bb_margin;
}


void
CDU::Page::reset()
{
	_focused_strip = nullptr;
	_capture_strip = nullptr;
}


CDU::Config::Config (CDU& cdu, QDomElement const& pages_element, xf::Logger const& logger):
	_logger (logger)
{
	_default_page_id = pages_element.attribute ("default");

	if (pages_element.hasAttribute ("rows"))
		_rows = pages_element.attribute ("rows").toUInt();

	for (QDomElement const& e: xf::iterate_sub_elements (pages_element))
	{
		if (e == "page")
		{
			Shared<Page> page = std::make_shared<Page> (cdu, e, *this, logger);
			if (!_pages_by_id.insert ({ page->id(), page }).second)
				throw xf::BadConfiguration ("duplicate page with id '" + page->id() + "'");
		}
		else
			throw xf::BadDomElement (e);
	}

	check_reachability();
}


bool
CDU::Config::scan_properties() const noexcept
{
	for (auto page: _pages_by_id)
		if (page.second->scan_properties())
			return true;
	return false;
}


QString
CDU::Config::default_page_id() const noexcept
{
	return _default_page_id;
}


CDU::Page*
CDU::Config::default_page() const noexcept
{
	return find_page_by_id (default_page_id());
}


std::size_t
CDU::Config::rows() const noexcept
{
	return _rows;
}


CDU::Page*
CDU::Config::find_page_by_id (QString const& id) const noexcept
{
	auto cp = _pages_by_id.find (id);

	if (cp == _pages_by_id.end())
		return nullptr;
	else
		return cp->second.get();
}


void
CDU::Config::check_reachability() const
{
	std::set<Page*> all_pages;
	for (auto pit: _pages_by_id)
		all_pages.insert (pit.second.get());

	std::function<void (Page*)> traverse = [&] (Page* page)
	{
		all_pages.erase (page);

		for (auto const& strip: page->strips())
		{
			GotoStrip* goto_strip = dynamic_cast<GotoStrip*> (strip.get());
			if (goto_strip)
			{
				Page* next_hop = find_page_by_id (goto_strip->target_page_id());
				if (!next_hop)
					_logger << "Warning: page '" << goto_strip->target_page_id().toStdString() << "' referenced by '" << page->id().toStdString() << "' doesn't exist." << std::endl;
				else if (all_pages.find (next_hop) != all_pages.end())
					traverse (next_hop);
			}
		}
	};

	if (default_page())
	{
		traverse (default_page());

		if (!all_pages.empty())
		{
			QStringList pages;
			for (auto& page: all_pages)
				pages.push_back (page->id());
			_logger << "Warning: the following pages are not reachable from the main page: " << pages.join (", ").toStdString() << "." << std::endl;
		}
	}
}


CDU::CDU (xf::ModuleManager* module_manager, QDomElement const& config):
	xf::Instrument (module_manager, config),
	InstrumentAids (0.5f)
{
	setFocusPolicy (Qt::StrongFocus);
	setMouseTracking (true);

	for (QDomElement const& e: xf::iterate_sub_elements (config))
	{
		if (e == "pages")
		{
			_config = std::make_unique<Config> (*this, e, log());
			break;
		}
		else if (e != "settings" && e != "properties")
			throw xf::BadDomElement (e);
	}

	_current_page_id = _config->default_page_id();

	parse_properties (config, {
		{ "time.utc", _time_utc, false },
	});

	update();
}


void
CDU::data_updated()
{
	if (_config->scan_properties())
		update();
}


void
CDU::post_message (QString const& message)
{
	_messages.push_back (message);
	update();
}


void
CDU::resizeEvent (QResizeEvent*)
{
	auto xw = dynamic_cast<xf::Window*> (window());
	if (xw)
		set_scaling (xw->pen_scale(), xw->font_scale());

	InstrumentAids::update_sizes (size(), window()->size());
}


void
CDU::paintEvent (QPaintEvent*)
{
	auto painting_token = get_token (this);
	clear_background (QColor (0x55, 0x63, 0x71));

	double y_margin = 1.4 * _font_16.pixelSize();
	double x_margin = 0.3 * y_margin;

	QRectF entry_rect = rect();
	entry_rect.setTop (5.0 / 7.0 * size().height());

	QRectF strips_rect = rect();
	strips_rect.setBottom (entry_rect.top());

	entry_rect.adjust (x_margin, 0.0, -x_margin, -x_margin);
	strips_rect.adjust (x_margin, y_margin, -x_margin, -x_margin);

	// Paint date and time:
	if (_time_utc.configured())
	{
		QString time_str = "NO TIME INFO";
		QString date_str = "";

		if (_time_utc.valid())
		{
			QDateTime datetime = QDateTime::fromTime_t (_time_utc->quantity<Second>());
			datetime.setTimeZone (QTimeZone (0));
			time_str = datetime.time().toString ("HH:mm:ss") + " z";
			date_str = datetime.date().toString ("d MMM yy").toUpper();
		}

		double dy = 0.475 * (rect().top() + strips_rect.top());
		double dx = 0.2 * dy;
		painter().setFont (_font_16);
		painter().setPen (get_pen (Qt::white, 1.0));
		painter().fast_draw_text (rect().topLeft() + QPointF (dx, dy), Qt::AlignLeft | Qt::AlignVCenter, time_str);
		painter().fast_draw_text (rect().topRight() + QPointF (-dx, dy), Qt::AlignRight | Qt::AlignVCenter, date_str);
	}

	paint_strips_area (strips_rect);
	paint_entry_area (entry_rect);
}


void
CDU::keyPressEvent (QKeyEvent* event)
{
	switch (event->key())
	{
		case Qt::Key_Backspace:
			if (_entry_value.size() > 0)
				_entry_value = _entry_value.left (_entry_value.size() - 1);
			break;

		case Qt::Key_Enter:
		case Qt::Key_Return:
			_entry_value.clear();
			break;

		default:
			_entry_value += event->text();
	}

	update();
}


void
CDU::showEvent (QShowEvent*)
{
	setFocus (Qt::OtherFocusReason);
}


void
CDU::mouseMoveEvent (QMouseEvent* event)
{
	Page* page = current_page();
	if (page)
	{
		if (page->handle_mouse_move (event))
			update();
	}
}


void
CDU::mousePressEvent (QMouseEvent* event)
{
	Page* page = current_page();
	if (page && page->handle_mouse_press (event, this))
		update();
}


void
CDU::mouseReleaseEvent (QMouseEvent* event)
{
	Page* page = current_page();
	if (page && page->handle_mouse_release (event, this))
		update();
}


void
CDU::paint_strips_area (QRectF const& rect)
{
	Page* page = current_page();
	if (page)
		page->paint (rect, *this, painter());
}


void
CDU::paint_entry_area (QRectF const& rect)
{
	QColor cyan = QColor (0x00, 0xb0, 0xcf);
	double ww = 0.16 * _font_20_digit_height;
	double lh = 1.0 * _font_20_digit_height;
	double bb = current_page() ? current_page()->bb_margin() : 0.0;

	// Entry box:
	QRectF entry_rect = rect;
	entry_rect.setLeft (bb);
	entry_rect.setRight (this->rect().right() - bb);
	entry_rect.setTop (rect.top());
	entry_rect.setHeight (1.8 * _font_20_digit_height);
	QRectF text_rect = entry_rect.adjusted (ww, 0.0, -ww, 0.0);
	painter().setFont (_font_20);
	QColor color (0xbb, 0xbb, 0xbb);
	if (hasFocus())
		color = Qt::white;
	painter().setPen (get_pen (color, 1.0));
	painter().setBrush (Qt::black);
	painter().drawRect (entry_rect);
	painter().setFont (_font_20);
	painter().setPen (get_pen (Qt::white, 1.0));
	QFontMetrics metrics (_font_20);
	if (metrics.width (_entry_value) > text_rect.width())
	{
		painter().setClipRect (text_rect);
		painter().fast_draw_text (QPointF (text_rect.right(), text_rect.center().y()),
								  Qt::AlignRight | Qt::AlignVCenter, _entry_value);
	}
	else
		painter().fast_draw_text (QPointF (text_rect.left(), text_rect.center().y()),
								  Qt::AlignLeft | Qt::AlignVCenter, _entry_value);
	painter().setClipping (false);

	// Message board:
	QRectF msgbrd_rect = entry_rect;
	msgbrd_rect.moveTop (entry_rect.bottom() + lh);
	msgbrd_rect.setBottom (rect.bottom());
	painter().setPen (Qt::NoPen);
	painter().setBrush (Qt::black);
	painter().drawRect (msgbrd_rect);

	// Msg board title:
	QRectF msgbrd_title = msgbrd_rect;
	msgbrd_title.setBottom (msgbrd_title.top() + 2.0 * _font_16_digit_height);
	msgbrd_title.setRight (msgbrd_title.right() - 6.0 * _font_20_digit_height);
	painter().setFont (_font_16);
	painter().fillRect (msgbrd_title, cyan);
	painter().setPen (get_pen (Qt::white, 1.0));
	painter().fast_draw_text (msgbrd_title, Qt::AlignCenter, "MESSAGE TITLE");

	// Msg board right panel:
	QRectF msgbrd_rpanel = msgbrd_rect;
	msgbrd_rpanel.setLeft (msgbrd_title.right() - 1.0);
	painter().fillRect (msgbrd_rpanel, cyan);

	// Message texts panel:
	QRectF msgbrd_texts = msgbrd_rect;
	msgbrd_texts.setTop (msgbrd_title.bottom());
	msgbrd_texts.setRight (msgbrd_rpanel.left());
	msgbrd_texts.adjust (ww, 0.0, -ww, -ww);
	painter().setClipRect (msgbrd_texts);
	painter().setFont (_font_16);
	painter().setPen (get_pen (Qt::white, 1.0));

	double msg_height = 1.25 * _font_20_digit_height;
	QRectF virtual_texts_frame = msgbrd_texts;
	virtual_texts_frame.setHeight (_messages.size() * msg_height);
	if (virtual_texts_frame.height() > msgbrd_texts.height())
		virtual_texts_frame.moveBottom (msgbrd_texts.bottom());

	for (std::size_t i = 0; i < _messages.size(); ++i)
	{
		QPointF hook = virtual_texts_frame.topLeft() + i * QPointF (0.0, msg_height);
		if (hook.y() + msg_height < msgbrd_texts.top())
			continue;
		painter().fast_draw_text (hook, Qt::AlignTop | Qt::AlignLeft, QString ("%1: %2").arg (i + 1, 2, 10, QChar ('0')).arg (_messages[i]));
	}

	// Draw msg board outline:
	painter().setClipping (false);
	painter().setPen (get_pen (Qt::white, 1.0));
	painter().setBrush (Qt::NoBrush);
	painter().drawRect (msgbrd_rect);
}


CDU::Page*
CDU::current_page() const noexcept
{
	return _config->find_page_by_id (_current_page_id);
}


void
CDU::switch_page (QString const& page_id)
{
	if (_config->find_page_by_id (page_id))
	{
		_current_page_id = page_id;
		current_page()->reset();
		update();
	}
	else
		post_message ("Page doesn't exist");
}


QString
CDU::entry_value() const
{
	return _entry_value;
}


void
CDU::clear_entry_value()
{
	_entry_value.clear();
}

