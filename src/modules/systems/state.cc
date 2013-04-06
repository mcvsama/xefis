/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include <memory>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/exception.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "state.h"


State::ManagedBoolean::ManagedBoolean (QDomElement const& element)
{
	if (!element.hasAttribute ("path"))
		throw Xefis::Exception ("property element needs @path attribute");

	_path = element.attribute ("path");
	_property = Xefis::PropertyBoolean (_path.toStdString());

	for (QDomElement const& e: element)
	{
		if (e == "default")
		{
			_default = e.text() == "true";
			_property.write (_default);
		}
		else
			throw Xefis::Exception (QString ("property '%1': unsupported element <%2>").arg (_path).arg (e.tagName()).toStdString());
	}

	process();
}


void
State::ManagedBoolean::process()
{
	if (_property.is_singular())
		return;

	if (_property.is_nil())
		_property.write (_default);
}


State::ManagedInteger::ManagedInteger (QDomElement const& element)
{
	if (!element.hasAttribute ("path"))
		throw Xefis::Exception ("property element needs @path attribute");

	_path = element.attribute ("path");
	_property = Xefis::PropertyInteger (_path.toStdString());

	for (QDomElement const& e: element)
	{
		if (e == "min")
			_min = e.text().toInt();
		else if (e == "max")
			_max = e.text().toInt();
		else if (e == "default")
		{
			_default = e.text().toInt();
			_property.write (_default);
		}
		else if (e == "winding")
			_winding = e.text() == "true";
		else
			throw Xefis::Exception (QString ("property '%1': unsupported element <%2>").arg (_path).arg (e.tagName()).toStdString());
	}

	if (_min > _max)
		throw Xefis::Exception (QString ("property '%1' min (%2) is greated than max (%3)").arg (_path).arg (_max).arg (_min).toStdString());

	process();
}


void
State::ManagedInteger::process()
{
	if (_property.is_singular())
		return;

	if (_property.is_nil())
		_property.write (_default);

	Xefis::PropertyInteger::Type value = _property.read();

	if (_winding)
		value = floored_mod (value - _min, _max - _min + 1) + _min;
	else
		value = limit (value, _min, _max);

	if (value != _property.read())
		_property.write (value);
}


State::ManagedFloat::ManagedFloat (QDomElement const& element)
{
	if (!element.hasAttribute ("path"))
		throw Xefis::Exception ("property element needs @path attribute");

	_path = element.attribute ("path");
	_property = Xefis::PropertyFloat (_path.toStdString());

	for (QDomElement const& e: element)
	{
		if (e == "min")
			_min = e.text().toInt();
		else if (e == "max")
			_max = e.text().toInt();
		else if (e == "default")
		{
			_default = e.text().toInt();
			_property.write (_default);
		}
		else
			throw Xefis::Exception (QString ("property '%1': unsupported element <%2>").arg (_path).arg (e.tagName()).toStdString());
	}

	if (_min > _max)
		throw Xefis::Exception (QString ("property '%1' min (%2) is greated than max (%3)").arg (_path).arg (_max).arg (_min).toStdString());

	process();
}


void
State::ManagedFloat::process()
{
	if (_property.is_singular())
		return;

	if (_property.is_nil())
		_property.write (_default);

	Xefis::PropertyFloat::Type value = _property.read();

	value = limit (value, _min, _max);

	if (value != _property.read())
		_property.write (value);
}


State::ManagedString::ManagedString (QDomElement const& element)
{
	if (!element.hasAttribute ("path"))
		throw Xefis::Exception ("property element needs @path attribute");

	_path = element.attribute ("path");
	_property = Xefis::PropertyString (_path.toStdString());

	for (QDomElement const& e: element)
	{
		if (e == "default")
		{
			_default = e.text();
			_property.write (_default.toStdString());
		}
		else
			throw Xefis::Exception (QString ("property '%1': unsupported element <%2>").arg (_path).arg (e.tagName()).toStdString());
	}

	process();
}


void
State::ManagedString::process()
{
	if (_property.is_singular())
		return;

	if (_property.is_nil())
		_property.write (_default.toStdString());
}


State::ObservedProperty::ObservedProperty (State* state, QDomElement const& element):
	_state (state),
	_path (element.attribute ("path"))
{
	if (element.hasAttribute ("path"))
		_path = element.attribute ("path");
	else if (element.hasAttribute ("var"))
		_path = _state->get_path_for_var (element.attribute ("var"));
	else
		throw Xefis::Exception ("<observe> element needs @path or @var attribute");

	_state->parse_instructions (element, _instructions);

	std::string spath (_path.toStdString());
	_prop_boolean = Xefis::PropertyBoolean (spath);
	_prop_integer = Xefis::PropertyInteger (spath);
	_prop_float = Xefis::PropertyFloat (spath);
	_prop_string = Xefis::PropertyString (spath);

	if (_prop_boolean.valid())
	{
		_prev_boolean = *_prop_boolean;
		_prev_integer = *_prop_integer;
		_prev_float = *_prop_float;
		_prev_string = *_prop_string;
	}
}


