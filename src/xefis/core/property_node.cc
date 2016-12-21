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

// Standard:
#include <cstddef>
#include <algorithm>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property_node.h"


namespace xf {

void
PropertyNode::update_path()
{
	PropertyStorage* storage = this->storage();

	if (storage)
		storage->uncache_path (_path);

	_path = _parent
		? PropertyPath (_parent->path().string() + "/" + _name)
		: PropertyPath (_name);

	if (storage)
		storage->cache_path (this);
}


PropertyNodeList
PropertyDirectoryNode::children() const
{
	return _children;
}


PropertyNode*
PropertyDirectoryNode::child (std::string const& name)
{
	auto it = _children_by_name.find (name);
	if (it == _children_by_name.end())
		return nullptr;
	// @it itself points to a list iterator.
	return *it->second;
}


PropertyNode*
PropertyDirectoryNode::locate (PropertyPath const& path)
{
	std::string const& path_str = path.string();

	if (path_str.empty())
		return this;

	// If we are root node, try searching PropertyStorage cache first.
	// Normalize path before searching:
	if (!_parent && _storage)
	{
		PropertyNode* node = _storage->locate (path_str[0] == '/' ? path : PropertyPath ('/' + path_str));
		if (node)
			return node;
		else
			// If not found by absolute path in Storage, then it doesn't exist:
			return nullptr;
	}

	if (path_str[0] == '/')
		return root()->locate (PropertyPath (path_str.substr (1)));

	std::string::size_type slash = path_str.find ('/');
	std::string segment = path_str.substr (0, slash);
	std::string rest;
	if (slash != std::string::npos)
		rest = path_str.substr (slash + 1);

	if (segment == ".")
		return locate (PropertyPath (rest));
	else if (segment == "..")
	{
		if (_parent)
			return _parent->locate (PropertyPath (rest));
		return nullptr;
	}
	else
	{
		PropertyNode* c = child (segment);
		if (c)
		{
			PropertyDirectoryNode* dir = dynamic_cast<PropertyDirectoryNode*> (c);
			if (dir)
				return dir->locate (PropertyPath (rest));
			else
				return nullptr;
		}
		else
			return nullptr;
	}
}


PropertyDirectoryNode*
PropertyDirectoryNode::mkpath (PropertyPath const& path)
{
	std::string const& path_str = path.string();

	if (path_str.empty())
		return this;

	if (path_str[0] == '/')
		return root()->mkpath (PropertyPath (path_str.substr (1)));

	std::string::size_type slash = path_str.find ('/');
	std::string segment = path_str.substr (0, slash);
	std::string rest;
	if (slash != std::string::npos)
		rest = path_str.substr (slash + 1);

	if (segment == ".")
		return mkpath (PropertyPath (rest));
	else if (segment == "..")
	{
		if (!_parent)
			throw PropertyPathConflict ("couldn't reach above the top node");
		return _parent->mkpath (PropertyPath (rest));
	}
	else
	{
		PropertyNode* c = child (segment);
		if (!c)
		{
			PropertyDirectoryNode* dir = new PropertyDirectoryNode (segment);
			add_child (dir);
			return dir->mkpath (PropertyPath (rest));
		}
		else
		{
			PropertyDirectoryNode* dir = dynamic_cast<PropertyDirectoryNode*> (c);
			if (dir)
				return dir->mkpath (PropertyPath (rest));
			else
				throw PropertyPathConflict ("can't create directory path, would conflict with intermediate node: "_str + path_str);
		}
	}
}


PropertyNode*
PropertyDirectoryNode::add_child (PropertyNode* child)
{
	if (child->parent())
		child->parent()->remove_child (child);
	child->_parent = this;

	_children.push_back (child);
	_children_by_name[child->name()] = --_children.end();

	child->update_path();

	return child;
}


void
PropertyDirectoryNode::remove_child (PropertyNode* child)
{
	auto it = std::find (_children.begin(), _children.end(), child);
	if (it != _children.end())
	{
		_children.erase (it);
		_children_by_name.erase (child->name());

		child->_parent = nullptr;
	}

	child->update_path();
}


void
PropertyDirectoryNode::clear()
{
	for (auto c: _children)
		delete c;

	_children.clear();
	_children_by_name.clear();
}

} // namespace xf

