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

#ifndef XEFIS__SUPPORT__UI__GL_H__INCLUDED
#define XEFIS__SUPPORT__UI__GL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/shape.h>
#include <xefis/support/simulation/rigid_body/shape_material.h>
#include <xefis/support/simulation/rigid_body/shape_vertex.h>

// System:
#define GL_GLEXT_PROTOTYPES // What kind of joke is that?
#include <GL/gl.h>

// Standard:
#include <cstddef>
#include <initializer_list>
#include <memory>


namespace xf {

/**
 * Unique pointer that's also implicitly convertible to an array.
 * Useful for OpenGL functions that take arrays.
 */
template<class Value>
	class GLArray
	{
	  public:
		// Ctor
		GLArray (std::initializer_list<Value>);

		/**
		 * Access given element.
		 */
		Value&
		operator[] (std::size_t index) noexcept
			{ return data()[index]; }

		/**
		 * Access given element.
		 */
		Value const&
		operator[] (std::size_t index) const noexcept
			{ return data()[index]; }

		/**
		 * Access the array.
		 */
		Value*
		data() const noexcept
			{ return _ptr.get(); }

		/**
		 * Access the array.
		 */
		operator Value*() const
			{ return _ptr.get(); }

	  private:
		std::unique_ptr<Value[]> _ptr;
	};


/**
 * Support for various OpenGL stuff.
 */
class GLSpace
{
  public:
	// Ctor
	GLSpace (decltype (1 / 1_m) position_scale);

	/**
	 * Add given offset to all vertex positions that are drawn.
	 */
	void
	set_global_offset (SpaceLength<rigid_body::BodySpace> const& offset) noexcept;

	/**
	 * Store current OpenGL matrix, call the lambda and restore matrix.
	 * Exception-safe.
	 */
	static void
	save_matrix (auto&& lambda);

	/**
	 * Convert QColor to array of floats for OpenGL functions.
	 */
	[[nodiscard]]
	static GLArray<float>
	to_opengl (QColor const&);

	/**
	 * Return ShapeMaterial for given color.
	 */
	[[nodiscard]]
	rigid_body::ShapeMaterial
	make_material (QColor const&);

	/**
	 * Set perspective parameters.
	 */
	static void
	set_hfov_perspective (QSize, si::Angle hfov, float near_plane, float far_plane);

	/**
	 * Rotate current OpenGL matrix by given rotation matrix.
	 */
	static void
	rotate (RotationMatrix<auto, auto> const&);

	/**
	 * Rotate current OpenGL matrix by given angle about given vector.
	 */
	static void
	rotate (si::Angle const angle, float x, float y, float z);

	/**
	 * Translate current OpenGL matrix by given vector.
	 */
	static void
	translate (float x, float y, float z);

	/**
	 * Translate current OpenGL matrix by given vector.
	 */
	void
	translate (si::Length x, si::Length y, si::Length z);

	/**
	 * Translate current OpenGL matrix by given vector.
	 */
	static void
	translate (SpaceVector<double, auto>);

	/**
	 * Translate current OpenGL matrix by given vector.
	 */
	void
	translate (SpaceVector<si::Length, auto>);

	/**
	 * Return value in OpenGL coordinates.
	 */
	float
	to_opengl (si::Length const value)
		{ return value * _position_scale; }

	/**
	 * Return value in OpenGL coordinates.
	 */
	SpaceVector<double, rigid_body::BodySpace>
	vector_to_opengl (SpaceLength<rigid_body::BodySpace> const value)
		{ return value * _position_scale; }

	/**
	 * Call glBegin(), the lambda and glEnd().
	 * Exception-safe.
	 */
	static void
	begin (GLenum mode, auto&& lambda);

	/**
	 * Set current OpenGL normal vector to the vertex' normal.
	 */
	static void
	set_normal (rigid_body::ShapeVertex const&);

	/**
	 * Set current OpenGL material parameters.
	 */
	static void
	set_material (rigid_body::ShapeMaterial const&);

	/**
	 * Set current OpenGL material/normal from vertex parameters.
	 */
	static void
	set_vertex (rigid_body::ShapeVertex const&);

