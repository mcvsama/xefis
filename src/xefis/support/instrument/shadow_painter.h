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

#ifndef XEFIS__SUPPORT__INSTRUMENT__SHADOW_PAINTER_H__INCLUDED
#define XEFIS__SUPPORT__INSTRUMENT__SHADOW_PAINTER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/instrument/shadow.h>

// Qt:
#include <QtGui/QPainter>

// Standard:
#include <cstddef>


namespace xf {

class ShadowPainter: virtual public QPainter
{
  public:
	using DefaultPaintFunction	= std::function<void()>;
	using PaintFunction			= std::function<void (bool painting_shadow)>;

  public:
	// Ctor
	ShadowPainter() = default;

	// Ctor
	explicit
	ShadowPainter (QPaintDevice&);

	/**
	 * Add a shadow under painted primitives.
	 * The PaintFunction will be called twice with different state of the
	 * painter to "repaint" the shadow.
	 */
	void
	paint (Shadow const&, PaintFunction);

	/**
	 * Overloaded for convenience.
	 */
	void
	paint (Shadow const&, DefaultPaintFunction);
};


inline
ShadowPainter::ShadowPainter (QPaintDevice& device):
	QPainter (&device)
{ }


inline void
ShadowPainter::paint (Shadow const& shadow, DefaultPaintFunction paint_function)
{
	paint (shadow, [&paint_function] (bool) { paint_function(); });
}

} // namespace xf

#endif

