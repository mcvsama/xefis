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

#ifndef XEFIS__SUPPORT__SIMULATION__ELECTRICAL__NODE_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__ELECTRICAL__NODE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/noncopyable.h>

// Standard:
#include <cstddef>
#include <string>
#include <string_view>


namespace xf::electrical {

class Element;

class Node: public Noncopyable
{
  public:
	enum Direction: int8_t
	{
		Anode	= +1,
		Cathode	= -1,
	};

  public:
	/**
	 * Creates a free node.
	 */
	explicit
	Node (std::string_view const& name);

	/**
	 * Creates an element-pin-type node.
	 * Element must not outlive the node.
	 */
	explicit
	Node (Element&, Direction);

	/**
	 * Return node name.
	 */
	std::string const&
	name() const noexcept
		{ return _name; }

	/**
	 * Connect another node to this node.
	 */
	void
	operator<< (Node&);

	/**
	 * Return related element, or nullptr if it's not an element-pin-type node.
	 */
	[[nodiscard]]
	Element*
	element() const noexcept
		{ return _element; }

	/**
	 * Return pin type for element-type nodes.
	 */
	[[nodiscard]]
	Direction
	direction() const noexcept
		{ return _direction; }

	/**
	 * Return vector of nodes connected to this node.
	 */
	[[nodiscard]]
	std::vector<Node*> const&
	connected_nodes() const noexcept
		{ return _connected_nodes; }

  private:
	std::string			_name;
	Element*			_element			{ nullptr };
	Direction			_direction			{ Anode };
	std::vector<Node*>	_connected_nodes;
};


inline
Node::Node (std::string_view const& name):
	_name (name)
{ }


inline void
Node::operator<< (Node& node)
{
	_connected_nodes.push_back (&node);
	node._connected_nodes.push_back (this);
}

} // namespace xf::electrical

#endif

