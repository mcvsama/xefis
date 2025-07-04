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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__FRAME_PRECOMPUTATION_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__FRAME_PRECOMPUTATION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/connected_bodies.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

/**
 * A struct of data computed on each simulation frame for certain types of constraints.
 * Instead of having each constraint recompute commonly required values, compute them
 * on demand here and let constraints use them.
 *
 * The solver is supposed to reset the data of all registered FramePrecomputations just before
 * calculating constraint forces in each frame.
 */
class BasicFramePrecomputation: public ConnectedBodies
{
  public:
	// Ctor
	explicit
	BasicFramePrecomputation (Body& body_1, Body& body_2);

	// Dtor
	virtual
	~BasicFramePrecomputation() = default;

	/**
	 * Forget the computed data.
	 */
	virtual void
	reset() = 0;
};


template<class pData>
	class FramePrecomputation: public BasicFramePrecomputation
	{
	  public:
		using Data = pData;

	  public:
		// Ctor
		using BasicFramePrecomputation::BasicFramePrecomputation;

		/**
		 * Access computed data.
		 * If it's not computed, calls compute() first.
		 */
		Data const&
		data();

		/**
		 * Access computed data.
		 * If it doesn't exist, return nullopt.
		 */
		std::optional<Data> const&
		data() const
			{ return _data; }

		// BasicFramePrecomputation API
		void
		reset() override
			{ _data.reset(); }

	  protected:
		/**
		 * Calculate the required data.
		 */
		virtual void
		compute (Data&) = 0;

	  private:
		std::optional<Data>	_data;
	};


inline
BasicFramePrecomputation::BasicFramePrecomputation (Body& body_1, Body& body_2):
	ConnectedBodies (body_1, body_2)
{ }


template<class D>
	inline auto
	FramePrecomputation<D>::data() -> Data const&
	{
		if (!_data)
		{
			_data = Data();
			compute (*_data);
		}

		return *_data;
	}

} // namespace xf::rigid_body

#endif

