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

#ifndef XEFIS__SUPPORT__SIMULATION__ELECTRICAL__NODE_VOLTAGE_SOLVER_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__ELECTRICAL__NODE_VOLTAGE_SOLVER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/electrical/element.h>
#include <xefis/support/simulation/electrical/network.h>
#include <xefis/support/simulation/electrical/node.h>

// Neutrino:
#include <neutrino/noncopyable.h>

// Standard:
#include <cstddef>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>


namespace xf::electrical {

/**
 * Solves voltages on electrical loads using loop current method and numerical approach.
 *
 * Solver must not outlive network or its components.
 * Solver will not reflect changes after network is reconfigured. A new Solver must be created
 * after changes are made to the network.
 */
class NodeVoltageSolver: public Noncopyable
{
  public:
	static constexpr uint32_t kDefaultMaxIterations = 10000;

  private:
	class SNode;

	class SEdge
	{
	  public:
		Element*	element;
		// Current from anode to cathode:
		si::Current	a_k_current;
	};

	class SDirEdge
	{
	  public:
		SEdge*		edge;
		int8_t		direction; // +1 is Anode to Cathode, -1 is otherwise.
		SNode*		this_node;
		SNode*		other_node;
		SDirEdge*	other_dir_edge;
	};

	class SNode
	{
	  public:
		std::string				name;
		std::vector<SDirEdge*>	dir_edges;
		si::Voltage				voltage;
	};

	class SNetwork: public Noncopyable
	{
	  public:
		std::vector<SNode>		nodes;
		std::vector<SEdge>		edges;
		std::vector<SDirEdge>	dir_edges;
		// Contains only anode→cathode directional edges (from anode to cathode):
		std::vector<SDirEdge*>	a_k_dir_edges;

	  public:
		/**
		 * Iterate over all nodes and call "function" on each.
		 */
		template<class Function, class ...Args>
			void
			for_each_node (Function&& function, Args&& ...args);

		/**
		 * Iterate over all nodes and call "function" on each.
		 */
		template<class Function, class ...Args>
			void
			for_each_edge (Function&& function, Args&& ...args);

		/**
		 * Iterate over anode→cathode-directioned voltage sources and call "function" on each.
		 */
		template<class Function, class ...Args>
			void
			for_each_a_k_dir_voltage_source (Function&& function, Args&& ...args);

		/**
		 * Iterate over anode→cathode-directioned load elements and call "function" on each.
		 */
		template<class Function, class ...Args>
			void
			for_each_a_k_dir_load (Function&& function, Args&& ...args);
	};

	struct TraversalStep
	{
		SNode*	node_1;
		SEdge*	edge;
		SNode*	node_2;
	};

	using TraversalPath = std::vector<TraversalStep>;

  public:
	/**
	 * Ctor
	 *
	 * \param	Network
	 *			Electrical network to analyze.
	 * \param	accuracy
	 *			Required voltage accuracy and current of each element.
	 * \throws	std::logic_error
	 *			On various occasions
	 */
	explicit
	NodeVoltageSolver (Network const&, double accuracy, uint32_t max_iterations = kDefaultMaxIterations);

	/**
	 * Solve the network voltages. It must be called before evolve() if changes have been
	 * made to the network elements (changed voltages, resistances, etc).
	 *
	 * \returns	true if solution converges before reaching iterations limit; false otherwise.
	 * \throws	InvalidArgument
	 *			On network errors.
	 */
	[[nodiscard]]
	bool
	solve();

	/**
	 * Version of solve that throws an exception if there's no convergence.
	 *
	 * \throws	NotConverged
	 *			When calculations do not converge.
	 */
	void
	solve_throwing();

	/**
	 * Evolve the state of the network (flow current and recalculate voltages).
	 * Ignores convergence errors.
	 */
	void
	evolve (si::Time dt);

	/**
	 * Return true if last solution converged.
	 */
	[[nodiscard]]
	bool
	converged() const noexcept
		{ return _converged; }

