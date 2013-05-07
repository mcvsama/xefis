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

#ifndef XEFIS__CORE__INSTRUMENT_WIDGET_H__INCLUDED
#define XEFIS__CORE__INSTRUMENT_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/work_performer.h>
#include <xefis/utility/mutex.h>
#include <xefis/utility/semaphore.h>


namespace Xefis {

class InstrumentWidget: public QWidget
{
  public:
	class PaintWorkUnit: public WorkPerformer::Unit
	{
	  public:
		// Ctor
		PaintWorkUnit (InstrumentWidget*);

		// Dtor
		~PaintWorkUnit() noexcept { }

		/**
		 * Prepare params from the queue to be processed.
		 * Default implementation does nothing.
		 * \threadsafe
		 */
		virtual void
		pop_params();

		/**
		 * Called after widget is resized.
		 * Default implementation does nothing.
		 * \threadsafe
		 */
		virtual void
		resized();

		/**
		 * Paints the widget on the canvas.
		 */
		virtual void
		paint (QImage& canvas) = 0;

	  protected:
		// WorkPerformer::Unit API
		void
		execute() override;

		/**
		 * Return size of the widget.
		 */
		QSize const&
		size() const;

		/**
		 * Return window size.
		 */
		QSize const&
		window_size() const;

	  private:
		InstrumentWidget*	_widget;
		QSize				_size;
		QSize				_window_size;
		QImage				_image;
	};

  private:
	constexpr static int UpdateEvent			= QEvent::MaxUser - 1;
	constexpr static int RequestRepaintEvent	= QEvent::MaxUser - 2;

  public:
	// Ctor
	InstrumentWidget (QWidget* parent, WorkPerformer* = nullptr);

	/**
	 * Enable threaded painter.
	 * Must have valid WorkPerformer passed in constructor for this.
	 */
	void
	set_painter (PaintWorkUnit* painter);

	/**
	 * Safely wait for painting thread to finish.
	 * Call this method at the beginning of derived class destructor.
	 */
	void
	wait_for_painter();

	/**
	 * Return pair of widget and window size in a threadsafe way.
	 * Can be used from painting thread.
	 */
	std::pair<QSize, QSize>
	threadsafe_sizes() const;

	/**
	 * Request update in a threadsafe way.
	 * May be called from a different thread.
	 */
	void
	threadsafe_update();

	/**
	 * Request repaint when parameter value changes.
	 */
	virtual void
	request_repaint();

	/**
	 * Pass params to painter object queue.
	 * Default implementation does nothing.
	 * \threadsafe
	 */
	virtual void
	push_params();

  protected:
	// QWidget API
	void
	resizeEvent (QResizeEvent*) override;

	// QWidget API
	void
	paintEvent (QPaintEvent*) override;

	// QWidget API
	void
	customEvent (QEvent*) override;

  private:
	WorkPerformer*			_work_performer				= nullptr;
	PaintWorkUnit*			_paint_work_unit			= nullptr;
	mutable RecursiveMutex	_paint_mutex;
	Semaphore				_paint_sem;
	QImage					_paint_buffer;
	QSize					_threadsafe_size;
	QSize					_threadsafe_window_size;
	bool					_paint_again				= false;
	bool					_paint_in_progress			= false;
	bool					_paint_requested			= false;
};


inline QSize const&
InstrumentWidget::PaintWorkUnit::size() const
{
	return _size;
}


inline QSize const&
InstrumentWidget::PaintWorkUnit::window_size() const
{
	return _window_size;
}


inline void
InstrumentWidget::set_painter (PaintWorkUnit* painter)
{
	_paint_work_unit = painter;
}

} // namespace Xefis

#endif

