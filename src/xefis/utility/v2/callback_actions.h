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

#ifndef XEFIS__UTILITY__V2__CALLBACK_ACTIONS_H__INCLUDED
#define XEFIS__UTILITY__V2__CALLBACK_ACTIONS_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/v2/actions.h>


namespace x2 {

/**
 * Actions call provided callback when an action happens.
 */
class CallbackAction: public PropAction
{
  public:
	/**
	 * Tell whether condition was met to execute an action.
	 * Additionally call configured callback function.
	 */
	virtual bool
	operator()() override = 0;
};


/**
 * Calls a callback when a property changed its value since last check.
 */
template<class pValue>
	class PropChangedAction:
		public CallbackAction,
		public PropChanged<pValue>
	{
	  public:
		using Value		= pValue;
		using Callback	= std::function<void()>;

	  public:
		/**
		 * \param	property
		 *			Property to observe for changes.
		 * \param	callback
		 *			Function to call when change is detected.
		 */
		PropChangedAction (PropertyIn<bool>& property, Callback callback):
			PropChanged<Value> (property),
			_callback (callback)
		{ }

		// CallbackAction API
		bool
		operator()() override
		{
			bool changed = PropChanged<Value>::operator()();

			if (changed && _callback)
				_callback();

			return changed;
		}

	  private:
		Callback _callback;
	};


/**
 * Calls a callback when a property has changed and has now given value.
 */
template<class pValue>
	class PropChangedToAction:
		public CallbackAction,
		public PropChangedTo<pValue>
	{
	  public:
		using Value		= pValue;
		using Callback	= std::function<void()>;

	  public:
		/**
		 * \param	property
		 *			Property to observe for changes.
		 * \param	callback
		 *			Function to call when change is detected.
		 */
		PropChangedToAction (PropertyIn<bool>& property, Value value, Callback callback):
			PropChangedTo<Value> (property, value),
			_callback (callback)
		{ }

		// CallbackAction API
		bool
		operator()() override
		{
			bool changed = PropChanged<Value>::operator()();

			if (changed && _callback)
				_callback();

			return changed;
		}

	  private:
		Callback _callback;
	};


// TODO document and migrate
#if 0
class ToggleButtonAction: public CallbackAction
{
  public:
	typedef std::function<void (bool)> Callback;

  public:
	// Ctor
	ToggleButtonAction (PropertyPath const& button_path, PropertyPath const& toggle_path)
	{
		_button.set_path (button_path);
		_toggle.set_path (toggle_path);
	}

	/**
	 * Set press callback.
	 */
	void
	set_callback (Callback callback)
	{
		_callback = callback;
	}

	/**
	 * Set to mode that disallows resetting state
	 * by another press.
	 */
	void
	set_radio_mode()
	{
		_radio_mode = true;
	}

	/**
	 * Return true if button is pressed.
	 */
	bool
	pressed() const
	{
		return _button.read (false);
	}

	/**
	 * Set toggle to false.
	 */
	void
	reset()
	{
		_toggle.write (false);
		call();
	}

	/**
	 * Return LED state.
	 */
	bool
	active() const
	{
		return _toggle.read (false);
	}

	/**
	 * Select this option.
	 */
	void
	select()
	{
		if (_radio_mode)
			_toggle.write (true);
		else
			_toggle.write (!_toggle.read (false));
		call();
	}

	/**
	 * Call the callback.
	 */
	void
	call()
	{
		if (_callback)
			_callback (_toggle.read (false));
	}

	// CallbackAction API
	void
	data_updated() override
	{
		if (_button.fresh() && *_button)
			select();
	}

  protected:
	PropertyBoolean	_button;
	PropertyBoolean	_toggle;
	Callback		_callback;
	bool			_radio_mode = false;
};


// TODO document and migrate
class ButtonOptionsAction: public CallbackAction
{
  public:
	class Option
	{
	  public:
		// Ctor
		Option (std::string const& button_path, std::string const& toggle_path, int value, bool is_default = false):
			button (PropertyPath (button_path), PropertyPath (toggle_path)),
			value (value),
			is_default (is_default)
		{
			button.set_radio_mode();
		}

		ToggleButtonAction	button;
		int					value;
		bool				is_default;
	};

	typedef std::vector<Option> Options;

  public:
	// Ctor
	ButtonOptionsAction (PropertyPath const& value_path, Options const& options):
		_options (options)
	{
		_value_target.set_path (value_path);

		// Press the "default" button:
		for (Option& option: _options)
			if (option.is_default)
				option.button.select();
	}

	// CallbackAction API
	void
	data_updated() override
	{
		for (Option& option: _options)
			option.button.data_updated();

		for (Option& option: _options)
		{
			if (option.button.pressed())
			{
				for (Option& option_to_reset: _options)
					if (&option_to_reset != &option)
						option_to_reset.button.reset();
				_value_target.write (option.value);
				break;
			}
		}
	}

  private:
	Options			_options;
	PropertyInteger	_value_target;
};
#endif

} // namespace x2

#endif