State::ObservedProperty::~ObservedProperty()
{
	for (auto* i: _instructions)
		delete i;
}


void
State::ObservedProperty::process()
{
	// Skip if property doesn't exist:
	if (_prop_boolean.is_singular() || _prop_boolean.is_nil())
		return;

	bool changed = false;

	switch (_prop_boolean.real_type())
	{
		case Xefis::PropBoolean:
			changed = *_prop_boolean != _prev_boolean;
			break;

		case Xefis::PropInteger:
			changed = *_prop_integer != _prev_integer;
			break;

		case Xefis::PropFloat:
			changed = *_prop_float != _prev_float;
			break;

		case Xefis::PropString:
			changed = *_prop_string != _prev_string;
			break;

		case Xefis::PropDirectory:
			throw Xefis::Exception (QString ("can't observe directory property '%1'").arg (_path).toStdString());
	}

	_prev_boolean = *_prop_boolean;
	_prev_integer = *_prop_integer;
	_prev_float = *_prop_float;
	_prev_string = *_prop_string;

	if (changed)
	{
		for (Instruction* i: _instructions)
			i->process();
	}
}


State::IfInstruction::IfInstruction (State* state, QDomElement const& element):
	_state (state)
{
	_state->parse_instructions (element, _instructions);

	if (element.hasAttribute ("path"))
		_path = element.attribute ("path");
	else if (element.hasAttribute ("var"))
		_path = _state->get_path_for_var (element.attribute ("var"));
	else
		throw Xefis::Exception ("<observe> element needs @path or @var attribute");

	if (element.hasAttribute ("equals"))
	{
		_comparison = Equals;
		_value = element.attribute ("equals");
	}
	else if (element.hasAttribute ("greater-than"))
	{
		_comparison = GreaterThan;
		_value = element.attribute ("greater-than");
	}
	else if (element.hasAttribute ("greater-or-equals"))
	{
		_comparison = GreaterOrEquals;
		_value = element.attribute ("greater-or-equals");
	}
	else if (element.hasAttribute ("less-than"))
	{
		_comparison = LessThan;
		_value = element.attribute ("less-than");
	}
	else if (element.hasAttribute ("less-or-equals"))
	{
		_comparison = LessOrEquals;
		_value = element.attribute ("less-or-equals");
	}
	else
		throw Xefis::Exception (
			QString ("<%1> instructions needs one of attributes: equals, greater-than, greater-or-equals, less-than, less-or-equals")
				.arg (element.tagName()).toStdString()
		);

	std::string spath (_path.toStdString());
	_prop_boolean = Xefis::PropertyBoolean (spath);
	_prop_integer = Xefis::PropertyInteger (spath);
	_prop_float = Xefis::PropertyFloat (spath);
	_prop_string = Xefis::PropertyString (spath);
}


State::IfInstruction::~IfInstruction()
{
	for (auto* i: _instructions)
		delete i;
}


void
State::IfInstruction::process()
{
	_result = false;

	if (!_prop_boolean.valid())
		return;

	switch (_prop_boolean.real_type())
	{
		case Xefis::PropBoolean:
			_result = evaluate_operator (static_cast<int> (*_prop_integer), static_cast<int> (_value == "true"));
			break;

		case Xefis::PropInteger:
			_result = evaluate_operator (*_prop_integer, _value.toInt());
			break;

		case Xefis::PropFloat:
			_result = evaluate_operator (*_prop_float, _value.toDouble());
			break;

		case Xefis::PropString:
			_result = evaluate_operator (*_prop_string, _value.toStdString());
			break;

		case Xefis::PropDirectory:
			throw Xefis::Exception (QString ("can't evaluate truth value for directory node '%1'").arg (_path).toStdString());
	}

	if (_result)
	{
		for (Instruction* i: _instructions)
			i->process();
	}
}


template<class Type>
	bool
	State::IfInstruction::evaluate_operator (Type value, Type test) const
	{
		switch (_comparison)
		{
			case Equals:
				return value == test;
			case GreaterThan:
				return value > test;
			case GreaterOrEquals:
				return value >= test;
			case LessThan:
				return value < test;
			case LessOrEquals:
				return value <= test;
		}

		return false;
	}


State::ChooseInstruction::ChooseInstruction (State* state, QDomElement const& element):
	_state (state)
{
	for (QDomElement const& e: element)
	{
		if (e == "when")
			_whens.push_back (new IfInstruction (state, e));
		else if (e == "otherwise")
			_state->parse_instructions (e, _otherwise);
	}
}


State::ChooseInstruction::~ChooseInstruction()
{
	for (auto* w: _whens)
		delete w;
	for (auto* i: _otherwise)
		delete i;
}


