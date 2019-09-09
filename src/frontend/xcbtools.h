#pragma once

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>

#ifdef __cplusplus
extern "C" {
#endif

	xcb_atom_t request_atom(xcb_connection_t *c, const char *str);
	void no_titlebar(xcb_connection_t *c, xcb_window_t w);
	void lock_winsize( xcb_connection_t *c, xcb_window_t w, int width, int height );
	void top_layer(xcb_connection_t *c, xcb_window_t w);
	void no_focus(xcb_connection_t *c, xcb_window_t w);
	
#ifdef __cplusplus
}
#endif
