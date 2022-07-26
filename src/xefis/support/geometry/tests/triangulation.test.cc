/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/airfoil_spline.h>
#include <xefis/support/geometry/triangulation.h>
#include <xefis/support/geometry/triangle.h>

// Neutrino:
#include <neutrino/qt/qutils.h>
#include <neutrino/test/dummy_qapplication.h>
#include <neutrino/test/manual_test.h>
#include <neutrino/test/test_widget.h>

// Lib:
#include <QPainter>
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf::test {
namespace {

ManualTest t_1 ("geometry: triangulate", []{
	static xf::AirfoilSpline const
	kSpline {
		{ 1.00,  +0.00 },
		{ 0.80,  +0.03 },
		{ 0.60,  -0.05 },
		{ 0.40,  +0.15 },
		{ 0.20,  +0.13 },
		{ 0.00,   0.00 },
		{ 0.20,  -0.13 },
		{ 0.40,  +0.05 },
		{ 0.60,  -0.10 },
		{ 0.80,  -0.05 },
	};

	auto triangles = triangulate<double, AirfoilSplineSpace> (begin (kSpline.points()), end (kSpline.points()));
	QPolygonF spline_polygon;

	for (auto const& vertex: kSpline.points())
		spline_polygon << QPointF (vertex[0], vertex[1]);

	// Close the spline by adding the first element again:
	auto const v0 = kSpline.points()[0];
	spline_polygon << QPointF (v0[0], v0[1]);

	{
		DummyQApplication app;

		QWidget w (nullptr);
		auto const lh = neutrino::default_line_height (&w);
		size_t draw_triangles = 0;

		neutrino::TestWidget t (QSize (50 * lh, 50 * lh), 0.5_s, [&] (QPaintDevice& canvas) {
			QPainter painter (&canvas);
			painter.fillRect (t.rect(), Qt::black);

			auto const k = 0.5 * std::min (t.width(), t.height());
			painter.translate (0.5 * t.width(), 0.5 * t.height());
			painter.scale (k, -k); // Also fix the Y axis in Qt.

			painter.setPen (QPen (Qt::gray, 1.0 / k));
			painter.drawLine (QPointF (-1.0, 0.0), QPointF (+1.0, 0.0));
			painter.drawLine (QPointF (0.0, -1.0), QPointF (0.0, +1.0));

			// Draw original polygon:
			painter.setPen (QPen (Qt::white, 1.0 / k));
			painter.drawPolyline (spline_polygon);

			// Draw triangles:
			for (size_t i = 0; i < draw_triangles; ++i)
			{
				auto const& triangle = triangles[i];

				QPolygonF polygon ({
					QPointF (triangle[0][0], triangle[0][1]),
					QPointF (triangle[1][0], triangle[1][1]),
					QPointF (triangle[2][0], triangle[2][1]),
					QPointF (triangle[0][0], triangle[0][1]),
				});

				painter.setPen (QPen (Qt::red, 1.0 / k));
				painter.drawPolyline (polygon);

				auto const centroid = triangle_centroid (triangle);
				auto const e = 3 / k;

				painter.setPen (QPen (Qt::green, 1.0 / k));
				painter.drawEllipse (QRectF (QPointF (centroid[0] - e, centroid[1] -e), QPointF (centroid[0] + e, centroid[1] + e)));
			}

			draw_triangles = (draw_triangles + 1) % (triangles.size() + 1);
		});

		t.show();
		app->exec();
	}
});

} // namespace
} // namespace xf::test

