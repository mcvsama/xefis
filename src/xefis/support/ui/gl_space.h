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

#ifndef XEFIS__SUPPORT__UI__GL_SPACE_H__INCLUDED
#define XEFIS__SUPPORT__UI__GL_SPACE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/simulation/rigid_body/shape.h>
#include <xefis/support/simulation/rigid_body/shape_material.h>
#include <xefis/support/simulation/rigid_body/shape_vertex.h>

// Qt:
#include <QSize>

// System:
#define GL_GLEXT_PROTOTYPES // What kind of joke is that?
#include <GL/gl.h>

// Standard:
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <optional>
#include <stack>


namespace xf {

/**
 * Unique pointer that's also implicitly convertible to an array.
 * Useful for OpenGL functions that take arrays.
 */
template<class Value, std::size_t Size>
	class GLArray: public std::array<Value, Size>
	{
	  public:
		template<std::same_as<Value> ...Args>
			requires (sizeof...(Args) == Size)
			GLArray (Args&& ...args):
				std::array<Value, Size> { std::forward<Args> (args)... }
			{ }

		/**
		 * Access the array.
		 */
		operator Value*()
			{ return this->data(); }

		/**
		 * Access the array.
		 */
		operator Value const*() const
			{ return this->data(); }
	};


// Deduction guide to allow writing GLArray { 1, 2, 3 } without explicitly
// stating the Value and Size parameters.
template<class... Args>
	GLArray (Args&&...) -> GLArray<std::common_type_t<Args...>, sizeof...(Args)>;


using GLMatrix = std::array<GLfloat, 16>;


/**
 * Support for various OpenGL stuff.
 * Adds a concept of camera and allows double-precision translation of camera-related stuff before forwarding translation to OpenGL library (which uses floats).
 */
class GLSpace
{
  public:
	struct AdditionalParameters
	{
		std::optional<GLColor>	color_override;
	};

  public:
	// Ctor
	GLSpace (decltype (1 / 1_m) position_scale = 1.0 / 1_m);

	/**
	 * Add given offset to all vertex positions that are drawn.
	 */
	void
	set_global_offset (SpaceLength<WorldSpace> const& offset) noexcept;

	/**
	 * Store current OpenGL matrix, call the lambda and restore matrix.
	 * Exception-safe.
	 */
	void
	save_context (auto&& lambda);

	/**
	 * Set perspective parameters.
	 */
	static void
	set_hfov_perspective (QSize, si::Angle hfov, float near_plane, float far_plane);

	/**
	 * Set camera placement. If enabled (non-nullopt Placement passed), the
	 * camera will be at the origin on OpenGL space, and all added objects
	 * will be translated by -camera_position.
	 */
	void
	set_camera (std::optional<Placement<WorldSpace, WorldSpace>> const& camera);

	/**
	 * Set camera rotations. Position will be set to 0.
	 */
	void
	set_camera_rotation_only (Placement<WorldSpace, WorldSpace>);

	/**
	 * Rotate current OpenGL matrix by given rotation quaternion.
	 */
	template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
		static void
		rotate (RotationQuaternion<TargetSpace, SourceSpace> const&);

	/**
	 * Rotate current OpenGL matrix by given rotation matrix.
	 */
	template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
		static void
		rotate (RotationMatrix<TargetSpace, SourceSpace> const&);

	/**
	 * Rotate current OpenGL matrix by given angle about given vector.
	 */
	template<math::CoordinateSystem Space>
		static void
		rotate (si::Angle const angle, SpaceVector<double, Space> const& normalized_axis)
			{ rotate (angle, normalized_axis.x(), normalized_axis.y(), normalized_axis.z()); }

	/**
	 * Rotate current OpenGL matrix by given angle about given vector.
	 */
	static void
	rotate (si::Angle const angle, float x, float y, float z)
		{ glRotatef (angle.in<si::Degree>(), x, y, z); }

	/**
	 * Convenience shortcut.
	 */
	static void
	rotate_x (si::Angle const angle)
		{ rotate (angle, 1, 0, 0); }

	/**
	 * Convenience shortcut.
	 */
	static void
	rotate_y (si::Angle const angle)
		{ rotate (angle, 0, 1, 0); }

	/**
	 * Convenience shortcut.
	 */
	static void
	rotate_z (si::Angle const angle)
		{ rotate (angle, 0, 0, 1); }

	/**
	 * Translate current OpenGL matrix by given vector.
	 */
	static void
	translate (float x, float y, float z)
		{ glTranslatef (x, y, z); }

	/**
	 * Translate current OpenGL matrix by given vector.
	 */
	void
	translate (si::Length x, si::Length y, si::Length z)
		{ glTranslatef (x * _position_scale, y * _position_scale, z * _position_scale); }

