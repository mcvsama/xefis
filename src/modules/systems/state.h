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

#ifndef XEFIS__MODULES__COMPUTERS__STATE_H__INCLUDED
#define XEFIS__MODULES__COMPUTERS__STATE_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QString>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


class State: public Xefis::Module
{
  private:
	class ManagedProperty;
	class ObservedProperty;
	class Instruction;

	typedef std::map<QString, QString>	Vars;
	typedef std::set<ManagedProperty*>	ManagedProperties;
	typedef std::set<ObservedProperty*>	ObservedProperties;
	typedef std::list<Instruction*>		Instructions;

	class ManagedProperty
	{
	  public:
		// Dtor
		virtual ~ManagedProperty();

		/**
		 * Return path to the property.
		 */
		QString const&
		path() const noexcept;

		/**
		 * Process the property (like limit its value,
		 * etc.
		 */
		virtual void
		process() = 0;
	};

	class ManagedInteger: public ManagedProperty
	{
	  public:
		// Ctor
		ManagedInteger (QDomElement const& element);

		// From ManagedProperty
		void
		process() override;

	  private:
		QString					_path;
		int						_min		= 0;
		int						_max		= 0;
		int						_default	= 0;
		bool					_winding	= false;
		Xefis::PropertyInteger	_property;
	};

	class ObservedProperty
	{
	  public:
		// Ctor
		ObservedProperty (State*, QDomElement const&);

		// Dtor
		~ObservedProperty();

		/**
		 * Check if property has changed and process conditions.
		 */
		void
		process();

	  private:
		State*							_state;
		QString							_path;
		Instructions					_instructions;
		Xefis::PropertyBoolean			_prop_boolean;
		Xefis::PropertyInteger			_prop_integer;
		Xefis::PropertyFloat			_prop_float;
		Xefis::PropertyString			_prop_string;
		Xefis::PropertyBoolean::Type	_prev_boolean;
		Xefis::PropertyInteger::Type	_prev_integer;
		Xefis::PropertyFloat::Type		_prev_float;
		Xefis::PropertyString::Type		_prev_string;
	};

	class Instruction
	{
	  public:
		// Dtor
		virtual ~Instruction();

		/**
		 * Check for condition and process actions.
		 * \param	observed_property_path Path to the examined property.
		 */
		virtual void
		process() = 0;
	};

	class IfInstruction: public Instruction
	{
	  public:
		enum Comparison { Equals, GreaterThan, GreaterOrEquals, LessThan, LessOrEquals };

	  public:
		// Ctor
		IfInstruction (State*, QDomElement const&);

		// Dtor
		~IfInstruction();

		// From Instruction
		void
		process() override;

		/**
		 * Return if the condition was true after most recent
		 * call of process().
		 */
		bool
		result() const;

	  private:
		/**
		 * Compare values using operator defined by _type.
		 */
		template<class Type>
			bool
			evaluate_operator (Type value, Type test) const;

	  private:
		State*					_state;
		QString					_path;
		Comparison				_comparison;
		QString					_value;
		Instructions			_instructions;
		bool					_result = false;
		Xefis::PropertyBoolean	_prop_boolean;
		Xefis::PropertyInteger	_prop_integer;
		Xefis::PropertyFloat	_prop_float;
		Xefis::PropertyString	_prop_string;
	};

	class ChooseInstruction: public Instruction
	{
	  public:
		// Ctor
		ChooseInstruction (State*, QDomElement const&);

		// Dtor
		~ChooseInstruction();

		// From Instruction
		void
		process() override;

	  private:
		State*						_state;
		std::list<IfInstruction*>	_whens;
		Instructions				_otherwise;
	};

	class ModifyInstruction: public Instruction
	{
	  public:
		enum Type { Toggle, Set, Add, Sub };

	  public:
		// Ctor
		ModifyInstruction (State*, QDomElement const&);

		// From Instruction
		void
		process() override;

	  private:
		State*	_state;
		Type	_type;
		QString	_path;
		QString	_value;
	};

  public:
	// Ctor
	State (Xefis::ModuleManager*, QDomElement const& config);

	// Dtor
	~State();

  protected:
	void
	data_updated() override;

  private:
	/**
	 * Get property path for given var name.
	 * Throw Xefis::Exception if no such var was defined.
	 */
	QString
	get_path_for_var (QString const& var_name) const;

	/**
	 * Parse instructions and put them to the output @instructions list.
	 */
	void
	parse_instructions (QDomElement const& element, Instructions& instructions);

	/**
	 * Process all observed properties and execute actions.
	 */
	void
	process_observed_properties();

	/**
	 * Process all properties to have a well-defined state.
	 */
	void
	process_managed_properties();

  private:
	Vars				_vars;
	ManagedProperties	_managed_properties;
	ObservedProperties	_observed_properties;
};


inline
State::ManagedProperty::~ManagedProperty()
{ }


inline
State::Instruction::~Instruction()
{ }


inline bool
State::IfInstruction::result() const
{
	return _result;
}

#endif
