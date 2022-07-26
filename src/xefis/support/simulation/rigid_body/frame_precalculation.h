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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__FRAME_PRECALCULATION_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__FRAME_PRECALCULATION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/connected_bodies.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

/**
 * A struct of data calculated on each simulation frame for certain types of constraints.
 * Instead of having each constraint recalculate commonly required values, calculate them
 * on demand here and let constaints use them.
 *
 * The solver is supposed to reset the data of all registered FramePrecalculations just before
 * calculating constraint forces in each frame.
 */
class BasicFramePrecalculation: public ConnectedBodies
{
  public:
	// Ctor
	explicit
	BasicFramePrecalculation (Body& body_1, Body& body_2);

	// Dtor
	virtual
	~BasicFramePrecalculation() = default;

	/**
	 * Forget the calculated data.
	 */
	virtual void
	reset() = 0;
};


template<class pData>
	class FramePrecalculation: public BasicFramePrecalculation
	{
	  public:
		using Data = pData;

	  public:
		// Ctor
		using BasicFramePrecalculation::BasicFramePrecalculation;

		/**
		 * Access calculated data.
		 * If it's not calculated, calls calculate() first.
		 */
		Data const&
		data();

		/**
		 * Access calculated data.
		 * If it doesn't exist, return nullopt.
		 */
		std::optional<Data> const&
		data() const
			{ return _data; }

		// BasicFramePrecalculation API
		void
		reset() override
			{ _data.reset(); }

	  protected:
		/**
		 * Calculate the required data.
		 */
		virtual void
		calculate (Data&) = 0;

	  private:
		std::optional<Data>	_data;
	};


inline
BasicFramePrecalculation::BasicFramePrecalculation (Body& body_1, Body& body_2):
	ConnectedBodies (body_1, body_2)
{ }


template<class D>
	inline auto
	FramePrecalculation<D>::data() -> Data const&
	{
		if (!_data)
		{
			_data = Data();
			calculate (*_data);
		}

		return *_data;
	}

} // namespace xf::rigid_body

#endif

