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

// Local:
#include "node_voltage_solver.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/electrical/exception.h>

// Neutrino:
#include <neutrino/stdexcept.h>

// Standard:
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>


namespace xf::electrical {

template<class Function, class ...Args>
	inline void
	NodeVoltageSolver::SNetwork::for_each_node (Function&& function, Args&& ...args)
	{
		for (auto& node: nodes)
			function (node, std::forward<Args> (args)...);
	}


template<class Function, class ...Args>
	inline void
	NodeVoltageSolver::SNetwork::for_each_edge (Function&& function, Args&& ...args)
	{
		for (auto& dir_edge: a_k_dir_edges)
			function (*dir_edge, std::forward<Args> (args)...);
	}


template<class Function, class ...Args>
	inline void
	NodeVoltageSolver::SNetwork::for_each_a_k_dir_voltage_source (Function&& function, Args&& ...args)
	{
		for (auto* dir_edge: a_k_dir_edges)
			if (dir_edge->edge->element->type() == Element::VoltageSource)
				function (*dir_edge, std::forward<Args> (args)...);
	}


template<class Function, class ...Args>
	inline void
	NodeVoltageSolver::SNetwork::for_each_a_k_dir_load (Function&& function, Args&& ...args)
	{
		for (auto* dir_edge: a_k_dir_edges)
			if (dir_edge->edge->element->type() == Element::Load)
				function (*dir_edge, std::forward<Args> (args)...);
	}


void
NodeVoltageSolver::evolve (si::Time const dt)
{
	flow_current (dt);
	static_cast<void> (solve());
}


bool
NodeVoltageSolver::solve()
{
	_converged = false;
	return _converged = solve (_snetwork, _accuracy / _snetwork.a_k_dir_edges.size(), _max_iterations, false);
}


void
NodeVoltageSolver::solve_throwing()
{
	try {
		static_cast<void> (solve (_snetwork, _accuracy / _snetwork.a_k_dir_edges.size(), _max_iterations, true));
	}
	catch (...)
	{
		_converged = false;
		throw;
	}
}


bool
NodeVoltageSolver::solve (SNetwork& network, double const accuracy, uint32_t max_iterations, bool throwing)
{
	bool accuracy_satisfied = false;
	si::Voltage max_voltage_error = 0_V;
	si::Current max_current_error = 0_A;

	for (uint32_t i = 0; i < max_iterations; ++i)
	{
		max_voltage_error = 0_V;
		max_current_error = 0_A;

		network.for_each_edge (&NodeVoltageSolver::auto_adjust_edge_voltage, max_voltage_error);
		network.for_each_edge (&NodeVoltageSolver::auto_adjust_edge_current, max_current_error);
		network.for_each_node (&NodeVoltageSolver::auto_adjust_node_currents, max_current_error);

		accuracy_satisfied = (abs (max_voltage_error) <= abs (1_V * accuracy)) && (abs (max_current_error) <= abs (1_A * accuracy));

		if (accuracy_satisfied)
			break;
	}

	bool converged = !!accuracy_satisfied;

	// Transfer calculated values of voltage and current to elements:
	for (auto const* dir_edge: network.a_k_dir_edges)
	{
		Element& element = *dir_edge->edge->element;

		element.set_current (dir_edge->edge->a_k_current);
		element.set_voltage (voltage_a_k (dir_edge));
	}

	if (!converged)
	{
		std::cout << "NOT CONVERGED   voltage_error = " << max_voltage_error << "; required = " << 1_V * accuracy << "     current_error = " << max_current_error << "; required = " << 1_A * accuracy << "\n";

		if (throwing)
			throw NotConverged ("simulation solution did not converge; best accuracy = " + to_string (max_voltage_error) + "/" + to_string (max_current_error));
	}

	return converged;
}


void
NodeVoltageSolver::flow_current (si::Time const dt) const
{
	for (auto& edge: _snetwork.edges)
		edge.element->flow_current (dt);
}


void
NodeVoltageSolver::simplify (Network const& network, SNetwork& snetwork)
{
	if (network.nodes().empty())
		return;

	auto snodes = std::unordered_map<Node const*, SNode*>();
	auto unvisited_nodes = std::unordered_set<Node const*>();
	auto unvisited_elements = std::unordered_set<Element*>();

	for (auto const& node: network.nodes())
		unvisited_nodes.insert (&node);

	for (auto const& element: network.elements())
		unvisited_elements.insert (element.get());

	// Important! Must not allow vectors to realloc (it will break the references),
	// so preallocate their sizes now.
	// We could use std::list, but std::vector is more cache-friendly.
	snetwork.nodes.reserve (network.nodes().size());
	snetwork.edges.reserve (network.elements().size());
	snetwork.dir_edges.reserve (2 * network.elements().size());
	snetwork.a_k_dir_edges.reserve (network.elements().size());

	// Passing nullptr as node creates dangling SNode:
	auto const add_snode = [&] (Node const* node) -> SNode&
	{
		snetwork.nodes.push_back (SNode());
		SNode& snode = snetwork.nodes.back();

		if (node)
		{
			snode.name = node->name();
			snodes[node] = &snode;
		}

		return snode;
	};

	// For each node, create a single SNode and iterate all nodes directly connected to the node.
	// Add a mapping between the s-node and node in "network.nodes".
	{
		while (!unvisited_nodes.empty())
		{
			Node const& node = **unvisited_nodes.begin();
			SNode& snode = add_snode (&node);
			unvisited_nodes.erase (unvisited_nodes.begin());

			for (auto const& connected_node: node.connected_nodes())
			{
				if (!connected_node->element())
				{
					snodes[connected_node] = &snode;
					unvisited_nodes.erase (connected_node);
				}
			}
		}
	}

	// Transfer element edges from Nodes to SNodes.
	{
		while (!unvisited_elements.empty())
		{
			Element& element = **unvisited_elements.begin();

			unvisited_elements.erase (unvisited_elements.begin());

			auto const& node_a = element.anode();
			auto const& node_c = element.cathode();

			// Make sure that element nodes have only 1 connection, to a normal node:
			for (auto* node: { &node_a, &node_c })
				if (node->connected_nodes().size() > 1)
					throw InvalidArgument ("Element Node " + node->name() + " has too many connections, maximum 1 allowed");

			SNode* snode_a = node_a.connected_nodes().empty()
				// This creates dangling edge:
				? &add_snode (nullptr)
				: snodes.at (node_a.connected_nodes().at (0));

			SNode* snode_c = node_c.connected_nodes().empty()
				// This creates dangling edge:
				? &add_snode (nullptr)
				: snodes.at (node_c.connected_nodes().at (0));

			snetwork.edges.push_back (SEdge { &element, 0_A });
			SEdge& sedge = snetwork.edges.back();

			snetwork.dir_edges.push_back (SDirEdge { &sedge, Node::Anode, snode_a, snode_c, nullptr });
			auto& dir_edge_a = snetwork.dir_edges.back();
			snode_a->dir_edges.push_back (&dir_edge_a);

			snetwork.a_k_dir_edges.push_back (&dir_edge_a);

			snetwork.dir_edges.push_back (SDirEdge { &sedge, Node::Cathode, snode_c, snode_a, nullptr });
			auto& dir_edge_c = snetwork.dir_edges.back();
			snode_c->dir_edges.push_back (&dir_edge_c);

			dir_edge_a.other_dir_edge = &dir_edge_c;
			dir_edge_c.other_dir_edge = &dir_edge_a;
		}
	}
}


si::Voltage
NodeVoltageSolver::adjust_voltage (si::Voltage& v1, si::Voltage& v2, si::Voltage const required_voltage)
{
	si::Voltage const old_voltage = v2 - v1;
	si::Voltage const error = old_voltage - required_voltage;
	si::Voltage const half_error = 0.5 * error;

	v2 -= half_error;
	v1 += half_error;

	return required_voltage - old_voltage;
}


si::Voltage
NodeVoltageSolver::adjust_a_k_voltage (SDirEdge& dir_edge, si::Voltage const required_voltage)
{
	SNode& tn = *dir_edge.this_node;
	SNode& on = *dir_edge.other_node;

	return adjust_voltage (on.voltage, tn.voltage, dir_edge.direction * required_voltage);
}


si::Current
NodeVoltageSolver::adjust_a_k_current (SDirEdge& dir_edge, si::Current const required_a_k_current)
{
	auto old = std::exchange (dir_edge.edge->a_k_current, average (dir_edge.edge->a_k_current, required_a_k_current));
	return dir_edge.edge->a_k_current - old;
}


void
NodeVoltageSolver::auto_adjust_edge_voltage (SDirEdge& dir_edge, si::Voltage& voltage_error)
{
	auto const& edge = *dir_edge.edge;
	auto const new_voltage = edge.element->voltage_for_current (edge.a_k_current);
	auto const delta = adjust_a_k_voltage (dir_edge, new_voltage);

	maximize_error (voltage_error, delta);
}


void
NodeVoltageSolver::auto_adjust_edge_current (SDirEdge& dir_edge, si::Current& current_error)
{
	auto& edge = *dir_edge.edge;
	auto const new_current = edge.element->current_for_voltage (voltage_a_k (dir_edge));
	auto const delta = adjust_a_k_current (dir_edge, new_current);

	maximize_error (current_error, delta);
}


void
NodeVoltageSolver::auto_adjust_node_currents (SNode& node, si::Current& current_error)
{
	// Calculate sum of the currents, it will be error that we try to reduce:
	si::Current i_node_error = 0_A;

	// Negative i_node_error means there's current missing in the node, positive - it's too much.
	for (auto* dir_edge: node.dir_edges)
		i_node_error += dir_edge->direction * -dir_edge->edge->a_k_current;

	si::Conductance sum_conductance = 0 / 1_Ohm;

	auto resistance_of = [](auto* dir_edge) {
		auto const& element = *dir_edge->edge->element;

		if (element.has_const_resistance() || element.type() == Element::VoltageSource)
			return element.resistance();
		else
		{
			// FIXME Unimplemented, needed for non-lienar elements:
			auto const u = voltage_a_k (dir_edge);
			auto const i = element.current_for_voltage (u);
			auto r = -dir_edge->direction * u / i; // FIXME might be -dir_edge->direction

			if (!isfinite (u) || u == 0_V || !isfinite (i) || i == 0_A)
				r = 1_Ohm;

			return r;
		}
	};

	for (auto* dir_edge: node.dir_edges)
		sum_conductance += 1 / resistance_of (dir_edge);

	for (auto* dir_edge: node.dir_edges)
	{
		si::Current const delta_i = i_node_error * 1 / (resistance_of (dir_edge) * sum_conductance);
		dir_edge->edge->a_k_current += dir_edge->direction * delta_i;
		maximize_error (current_error, delta_i);
	}
}


inline si::Voltage
NodeVoltageSolver::relative_voltage_on (SDirEdge const* dir_edge)
{
	return relative_voltage_on (*dir_edge);
}


inline si::Voltage
NodeVoltageSolver::relative_voltage_on (SDirEdge const& dir_edge)
{
	return dir_edge.other_node->voltage - dir_edge.this_node->voltage;
}


inline si::Voltage
NodeVoltageSolver::voltage_a_k (SDirEdge const* dir_edge)
{
	return voltage_a_k (*dir_edge);
}


inline si::Voltage
NodeVoltageSolver::voltage_a_k (SDirEdge const& dir_edge)
{
	return dir_edge.direction * (dir_edge.this_node->voltage - dir_edge.other_node->voltage);
}

} // namespace xf::electrical

