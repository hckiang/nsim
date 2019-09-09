#include "xcbtools.h"
#include "stdlib.h"
#include "string.h"

xcb_atom_t request_atom(xcb_connection_t *c, const char *str) {
	xcb_intern_atom_cookie_t cke;
	xcb_intern_atom_reply_t *rpy;
	xcb_atom_t atom;

	cke = xcb_intern_atom(c, 0, strlen(str), str);
	rpy = xcb_intern_atom_reply(c, cke, NULL);
	atom = rpy->atom;
	free(rpy);
	return atom;
}

void no_titlebar(xcb_connection_t *c, xcb_window_t w) {
	struct MotifHints
	{
		uint32_t   flags;
		uint32_t   functions;
		uint32_t   decorations;
		int32_t    input_mode;
		uint32_t   status;
	} mhints;

	mhints.flags = 2;
	mhints.functions = 0;
	mhints.decorations = 0;
	mhints.input_mode = 0;
	mhints.status = 0;

	xcb_atom_t motif_wh = request_atom(c, "_MOTIF_WM_HINTS");
	xcb_change_property ( c,
			      XCB_PROP_MODE_REPLACE,
			      w,
			      motif_wh,
			      motif_wh,
			      32,
			      5,
			      &mhints );
}
void lock_winsize(xcb_connection_t *c, xcb_window_t w, int width, int height) {
	xcb_size_hints_t hints;
	xcb_icccm_size_hints_set_min_size(&hints, width, height);
	xcb_icccm_size_hints_set_max_size(&hints, width, height);
	xcb_icccm_set_wm_size_hints(c, w, XCB_ATOM_WM_NORMAL_HINTS, &hints);
}

void top_layer(xcb_connection_t *c, xcb_window_t w) {
	xcb_atom_t wmstate = request_atom(c, "_NET_WM_STATE");
	xcb_atom_t wmstate_above = request_atom(c, "_NET_WM_STATE_ABOVE");
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, w,
			    wmstate, XCB_ATOM_ATOM, 32, 1, &wmstate_above);
}
void no_focus(xcb_connection_t *c, xcb_window_t w) { return; }
/*
void no_focus(xcb_connection_t *c, xcb_window_t w) {
	xcb_get_property_cookie_t cook, cook2;
	xcb_icccm_wm_hints_t wmhint;
	xcb_generic_error_t *e;

	cook = xcb_icccm_get_wm_hints(c, w);
	if (! xcb_icccm_get_wm_hints_reply(c, cook, &wmhint, &e)) {
		printf("[ERROR] Failed to get ICCCM properties through XCB.\n");
	}
	xcb_icccm_wm_hints_set_input(&wnhint, 0);
	cook2 = 
*/
