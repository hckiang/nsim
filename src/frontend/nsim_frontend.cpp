#include <QApplication>
#include <QX11Info>
#include <QThread>
#include <QMainWindow>

#include "NSGUIWindow.h"
#include "CoreThread.h"
#include "xcbtools.h"
#include <stdlib.h>

#define WIN_W 160
#define WIN_H 160

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QMainWindow rootWindow;
	NSGUIWindow window(&rootWindow);
	
	xcb_connection_t *c = QX11Info::connection();
	xcb_window_t w = (xcb_window_t) window.winId();
	window.setFixedSize(WIN_W, WIN_H);
	window.setWindowFlags((Qt::ToolTip            // | Qt::WindowTransparentForInput
			       | Qt::WindowDoesNotAcceptFocus 
			       | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
			      & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint
			      & ~Qt::WindowCloseButtonHint);
	window.hide();
	return app.exec();
}
