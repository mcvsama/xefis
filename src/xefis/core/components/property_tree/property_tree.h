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

#ifndef XEFIS__CORE__COMPONENTS__PROPERTY_TREE__PROPERTY_TREE_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__PROPERTY_TREE__PROPERTY_TREE_H__INCLUDED

// Standard:
#include <cstddef>

// Boost:
#include <boost/algorithm/string.hpp>

// Qt:
#include <QTreeWidget>
#include <QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/utility/qutils.h>
#include <xefis/utility/sequence.h>

// Local:
#include "property_item.h"


namespace xf {

class PropertyTree: public QWidget
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
	 * QTreeItem from a set of xf::Property objects.
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
		template<class Property>
			void
			add_child (Property& property)
			{
				std::vector<std::string> steps;
				boost::split (steps, property.path().string(), boost::is_any_of ("/"));
				add_child (property, steps.begin(), steps.end());
			}

	  private:
		template<class Property, class Iterator>
			void
			add_child (Property& property, Iterator begin, Iterator end)
			{
				if (begin != end)
				{
					auto const& name = *begin;
					auto found = _children_map.find (name);

					if (found == _children_map.end())
					{
						auto new_item = new PropertyItem (std::next (begin) == end ? &property : nullptr, _tree_item);
						new_item->setText (PropertyTree::NameColumn, QString::fromStdString (name));
						setup_appereance (*new_item);
						found = _children_map.try_emplace (name, name, *new_item).first;
					}

					found->second.add_child (property, std::next (begin), end);
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
	PropertyTree (QWidget* parent);

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
	PropertyTree::populate (Sequence<Iterator> const& sequence)
	{
		Item root ("Root", *_tree->invisibleRootItem());

		for (auto const& property: sequence)
			if (auto* basic_property = dynamic_cast<BasicProperty*> (property))
				root.add_child (*basic_property);

		setup_icons();
	}

} // namespace xf

#endif