	/**
	 * Translate current OpenGL matrix by given vector.
	 */
	template<math::CoordinateSystem Space>
		static void
		translate (SpaceVector<double, Space> const& offset)
			{ glTranslatef (offset[0], offset[1], offset[2]); }

	/**
	 * Translate current OpenGL matrix by given vector.
	 */
	template<math::CoordinateSystem Space>
		void
		translate (SpaceVector<si::Length, Space> const& offset)
			{ translate (offset * _position_scale); }

	/**
	 * Apply translation and rotation.
	 */
	template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
		void
		transform (Placement<BaseSpace, Space> const&);

	/**
	 * Disable translation by zeroing last row and column of the transformation matrix.
	 */
	void
	reset_translation();

	/**
	 * Return value in OpenGL coordinates.
	 */
	float
	to_opengl (si::Length const value)
		{ return value * _position_scale; }

	/**
	 * Return value in OpenGL coordinates.
	 */
	SpaceVector<double, WorldSpace>
	vector_to_opengl (SpaceLength<WorldSpace> const value)
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
	void
	set_material (rigid_body::ShapeMaterial const&);

	/**
	 * Set current OpenGL material/normal from vertex parameters.
	 */
	void
	set_vertex (rigid_body::ShapeVertex const&);

	/**
	 * Add vertex with its normal and material information.
	 */
	void
	add_vertex (rigid_body::ShapeVertex const&);

	/**
	 * Add OpenGL vertex at given position.
	 */
	template<math::CoordinateSystem Space>
		void
		add_vertex (SpaceVector<double, Space> position);

	/**
	 * Add OpenGL vertex at given position.
	 */
	template<math::CoordinateSystem Space>
		void
		add_vertex (SpaceVector<si::Length, Space> position)
			{ add_vertex (position * _position_scale); }

	/**
	 * Return reference to current additional parameters struct.
	 * It gets saved/restored with save_context().
	 */
	AdditionalParameters&
	additional_parameters();

	/**
	 * Draw given shape in OpenGL.
	 */
	void
	draw (rigid_body::Shape const& shape);

	void
	clear_z_buffer (float value = 1.0f);

	void
	set_wireframe_enabled (bool enabled)
		{ glPolygonMode (GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL); }

	static void
	load_identity()
		{ glLoadIdentity(); }

	static GLMatrix
	extract_modelview_matrix();

	static void
	multiply_matrix_by (GLMatrix const& matrix)
		{ glMultMatrixf (matrix.data()); }

  private:
	void
	push_context();

	void
	pop_context();

  private:
	std::optional<Placement<WorldSpace, WorldSpace>>
										_camera;
	decltype (1 / 1_m)					_position_scale					{ 1 };
	std::stack<AdditionalParameters>	_additional_parameters_stack;
};


inline void
GLSpace::save_context (auto&& lambda)
{
	try {
		push_context();
		lambda();
		pop_context();
	}
	catch (...)
	{
		pop_context();
		throw;
	}
}


template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	inline void
	GLSpace::rotate (RotationQuaternion<TargetSpace, SourceSpace> const& q)
	{
		// People on the Internet suggest not to use glRotatef()
		// but instead to convert the quaternion to a rotation matrix and
		// use matrix multiplication. Since using glRotatef() indeed
		// yields weird results, I'm fine with the matrix.
		rotate (RotationMatrix (q));
	}


template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	inline void
	GLSpace::rotate (RotationMatrix<TargetSpace, SourceSpace> const& r)
	{
		std::array<double, 16> const array {
			r[0, 0], r[1, 0], r[2, 0], 0.0,
			r[0, 1], r[1, 1], r[2, 1], 0.0,
			r[0, 2], r[1, 2], r[2, 2], 0.0,
			0.0,     0.0,     0.0,     1.0,
		};

		glMultMatrixd (array.data());
	}


template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
	inline void
	GLSpace::transform (Placement<BaseSpace, Space> const& placement)
	{
		translate (placement.position());
		// With OpenGL it's base→body, not body→space as one would normally expect:
		rotate (placement.base_to_body_rotation());
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
	add_vertex (math::coordinate_system_cast<WorldSpace, void> (vertex.position()));
}


template<math::CoordinateSystem Space>
	inline void
	GLSpace::add_vertex (SpaceVector<double, Space> position)
	{
		if (_camera)
			position -= _camera->position() * _position_scale;

		glVertex3f (position[0], position[1], position[2]);
	}


inline GLMatrix
GLSpace::extract_modelview_matrix()
{
	GLMatrix matrix;
	glGetFloatv (GL_MODELVIEW_MATRIX, matrix.data());
	return matrix;
}

} // namespace xf

#endif

