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

#ifndef XEFIS__CORE__COMPONENTS__SOCKET_TREE__SOCKET_TREE_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__SOCKET_TREE__SOCKET_TREE_H__INCLUDED

// Standard:
#include <cstddef>

// Boost:
#include <boost/algorithm/string.hpp>

// Qt:
#include <QTreeWidget>
#include <QTimer>

// Neutrino:
#include <neutrino/qt/qutils.h>
#include <neutrino/sequence.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>

// Local:
#include "socket_item.h"


namespace xf {

class SocketTree: public QWidget
{
  public:
	constexpr static int NameColumn				= 0;
	constexpr static int UseCountColumn			= 1;
	constexpr static int ActualValueColumn		= 2;
	constexpr static int SetValueColumn			= 3;
	constexpr static int FallbackValueColumn	= 4;

  private:
	/**
	 * Helper class used by populate_tree to effectively create a tree of
	 * QTreeItem from a set of xf::Socket objects.
	 */
	class Item
	{
	  public:
		using ChildrenMap = std::map<std::string, Item>;

	  public:
		// Ctor
		explicit
		Item (std::string const& name, QTreeWidgetItem& tree_item):
			_name (name),
			_tree_item (tree_item)
		{ }

		/**
		 * Associated tree-widget item.
		 */
		QTreeWidgetItem&
		tree_item() const noexcept
		{
			return _tree_item;
		}

		/**
		 * Adds children recursively.
		 */
		template<class SocketType>
			void
			add_child (SocketType& socket)
			{
				std::vector<std::string> steps;

				if (auto* module_socket = dynamic_cast<BasicModuleSocket*> (&socket))
					boost::split (steps, module_socket->path().string(), boost::is_any_of ("/"));
				else
					steps = { "free floating" };

				add_child (socket, steps.begin(), steps.end());
			}

	  private:
		template<class SocketType, class Iterator>
			void
			add_child (SocketType& socket, Iterator begin, Iterator end)
			{
				if (begin != end)
				{
					auto const& name = *begin;
					auto found = _children_map.find (name);

					if (found == _children_map.end())
					{
						auto new_item = new SocketItem (std::next (begin) == end ? &socket : nullptr, _tree_item);
						new_item->setText (SocketTree::NameColumn, QString::fromStdString (name));
						setup_appereance (*new_item);
						found = _children_map.try_emplace (name, name, *new_item).first;
					}

					found->second.add_child (socket, std::next (begin), end);
				}
			}

	  private:
		std::string			_name;
		ChildrenMap			_children_map;
		QTreeWidgetItem&	_tree_item;
	};


  public:
	// Ctor
	explicit
	SocketTree (QWidget* parent);

	template<class Iterator>
		void
		populate (Sequence<Iterator> const&);

  private:
	void
	setup_icons();

	void
	read_values();

  protected:
	// QWdiget API
	void
	showEvent (QShowEvent*) override;

	// QWdiget API
	void
	hideEvent (QHideEvent*) override;

  private:
	QTreeWidget*	_tree;
	QTimer*			_refresh_timer;
};


template<class Iterator>
	inline void
	SocketTree::populate (Sequence<Iterator> const& sequence)
	{
		Item root ("Root", *_tree->invisibleRootItem());

		for (auto const& socket: sequence)
			if (auto* basic_socket = dynamic_cast<BasicSocket*> (socket))
				root.add_child (*basic_socket);

		setup_icons();
	}

} // namespace xf

#endif

