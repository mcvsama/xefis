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

#ifndef NEUTRINO__CPP20_H__INCLUDED
#define NEUTRINO__CPP20_H__INCLUDED

namespace std {

template<class T>
	struct remove_cvref
	{
		typedef std::remove_cv_t<std::remove_reference_t<T>> type;
	};


template<class T>
	using remove_cvref_t = typename remove_cvref<T>::type;

} // namespace std

#endif