	/**
	 * Add vertex with its normal and material information.
	 */
	void
	add_vertex (rigid_body::ShapeVertex const&);

	/**
	 * Add OpenGL vertex at given position.
	 */
	void
	add_vertex (SpaceVector<double, auto> position);

	/**
	 * Add OpenGL vertex at given position.
	 */
	void
	add_vertex (SpaceVector<si::Length, auto> position);

	/**
	 * Draw given shape in OpenGL.
	 */
	void
	draw (rigid_body::Shape const& shape);

  private:
	SpaceLength<rigid_body::BodySpace>			_global_offset;
	SpaceVector<double, rigid_body::BodySpace>	_global_offset_float;
	decltype (1 / 1_m)							_position_scale			{ 1 };
};


template<class Value>
	inline
	GLArray<Value>::GLArray (std::initializer_list<Value> list)
	{
		_ptr = std::make_unique<Value[]> (list.size());
		std::copy (list.begin(), list.end(), _ptr.get());
	}


inline
GLSpace::GLSpace (decltype (1 / 1_m) position_scale):
	_position_scale (position_scale)
{ }


inline void
GLSpace::set_global_offset (SpaceLength<rigid_body::BodySpace> const& offset) noexcept
{
	_global_offset = offset;
	_global_offset_float = offset * _position_scale;
}


inline void
GLSpace::save_matrix (auto&& lambda)
{
	try {
		glPushMatrix();
		lambda();
		glPopMatrix();
	}
	catch (...)
	{
		glPopMatrix();
		throw;
	}
}


inline GLArray<float>
GLSpace::to_opengl (QColor const& color)
{
	return {
		color.red() / 255.0f,
		color.green() / 255.0f,
		color.blue() / 255.0f,
		color.alpha() / 255.0f,
	};
}


inline void
GLSpace::rotate (RotationMatrix<auto, auto> const& r)
{
	std::array<double, 16> const array {
		r (0, 0), r (1, 0), r (2, 0), 0.0,
		r (0, 1), r (1, 1), r (2, 1), 0.0,
		r (0, 2), r (1, 2), r (2, 2), 0.0,
		0.0,      0.0,      0.0,      1.0,
	};

	glMultMatrixd (array.data());
}


inline void
GLSpace::rotate (si::Angle const angle, float x, float y, float z)
{
	glRotatef (angle.in<si::Degree>(), x, y, z);
}


inline void
GLSpace::translate (float x, float y, float z)
{
	glTranslatef (x, y, z);
}


inline void
GLSpace::translate (si::Length x, si::Length y, si::Length z)
{
	glTranslatef (x * _position_scale, y * _position_scale, z * _position_scale);
}


inline void
GLSpace::translate (SpaceVector<double, auto> offset)
{
	glTranslatef (offset[0], offset[1], offset[2]);
}


inline void
GLSpace::translate (SpaceVector<si::Length, auto> offset)
{
	translate (offset * _position_scale);
}


inline void
GLSpace::begin (GLenum const mode, auto&& lambda)
{
	try {
		glBegin (mode);
		lambda();
		glEnd();
	}
	catch (...)
	{
		glEnd();
		throw;
	}
}


inline void
GLSpace::set_normal (rigid_body::ShapeVertex const& vertex)
{
	if (auto const opt_n = vertex.normal())
	{
		auto const& n = *opt_n;
		glNormal3f (n[0], n[1], n[2]);
	}
}


inline void
GLSpace::set_vertex (rigid_body::ShapeVertex const& vertex)
{
	set_normal (vertex);
	set_material (vertex.material());
}


inline void
GLSpace::add_vertex (rigid_body::ShapeVertex const& vertex)
{
	set_vertex (vertex);
	add_vertex (vertex.position());
}


inline void
GLSpace::add_vertex (SpaceVector<double, auto> position)
{
	position += _global_offset_float;
	glVertex3f (position[0], position[1], position[2]);
}


inline void
GLSpace::add_vertex (SpaceVector<si::Length, auto> position)
{
	position += _global_offset;
	add_vertex (position * _position_scale);
}

} // namespace xf

#endif