void
State::ChooseInstruction::process()
{
	bool _match = false;

	for (IfInstruction* if_instruction: _whens)
	{
		if_instruction->process();

		if (if_instruction->result())
		{
			_match = true;
			break;
		}
	}

	if (!_match)
		for (Instruction* i: _otherwise)
			i->process();
}


State::ModifyInstruction::ModifyInstruction (State* state, QDomElement const& element):
	_state (state)
{
	if (element == "toggle")
		_type = Toggle;
	else if (element == "add")
		_type = Add;
	else if (element == "sub")
		_type = Sub;
	else if (element == "set")
		_type = Set;
	else
		throw Xefis::Exception (QString ("unsupported instruction <%1>").arg (element.tagName()).toStdString());

	_path = element.attribute ("path");
	_value = element.attribute ("value");
}


void
State::ModifyInstruction::process()
{
	std::string spath = _path.toStdString();
	Xefis::PropertyBoolean prop_boolean (spath);

	if (prop_boolean.is_nil())
		return;

	Xefis::PropertyType prop_type = prop_boolean.real_type();
	int sgn = 1;

	switch (_type)
	{
		case Toggle:
			switch (prop_type)
			{
				case Xefis::PropBoolean:
				{
					Xefis::PropertyBoolean prop (spath);
					prop.write (!prop.read());
					break;
				}

				default:
					throw Xefis::Exception ("'toggle' action can be only used on boolean properties");
			}
			break;

		case Sub:
			sgn = -1;
		case Add:
			switch (prop_type)
			{
				case Xefis::PropInteger:
				{
					Xefis::PropertyInteger prop (spath);
					prop.write (*prop + sgn * _value.toInt());
					break;
				}

				case Xefis::PropFloat:
				{
					Xefis::PropertyFloat prop (spath);
					prop.write (*prop + sgn * _value.toDouble());
					break;
				}

				default:
					throw Xefis::Exception ("'add'/'sub' actions can be only used on integer and float properties");
			}
			break;

		case Set:
			switch (prop_type)
			{
				case Xefis::PropBoolean:
				{
					Xefis::PropertyBoolean prop (spath);
					prop.write (_value == "true");
					break;
				}

				case Xefis::PropInteger:
				{
					Xefis::PropertyInteger prop (spath);
					prop.write (_value.toInt());
					break;
				}

				case Xefis::PropFloat:
				{
					Xefis::PropertyInteger prop (spath);
					prop.write (_value.toDouble());
					break;
				}

				case Xefis::PropString:
				{
					Xefis::PropertyString prop (spath);
					prop.write (_value.toStdString());
					break;
				}

				default:
					throw Xefis::Exception ("'set' action can be only used on boolean, integer, float or string properties");
			}
			break;
	}

	_state->state_changed();
}


State::State (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager)
{
	for (QDomElement const& e: config)
	{
		if (e == "property")
		{
			if (e.attribute ("type") == "boolean")
				_managed_properties.insert (new ManagedBoolean (e));
			else if (e.attribute ("type") == "integer")
				_managed_properties.insert (new ManagedInteger (e));
			else if (e.attribute ("type") == "float")
				_managed_properties.insert (new ManagedFloat (e));
			else if (e.attribute ("type") == "string")
				_managed_properties.insert (new ManagedString (e));
		}
		else if (e == "var")
			_vars[e.attribute ("name")] = e.attribute ("path");
		else if (e == "observe")
			_observed_properties.insert (new ObservedProperty (this, e));
	}
}


State::~State()
{
	for (auto* p: _managed_properties)
		delete p;
	for (auto* p: _observed_properties)
		delete p;
}


void
State::data_updated()
{
	_state_changed = false;
	process_observed_properties();
	process_managed_properties();
	if (_state_changed)
		signal_data_updated();
}


QString
State::get_path_for_var (QString const& var_name) const
{
	auto v = _vars.find (var_name);
	if (v == _vars.end())
		throw Xefis::Exception (QString ("could not find variable '%1'").arg (var_name).toStdString());
	return v->second;
}


void
State::parse_instructions (QDomElement const& element, Instructions& instructions)
{
	for (QDomElement const& e: element)
	{
		if (e == "if")
			instructions.push_back (new IfInstruction (this, e));
		else if (e == "choose")
			instructions.push_back (new ChooseInstruction (this, e));
		else if (e == "toggle" || e == "set" || e == "add" || e == "sub")
			instructions.push_back (new ModifyInstruction (this, e));
		else
			throw Xefis::Exception (QString ("unknown instruction <%1>").arg (e.tagName()).toStdString());
	}
}


void
State::process_observed_properties()
{
	for (ObservedProperty* op: _observed_properties)
		op->process();
}


void
State::process_managed_properties()
{
	for (ManagedProperty* mp: _managed_properties)
		mp->process();
}


void
State::state_changed()
{
	_state_changed = true;
}

