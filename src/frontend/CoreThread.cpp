#include <wchar.h>
#include <stdlib.h>
#include "CoreThread.h"
#include "nsim_xim.h"

static CoreThread *core_worker = NULL;

CoreThread::CoreThread() {
	core_worker = this;
	qRegisterMetaType<StrPtr>( "StrPtr" );
}

void redraw_callback(char *str, size_t n) {
	StrPtr p((void*)str, n);
	emit core_worker->draw_base(p);
}

void redraw_img_callback(struct pngdata *dat, size_t n) {
	StrPtr p((void*)dat, n);
	emit core_worker->draw_img(p);
}

void disable_callback() {
	emit core_worker->disable_im();
}
void enable_callback() {
	emit core_worker->enable_im();
}

void CoreThread::thr_started() {
	/* Now make another XCB connection here for setting up the XIM service.
	   I want another connection because the XIM toolkit needs a window
	   and I don't know what Qt will do to my window. There are too much
	   magic in Qt itself.
	   Now pass the logic back into the C code.
	*/
	start_xim();
}

StrPtr::StrPtr() { p = NULL; np = 0; }
StrPtr::StrPtr(void *ptr, size_t len) { p = ptr; np = len; }
StrPtr::StrPtr(const StrPtr& s) { p = s.p; np = s.np; }

