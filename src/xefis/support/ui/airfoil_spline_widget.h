/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__UI__AIRFOIL_SPLINE_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__UI__AIRFOIL_SPLINE_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/airfoil.h>
#include <xefis/support/aerodynamics/airfoil_spline.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/ui/canvas_widget.h>

// Qt:
#include <QPen>
#include <QBrush>
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf {

class AirfoilSplineWidget: public CanvasWidget
{
  public:
	// Ctor
	explicit
	AirfoilSplineWidget (QWidget* parent = nullptr, Qt::WindowFlags = Qt::Widget);

	/**
	 * Set new airfoil to use.
	 * Calling this resets the center of pressure value.
	 */
	void
	set_airfoil (Airfoil const& airfoil);

	/**
	 * Set new airfoil spline to use.
	 * Calling this resets the center of pressure value.
	 */
	void
	set_airfoil_spline (AirfoilSpline const&, std::optional<si::Length> chord_length = std::nullopt);

	/**
	 * Set center of pressure position.
	 */
	template<class Space>
		requires (std::is_same_v<Space, BodyCOM> || std::is_same_v<Space, AirfoilSplineSpace>)
		void
		set_center_of_pressure_position (PlaneVector<double, Space> const& position)
		{
			_center_of_pressure_position = { position.x(), position.y() };

			if constexpr (std::is_same_v<Space, BodyCOM>)
				_center_of_pressure_position_relative_to_com = true;
			else if constexpr (std::is_same_v<Space, AirfoilSplineSpace>)
				_center_of_pressure_position_relative_to_com = false;
			else
				static_assert (false, "Unsupported coordinate system");

			mark_dirty();
		}

	/**
	 * Set lift force (relative to center of mass).
	 */
	void
	set_lift_force (PlaneVector<si::Force, BodyCOM> const& lift_force)
		{ _lift_force = lift_force; }

	/**
	 * Set drag force (relative to center of mass).
	 */
	void
	set_drag_force (PlaneVector<si::Force, BodyCOM> const& drag_force)
		{ _drag_force = drag_force; }

	/**
	 * Set pitching moment (relative to center of mass).
	 */
	void
	set_pitching_moment (PlaneVector<si::Torque, BodyCOM> const& pitching_moment)
		{ _pitching_moment = pitching_moment; }

	/**
	 * Set aerodynamic center position. Can be invoked only if chord_length
	 * was provided in set_airfoil_spline().
	 */
	template<class Space>
		requires (std::is_same_v<Space, BodyCOM> || std::is_same_v<Space, AirfoilSplineSpace>)
		void
		set_center_of_pressure_position (PlaneVector<si::Length, Space> const& position)
		{
			if (_chord_length)
				set_center_of_pressure_position<Space> (position / *_chord_length);
			else
				throw neutrino::PreconditionFailed ("chord length must be set");
		}

	/**
	 * Set how long the force arrows should be.
	 */
	void
	set_force_per_spline_space_unit (si::Force const force)
		{ _force_per_spline_space_unit = force; }

  protected:
	// QWidget/CanvasWidget API
	void
	resizeEvent (QResizeEvent*) override;

	void
	prepare_for_painting();

	void
	setup_painting_transform();

	// CanvasWidget API
	void
	update_canvas() override;

  private:
	AirfoilSpline						_airfoil_spline {};
	std::optional<si::Length>			_chord_length;
	QPointF								_center_of_mass_position;
	std::optional<QPointF>				_center_of_pressure_position;
	std::optional<PlaneVector<si::Force, BodyCOM>>
										_lift_force;
	std::optional<PlaneVector<si::Force, BodyCOM>>
										_drag_force;
	std::optional<PlaneVector<si::Torque, BodyCOM>>
										_pitching_moment;
	// If not relative to COM, then it's relative to space origin in AirfoilSplineSpace:
	bool								_center_of_pressure_position_relative_to_com { false };
	si::Force							_force_per_spline_space_unit { 500_N };
	bool								_pens_calculated { false };
	QPen								_coordinate_lines_pen;
	QPen								_airfoil_pen;
	QBrush								_airfoil_brush;
	QPen								_center_of_mass_black_pen;
	QPen								_lift_force_pen;
	QPen								_drag_force_pen;
	QPen								_center_of_pressure_pen;
	// Values computed by prepare_for_painting():
	QPolygonF							_airfoil_polygon;
	// Range of airfoil spline points in AirfoilSplineSpace:
	Range<double>						_range[2]; // X and Y respectively
	// Computed by setup_painting_transform():
	double								_scale { 1.0 };
	QTransform							_painting_transform;
};

} // namespace xf

#endif

