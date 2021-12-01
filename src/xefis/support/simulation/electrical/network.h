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

#ifndef XEFIS__SUPPORT__SIMULATION__ELECTRICAL__NETWORK_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__ELECTRICAL__NETWORK_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>
#include <string_view>
#include <vector>

// Neutrino:
#include <neutrino/noncopyable.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/electrical/element.h>
#include <xefis/support/simulation/electrical/node.h>


namespace xf::electrical {

class Network: public Noncopyable
{
  public:
	using Elements	= std::vector<std::unique_ptr<Element>>;
	using Nodes		= std::list<Node>;

  public:
	/**
	 * Make new node and return its reference.
	 */
	[[nodiscard]]
	Node&
	make_node (std::string_view const& name);

	/**
	 * Add element to the network.
	 */
	template<ElementConcept SpecificElement, class ...Args>
		[[nodiscard]]
		SpecificElement&
		add (Args&&...);

	/**
	 * Add element to the network.
	 */
	template<ElementConcept SpecificElement>
		[[nodiscard]]
		SpecificElement&
		add (std::unique_ptr<SpecificElement>&&);

	/**
	 * Return list of elements in the network.
	 */
	Elements const&
	elements() const noexcept
		{ return _elements; }

	/**
	 * Return list of free nodes in the network (that is not ones that belong to elements).
	 */
	Nodes const&
	nodes() const noexcept
		{ return _free_nodes; }

	/**
	 * Reduce number of nodes by replacing a set of connected nodes with a single node.
	 * Invalidates all node references obtained through make_node().
	 * Normally user shouldn't have to use it, as it will be used by Solver.
	 */
	void
	simplify();

  private:
	Elements	_elements;
	// This should not contain Element-pin-type nodes, only free nodes:
	Nodes		_free_nodes;
};


inline Node&
Network::make_node (std::string_view const& name)
{
	_free_nodes.emplace_back (name);
	return _free_nodes.back();
}


template<ElementConcept SpecificElement, class ...Args>
	inline SpecificElement&
	Network::add (Args&& ...args)
	{
		return add (std::make_unique<SpecificElement> (std::forward<Args> (args)...));
	}


template<ElementConcept SpecificElement>
	inline SpecificElement&
	Network::add (std::unique_ptr<SpecificElement>&& element)
	{
		_elements.push_back (std::move (element));
		return static_cast<SpecificElement&> (*_elements.back());
	}

} // namespace xf::electrical

#endif