  private:
	[[nodiscard]]
	static bool
	solve (SNetwork& network, double accuracy, uint32_t max_iterations, bool throwing);

	/**
	 * Flow current through elements.
	 */
	void
	flow_current (si::Time dt) const;

	/**
	 * Create an SNetwork from Network. SNetwork is used in calculations.
	 * Original network is not modified.
	 *
	 * Specifically: simplify given network (join connected nodes into single node). Normally a user can create multiple nodes and connect them together, but
	 * to simplify calculations, such set of connected nodes should be reduced to a single node with connections to multiple elements.
	 */
	static void
	simplify (Network const& network, SNetwork& result);

	/**
	 * Adjust both v1 and v2 so that v2 - v1 == required_voltage.
	 *
	 * \returns	difference between new and old voltage.
	 */
	static si::Voltage
	adjust_voltage (si::Voltage& v1, si::Voltage& v2, si::Voltage required_voltage);

	/**
	 * Adjust voltage on the element edge to closer match required_voltage.
	 *
	 * \returns	difference between new and old voltage.
	 */
	static si::Voltage
	adjust_a_k_voltage (SDirEdge& dir_edge, si::Voltage required_voltage);

	/**
	 * Adjust current flowing through edge to closer match required_current.
	 *
	 * \returns	difference between new and old current.
	 */
	static si::Current
	adjust_a_k_current (SDirEdge& edge, si::Current required_current);

	/**
	 * Adjust voltage on an edge depending on type of element.
	 *
	 * \param	voltage_error, current_error
	 *			Values to be updated to reflect maximum encountered error during computation.
	 */
	static void
	auto_adjust_edge_voltage (SDirEdge&, si::Voltage& voltage_error);

	/**
	 * Adjust current on an edge depending on type of element.
	 *
	 * \param	voltage_error, current_error
	 *			Values to be updated to reflect maximum encountered error during computation.
	 */
	static void
	auto_adjust_edge_current (SDirEdge&, si::Current& current_error);

	/**
	 * Adjust currents on edges connected to given node, so that total sum is 0 A.
	 *
	 * \param	voltage_error, current_error
	 *			Values to be updated to reflect maximum encountered error during computation.
	 */
	static void
	auto_adjust_node_currents (SNode&, si::Current& current_error);

	/**
	 * Return voltage difference between nodes of given SDirEdge:
	 * remote_node voltage - this node voltage.
	 */
	[[nodiscard]]
	static si::Voltage
	relative_voltage_on (SDirEdge const&);

	/**
	 * Convenience overload
	 */
	[[nodiscard]]
	static si::Voltage
	relative_voltage_on (SDirEdge const*);

	/**
	 * Return voltage difference between nodes of given SDirEdge:
	 * cathode voltage - anode voltage.
	 */
	[[nodiscard]]
	static si::Voltage
	voltage_a_k (SDirEdge const&);

	/**
	 * Convenience overload
	 */
	[[nodiscard]]
	static si::Voltage
	voltage_a_k (SDirEdge const*);

	/**
	 * Return average of two values.
	 */
	template<class Value>
		static Value
		average (Value old_value, Value new_value)
			{ return 0.5 * (old_value + new_value); }

	/**
	 * Set error to be maximum of the two values.
	 */
	template<class Value>
		static void
		maximize_error (Value& current_error, Value const new_error)
			{ current_error = std::max (current_error, abs (new_error)); }

  private:
	SNetwork	_snetwork;
	double		_accuracy;
	uint32_t	_max_iterations;
	bool		_converged		{ false };
};


inline
NodeVoltageSolver::NodeVoltageSolver (Network const& network, double const accuracy, uint32_t max_iterations):
	_accuracy (accuracy),
	_max_iterations (max_iterations)
{
	simplify (network, _snetwork);
	static_cast<void> (solve());
}

} // namespace xf::electrical

#endif

