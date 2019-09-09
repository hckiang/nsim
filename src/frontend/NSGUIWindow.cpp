#include <QApplication>
#include <QPainter>
#include <QThread>
#include <QFont>
#include <QRectF>
#include <QDesktopWidget>
#include <QMouseEvent>
#include "nsim_xim.h"
#include "CoreThread.h"
#include "NSGUIWindow.h"

NSGUIWindow::NSGUIWindow (QWidget *parent) : QWidget(parent) {
	CoreThread *worker = new CoreThread;
	worker->moveToThread(&core_thr);
	connect(worker, &CoreThread::draw_base, this, &NSGUIWindow::draw_base);
	connect(worker, &CoreThread::draw_img, this, &NSGUIWindow::draw_img);
	connect(worker, &CoreThread::disable_im, this, &NSGUIWindow::disable_im);
	connect(worker, &CoreThread::enable_im, this, &NSGUIWindow::enable_im);
	connect(&core_thr, &QThread::started,   worker, &CoreThread::thr_started);
	
	core_thr.start();
	draw_type = NSIM_DRAW_TYPE_BASE;
	grid_str = NULL;
	grid_strlen = 0;
	last_screen = -10000;
}

void NSGUIWindow::paintEvent(QPaintEvent *e) {
	Q_UNUSED(e);
	do_painting();
}

void NSGUIWindow::showEvent( QShowEvent* e ) {
	Q_UNUSED(e);
	QDesktopWidget* m = QApplication::desktop();
	int cur_screen = m->screenNumber(QCursor::pos());
	if (last_screen != cur_screen) {
		last_screen = cur_screen;
		QRect desk_rect = m->screenGeometry(cur_screen);
		int desk_w = desk_rect.width();
		int desk_h = desk_rect.height();
		int x = -width() + desk_rect.width() + desk_rect.x() - 70;
		int y = -height() + desk_rect.height() + desk_rect.y() - 70;
		move(x, y);
	}
}

int NSGUIWindow::tcelldraw(char *cnch, int grid) {
	int x, y, consumed;
	float gw, gh, gx, gy;
	char cnst[6] = {0};
	if (is_nullchar_placeholder(cnch, 2))
		return 2;
	if (grid >= 7)      {   x = grid-7;   y = 0;    }
	else if (grid >= 4) {   x = grid-4;   y = 1;    }
	else                {   x = grid-1;   y = 2;    }
	gw = width()/3.f;          gh = height()/3.f;
	gx = gw*x;                 gy = (gh*y);
	memcpy(cnst, cnch, (consumed = mbrlen(cnch, 6, NULL)));
	QPainter painter(this);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);
	painter.setFont(QFont("TW-Kai", 23, -1, false));
	painter.drawText(QRectF(gx,gy,gw,gh), Qt::AlignCenter, cnst, NULL);
	return consumed;
}
int NSGUIWindow::tcelldraw_img(struct pngdata *img, int grid) {
	int x, y;
	float gw, gh, gx, gy;
	float pw, ph, px, py;
	if (grid >= 7)      {   x = grid-7;   y = 0;    }
	else if (grid >= 4) {   x = grid-4;   y = 1;    }
	else                {   x = grid-1;   y = 2;    }
	gw = width()/3.f;          gh = height()/3.f;
	gx = gw*x;                 gy = (gh*y);
	pw = 46;                   ph = 46;
	px = gx + (gw-pw)/2.;      py = gy + (gh-ph)/2.;

	QImage qimg = QImage::fromData(img->bytes, img->n, "PNG");
	QPainter painter(this);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);
	painter.drawImage(QRectF(px,py,pw,ph), qimg, QRectF(0,0,46,46));
	return sizeof(struct pngdata);
}
void NSGUIWindow::mousePressEvent (QMouseEvent *e) {
	cur_pos = e->globalPos();
	setCursor(Qt::ClosedHandCursor);
}
void NSGUIWindow::mouseMoveEvent (QMouseEvent *e) {
	QPoint delta = e->globalPos() - cur_pos;
	move(x()+delta.x(), y()+delta.y());
	cur_pos = e->globalPos();
}
void NSGUIWindow::mouseReleaseEvent (QMouseEvent *e) {
	(void) e;
	setCursor(Qt::ArrowCursor);
}
void NSGUIWindow::do_painting() {
	QPainter painter(this);
	
	int ww = width();
	int wh = height();
	
	painter.fillRect(0, 0, ww, wh, Qt::white);
	QPen pen(Qt::black, Qt::SolidLine);
	pen.setWidth(2);
	painter.setPen(pen);
	painter.drawLine(1,1,1,wh-1);
	painter.drawLine(1,wh-1,ww-1,wh-1);
	painter.drawLine(1,1,ww-1,1);
	painter.drawLine(ww-1,0,ww-1,wh-1);
	for (int y = 0; y < 2; ++y) {
		int yc = wh*(y+1)/3;
		painter.drawLine(0, yc , ww, yc);
	}
	for (int x = 0; x < 2; ++x) {
		int xc = ww*(x+1)/3;
		painter.drawLine(xc, 0, xc, wh);
	}
	switch (draw_type) {
	case NSIM_DRAW_TYPE_BASE:
		if (!grid_str) {
			QPainter painter2(this);
			painter2.setFont(QFont("serif", 16, -1, false));
			const QRect rect = QRect(0, 0, 100, 50);
			painter2.drawText(rect, Qt::AlignCenter, tr("錯"), NULL);
		} else {
			char *grid_str_ptr = (char*)grid_str;
			for (int i = 0; i < 9;
			     grid_str_ptr += tcelldraw(grid_str_ptr, idx_to_xknum(i)), ++i);
		}
		break;
	case NSIM_DRAW_TYPE_IMG:
		if (!grid_str) {
			QPainter painter2(this);
			painter2.setFont(QFont("serif", 16, -1, false));
			const QRect rect = QRect(0, 0, 100, 50);
			painter2.drawText(rect, Qt::AlignCenter, tr("錯"), NULL);
		} else {
			struct pngdata *grid_str_ptr = (struct pngdata*)grid_str;
			for (int i = 0; i < 9; ++grid_str_ptr, ++i)
				tcelldraw_img(grid_str_ptr, idx_to_xknum(i));
					      
		}

		break;
	}
}

void NSGUIWindow::draw_base(StrPtr p) {
	draw_type = NSIM_DRAW_TYPE_BASE;
	grid_str = p.p;
	grid_strlen = p.np;
	this->repaint();                     /* necessary */
}

void NSGUIWindow::draw_img(StrPtr p) {
	draw_type = NSIM_DRAW_TYPE_IMG;
	grid_str = p.p;
	grid_strlen = p.np;
	this->repaint();                     /* necessary */
}

void NSGUIWindow::disable_im() {
	hide();
}
void NSGUIWindow::enable_im() {
	show();
}
