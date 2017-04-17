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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qdom_iterator.h>

// Local:
#include "string_translator.h"


XEFIS_REGISTER_MODULE_CLASS ("helpers/string-translator", StringTranslator)


StringTranslator::StringsSet::StringsSet (QDomElement const& config)
{
	if (!config.hasAttribute ("input-path"))
		throw xf::MissingDomAttribute (config, "input-path");
	if (!config.hasAttribute ("output-path"))
		throw xf::MissingDomAttribute (config, "output-path");

	_input.set_path (xf::PropertyPath (config.attribute ("input-path")));
	_output.set_path (xf::PropertyPath (config.attribute ("output-path")));

	for (QDomElement const& e: xf::iterate_sub_elements (config))
	{
		if (e == "string")
		{
			if (!e.hasAttribute ("input"))
				throw xf::MissingDomAttribute (e, "input");
			if (!e.hasAttribute ("output"))
				throw xf::MissingDomAttribute (e, "output");
			_map.insert ({ e.attribute ("input").toLong(), e.attribute ("output").toStdString() });
		}
		else if (e == "default")
		{
			if (!e.hasAttribute ("output"))
				throw xf::MissingDomAttribute (e, "output");
			_default = e.attribute ("output").toStdString();
		}
	}

	update();
}


void
StringTranslator::StringsSet::process()
{
	if (_input.fresh())
		update();
}


void
StringTranslator::StringsSet::update()
{
	if (_input.valid())
	{
		Map::iterator it = _map.find (*_input);
		if (it != _map.end())
			_output.write (it->second);
		else
			_output.write (_default);
	}
	else
		_output.write (_default);
}


StringTranslator::StringTranslator (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	for (QDomElement const& e: xf::iterate_sub_elements (config))
	{
		if (e == "translate")
			_sets.emplace_back (e);
	}
}


void
StringTranslator::data_updated()
{
	for (auto& s: _sets)
		s.process();
}

