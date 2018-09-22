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

#ifndef XEFIS__CORE__SCREEN_H__INCLUDED
#define XEFIS__CORE__SCREEN_H__INCLUDED

// Standard:
#include <atomic>
#include <cstddef>
#include <optional>
#include <vector>

// Qt:
#include <QSize>
#include <QImage>
#include <QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/logger.h>
#include <xefis/core/machine.h>
#include <xefis/core/screen_spec.h>
#include <xefis/support/system/work_performer.h>
#include <xefis/utility/named_instance.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/tracker.h>


namespace xf {
namespace detail {

/**
 * Additional information for each instrument needed by the Screen object,
 * such as its position on the screen.
 */
class InstrumentDetails
{
  public:
	BasicInstrument&			instrument;
	QRectF						requested_position;
	QPointF						anchor_position;
	std::optional<QRect>		computed_position;
	QSize						previous_size;
	int							z_index			{ 0 };
	// This future returns time it took to paint the instrument:
	std::future<si::Time>		result;
	// The canvas and canvas_to_use constitute a double-buffer. std::unique_ptr<> is used
	// since it's not known if std::swap() on QImages is fast or not.
	std::unique_ptr<QImage>		canvas;
	std::unique_ptr<QImage>		canvas_to_use;
	WorkPerformer*				work_performer;

  public:
	// Ctor
	explicit
	InstrumentDetails (BasicInstrument&, WorkPerformer& work_performer);
};

} // namespace detail


class Machine;


/**
 * Collects instrument images and composites them onto its own area.
 */
class Screen:
	public QWidget,
	public NamedInstance,
	private Noncopyable
{
	Q_OBJECT

  private:
	using InstrumentTracker = Tracker<BasicInstrument, detail::InstrumentDetails>;

  public:
	// Ctor
	explicit
	Screen (ScreenSpec const&, Graphics const&, Machine&, std::string_view const& instance, Logger const&);

	// Dtor
	~Screen();

	/**
	 * Register instrument
	 */
	template<class Instrument>
		void
		register_instrument (Registrant<Instrument>&, WorkPerformer&);

	/**
	 * Set position and size of an instrument.
	 * Values are factors, { 0, 0 } is top-left, { 1, 1 } is bottom-right.
	 */
	void
	set (BasicInstrument const&, QRectF requested_position, QPointF const anchor_position = { 0.0f, 0.0f });

	/**
	 * Set position and size of an instrument.
	 */
	void
	set_centered (BasicInstrument const&, QRectF requested_position);

	/**
	 * Set z-index for an instrument
	 */
	void
	set_z_index (BasicInstrument const&, int z_index);

	/**
	 * Enable/disable debug bounding boxes of instruments.
	 */
	void
	set_paint_bounding_boxes (bool enable);

	/**
	 * Wait for all asynchronous paintings to be finished.
	 * Call it before trying to destroy any registered instrument.
	 */
	void
	wait();

	/**
	 * Return the instrument tracker object.
	 */
	InstrumentTracker&
	instrument_tracker() noexcept;

	/**
	 * Return the instrument tracker object.
	 */
	InstrumentTracker const&
	instrument_tracker() const noexcept;

  protected:
	// QWidget API:
	void
	paintEvent (QPaintEvent*) override;

	// QWidget API:
	void
	resizeEvent (QResizeEvent*) override;

  private:
	/**
	 * Update screen canvas if parameters changed.
	 */
	void
	update_canvas (QSize);

	/**
	 * Request painting of all instruments on the canvas-buffer.
	 */
	void
	paint_instruments_to_buffer();

	/**
	 * Wait for async paint to be done in an active loop.
	 */
	void
	wait_for_async_paint (InstrumentTracker::Disclosure&);

	/**
	 * Prepare canvas for an instrument.
	 * Ensure it has requested size and set it to full alpha with color black.
	 */
	void
	prepare_canvas_for_instrument (std::unique_ptr<QImage>&, QSize);

	/**
	 * Create new image suitable for screen and instrument buffers.
	 */
	QImage
	allocate_image (QSize) const;

	void
	instrument_registered (InstrumentTracker::Disclosure&);

	void
	instrument_deregistered (InstrumentTracker::Disclosure&);

	void
	sort_by_z_index();

  private slots:
	/**
	 * Called when next frame should be painted.
	 */
	void
	refresh();

	void
	show_configurator();

  private:
	Machine&									_machine;
	Logger										_logger;
	InstrumentTracker							_instrument_tracker;
	QTimer*										_refresh_timer;
	QImage										_canvas;
	std::vector<InstrumentTracker::Disclosure*>	_z_index_sorted_disclosures;
	ScreenSpec									_screen_spec;
	si::Time const								_frame_time;
	bool										_paint_bounding_boxes	{ false };
};


template<class Instrument>
	inline void
	Screen::register_instrument (Registrant<Instrument>& instrument, WorkPerformer& work_performer)
	{
		_instrument_tracker.register_object (instrument, detail::InstrumentDetails (*instrument, work_performer));
	}


inline Screen::InstrumentTracker&
Screen::instrument_tracker() noexcept
{
	return _instrument_tracker;
}


inline Screen::InstrumentTracker const&
Screen::instrument_tracker() const noexcept
{
	return _instrument_tracker;
}

} // namespace xf

#endif

