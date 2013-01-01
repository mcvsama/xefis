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
 * --
 * Here be basic, global functions and macros like asserts, debugging helpers, etc.
 */

#ifndef XEFIS__CONFIG__RESOURCES_H__INCLUDED
#define XEFIS__CONFIG__RESOURCES_H__INCLUDED

// Qt:
#include <QtGui/QPixmap>
#include <QtGui/QPixmapCache>

// Xefis:
#include <xefis/config/all.h>


namespace Config {

enum {
	Spacing			= 3,
	Margin			= 2,
	SmallSpacing	= 2,
	WindowMargin	= 4,
	DialogMargin	= 6
};

} // namespace Config


#ifndef XEFIS_PREFIX
#define XEFIS_PREFIX ""
#endif

#ifndef XEFIS_SHARED_DIRECTORY
#define XEFIS_SHARED_DIRECTORY XEFIS_PREFIX "share"
#endif

#ifndef XEFIS_XDG_SETTINGS_HOME
#define XEFIS_XDG_SETTINGS_HOME "mulabs.org/xefis"
#endif

#ifndef XEFIS_XDG_DATA_HOME
#define XEFIS_XDG_DATA_HOME "mulabs.org/xefis"
#endif


namespace Resources {

/*
 * Icons resources
 */

#define XEFIS_CONFIG_HAS_ICON(key, file)				\
	static inline QPixmap key()							\
	{													\
		QPixmap p;										\
		if (!QPixmapCache::find (#key, p))				\
		{												\
			p = QPixmap (file);							\
			QPixmapCache::insert (#key, p);				\
		}												\
		return p;										\
	}

	namespace Icons16
	{
#define XEFIS_CONFIG_HAS_ICON_16(key, file) \
		XEFIS_CONFIG_HAS_ICON(key, XEFIS_SHARED_DIRECTORY "/images/16/" file)

		XEFIS_CONFIG_HAS_ICON_16 (property_dir, "property-dir.png");
		XEFIS_CONFIG_HAS_ICON_16 (property_value, "property-value.png");

#undef XEFIS_CONFIG_HAS_ICON_16
	} // namespace Icons16

	namespace Icons22
	{
#define XEFIS_CONFIG_HAS_ICON_22(key, file) \
		XEFIS_CONFIG_HAS_ICON(key, XEFIS_SHARED_DIRECTORY "/images/22/" file)

#undef XEFIS_CONFIG_HAS_ICON_22
	} // namespace Icons22

#undef XEFIS_CONFIG_HAS_ICON

} // namespace Config

#endif

