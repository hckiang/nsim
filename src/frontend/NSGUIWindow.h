#pragma once

#include <QWidget>
#include <QThread>
#include "CoreThread.h"

enum draw_type_t {
	NSIM_DRAW_TYPE_BASE = 0,
	NSIM_DRAW_TYPE_IMG = 1,
};

class NSGUIWindow : public QWidget
{
	Q_OBJECT
	QThread core_thr;
	enum draw_type_t draw_type;
	void *grid_str;
	size_t grid_strlen;
	int last_screen;
	QPoint cur_pos;
public:
	NSGUIWindow(QWidget *parent = 0);

protected:
	void paintEvent(QPaintEvent *e);
	void showEvent(QShowEvent *e);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
private:
	void do_painting();
	int tcelldraw(char *cnch, int grid);
	int tcelldraw_img(struct pngdata *img, int grid);
public slots:
	void draw_base(StrPtr p);
	void draw_img(StrPtr p);
	void enable_im();
	void disable_im();
};

