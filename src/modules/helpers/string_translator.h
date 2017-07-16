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

#ifndef XEFIS__MODULES__HELPERS__STRING_TRANSLATOR_H__INCLUDED
#define XEFIS__MODULES__HELPERS__STRING_TRANSLATOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module.h>
#include <xefis/core/v1/property.h>


/**
 * Map input strings to output strings.
 */
class StringTranslator: public v1::Module
{
	/**
	 * Represents set of one translations
	 * (integer to strings).
	 */
	class StringsSet
	{
		typedef std::map<v1::PropertyInteger::Type, v1::PropertyString::Type> Map;

	  public:
		// Ctor
		StringsSet (QDomElement const& config);

		/**
		 * Process translation. Call update() if input property has changed.
		 */
		void
		process();

	  private:
		/**
		 * Do the update on output property.
		 */
		void
		update();

	  private:
		v1::PropertyInteger			_input;
		v1::PropertyString			_output;
		Map							_map;
		v1::PropertyString::Type	_default;
	};

  public:
	// Ctor
	StringTranslator (v1::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	std::vector<StringsSet>	_sets;
};

#endif
