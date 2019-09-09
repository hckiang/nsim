// #define DEBUG

#include <stdlib.h>
#include <wchar.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <locale.h>
#include <assert.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_icccm.h>     /* window size hint */
#include <xcb/xcb_atom.h>
#include <xcb-imdkit/imdkit.h>
#include <xcb-imdkit/encoding.h>
#include <xcb-imdkit/ximproto.h>
#include "CoreThread.h"
#include "pics.h"

#ifdef DEBUG
 #define D 
#else
 #define D for(;0;)
#endif


#define TREE_PART_SIZE 4000
extern uint8_t _binary_______data_chars_db_tab_start;
extern uint8_t _binary_______data_chars_db_tab_end;
static uint8_t *nsimbin;

static struct pngdata picstructs[10][9];

int to_nsimbin (uint8_t **dst, uint8_t *tab, size_t tablen) {
	uint8_t *tabq;    /* tab position */
	uint32_t *trep;   /* start of the tree part */
	uint8_t *chrq;    /* current position to the character part */
	uint32_t l, n;
	
	*dst = (uint8_t *) calloc(103000, 1);  /* Probably too much. calculate later. */
	trep = (uint32_t *) (*dst);
	chrq = (void*) trep + TREE_PART_SIZE;
	
	tabq = tab;
	assert(tabq[tablen-1] == '\n');
	for(; (unsigned long int)(tabq-tab) != tablen;) {
		for (l = 0; tabq[l] != ':'; ++l) ;
		for (n = l; tabq[n] != '\n'; ++n) ;
		++l;
		*((uint32_t*)chrq) = n-l;
		memcpy(chrq+4, tabq+l, n-l);
		if (tabq[0]=='0' && tabq[1]==':')
			trep[0] = (uint32_t) ((void*)chrq - (void*)trep);
		else if (tabq[0]=='0' && tabq[1]=='9' && tabq[2]==':')
			trep[90] = (uint32_t) ((void*)chrq - (void*)trep);
		else
			trep[(tabq[0]-'0')*100 + (tabq[1]-'0')*10 + (tabq[2]-'0')] =
				(uint32_t) ((void*)chrq - (void*)trep);
		chrq += n-l+4;
		tabq += n+1;
	}
	return (void*)chrq - (void*)trep;
}

void nschlst(uint8_t *bin, int c1, int c2, int c3, char **chlst, size_t *n) {
	uint32_t size;
	uint8_t *binq;
	D printf("actual size: %lx\n", (size_t)PICSIZ(2,3));
	D printf("ld size: %x\n", (int)&_binary_______data_pics_f9_3_04_png_size);
	if (c1 == 0 && c2 < 0 && c3 < 0) {
		binq = bin + *((uint32_t*)bin);
		size = *((uint32_t*)binq);
		D printf("address (zero branch): %lu\n", (binq - bin));
		D printf("bin block size = %u\n", size);
		binq += 4;
	} else {
		D printf("nschlst: c1: %d, c2: %d, c3: %d\n", c1, c2, c3);
		uint32_t offset = *((uint32_t*)bin + c1*100 +c2*10 +c3);
		binq = bin + offset;
		D printf("address (non-zero branch): %lu   offset %u\n", (binq - bin), offset);
		if (!offset) {
			*n = 0;
			return;
		}
		size = *((uint32_t*)binq);
		D printf("bin block size = %u\n", size);
		binq += 4;
	}
	*chlst = (char *)binq;
	*n = size;
}


static xcb_im_t *im;
static xcb_connection_t *c;
static xcb_window_t w;

struct imst {
	uint8_t heldkeys[3];
	int heldkeys_len;
	int cmtready;
	/* belows are only set iff cmtready is true */
	char *rstr;
	size_t nrstr;
	int curpage;
	int nxtpage;
	int notfound;
};
static struct imst curimst = {0};

int cut_for_redraw(char *str, size_t n, int page, char **start, size_t *nout, size_t *nwcout) {
	char *str_p;
	int i, j;
	for (i=0, str_p=str;  i<page && (str_p-str < n-9); ++i) {
		for (j=0; j<9; ++j) {
			str_p += mbrlen(str_p, n-(str_p-str), NULL);
		}
	}
	*start = str_p;
	for (i=0;             i<9 && str_p-str<n;      ++i, str_p+= mbrlen(str_p, n-(str_p-str), NULL));
	*nwcout = i;
	*nout = str_p - *start;
	D printf("*start - str: %lu\n", *start - str);
	D printf("n - *sstart - str: %lu\n", *start - str);
	return (*start-str) + *nout >= n;
}


void do_nextrspage(struct imst *st) {
	char *start;
	size_t len, nwcout;
	int s;
	
	s = cut_for_redraw(st->rstr, st->nrstr, st->nxtpage, &start, &len, &nwcout);
	st->curpage = st->nxtpage;
	st->nxtpage = s ? 0 : st->nxtpage + 1;
	if (s) D printf("no next page anymore. next space set to 0\n");
	if (nwcout == 9) redraw_callback(start, len);
	else if (nwcout == 0) ;
	else D printf("WARNING!!\nWARNING!!\nWARNING!!\n nwcout = %lu!!\n", nwcout);
	
}

int xknum_to_idx(int xknum) {
	static int t[9] = { 6,7,8,3,4,5,0,1,2 };
	return t[xknum-1];
}

int idx_to_xknum(int idx) {
	static int t2[9] = { 7,8,9,4,5,6,1,2,3 };
	return t2[idx];
}


int is_nullchar_placeholder (char *str, size_t n) {
	return (n == 2) && ((uint8_t)str[0] == 0xc3) && ((uint8_t)str[1] == 0x98);
}

void do_commit(xcb_im_t* im, xcb_im_input_context_t* ic, struct imst *st, int xknum) {
	int idx, l, i;
	char *start_p;
	size_t compound_l;
	char* compound_s;
	idx = xknum_to_idx(xknum);
	/* See wengxt/xcb-imdkit commit:4a04ba78c51fba58594e22997bff252590083597 */
	for (i = 0, start_p = st->rstr; i < idx + st->curpage*9; ++i) {
		start_p += l = mbrlen(start_p, st->nrstr - (start_p - st->rstr), NULL);
	}
	l = mbrlen(start_p, st->nrstr - (start_p - st->rstr), NULL);
	D printf("i:%d, l=%d, idx=%d, curpage=%d\n", i, l, idx, st->curpage);
	D printf("start_p-st->rstr = %lu\n", start_p-st->rstr);
	D printf("start_p text:");
	for (i = 0; i < l; ++i) {
		D printf("%02hhX ", start_p[i]);
	}
	D printf("\n");
	if (! is_nullchar_placeholder(start_p, l)) {
		compound_s = xcb_utf8_to_compound_text(start_p, l, &compound_l);
		if (!compound_s) printf("[ERROR] xcb_utf8_to_compound_text returns NULL!\n");
		xcb_im_commit_string(im, ic, XCB_XIM_LOOKUP_CHARS, compound_s, compound_l, 0);
		free(compound_s);
	}
	st->cmtready = 0;
	st->heldkeys_len = 0;
	st->notfound = 0;

	/* Now draw */
#define ST_STR00 "〇〇〇〇〇〇〇〇〇〇"

	// redraw_callback(ST_STR00, strlen(ST_STR00));
	// Should pass an struct pngdata[9] and the size should be 9 * sizeof(struct pngdata).
	redraw_img_callback(&(picstructs[9][0]), 9*sizeof(struct pngdata));
}



void do_progress (struct imst *st, int xknum) {
	(void) xknum;
	#define ST_STR0 "〇〇〇〇〇〇〇〇〇〇"
	#define ST_STR1 "一一一一一一一一一"
	#define ST_STR2 "二二二二二二二二二"
	D printf("[DO_PROG] st->heldkeys_len: %d\n", st->heldkeys_len);
	assert(st->heldkeys_len != 3);
	switch (st->heldkeys_len) {
	case 0:
		redraw_img_callback(&(picstructs[9][0]), 9*sizeof(struct pngdata));
		break;
	case 1:
		redraw_img_callback(&(picstructs[xknum_to_idx(xknum)][0]), 9*sizeof(struct pngdata));
		break;
	case 2:
		redraw_img_callback(&(picstructs[9][0]), 9*sizeof(struct pngdata));
		break;
	default: printf("BUG!!!!\n");
	}
}


int dispatch_keyact (xcb_im_t* im, xcb_im_input_context_t* ic,
		     struct imst *st, xcb_keysym_t sym) {
	int nsnum;
	D printf("Processing keysym: %x\n", sym);
	if (sym == 0xffae) { // the '.' on keypad
		st->cmtready = 0;
		st->heldkeys_len = 0;
		st->notfound = 0;
		redraw_img_callback(&(picstructs[9][0]), 9*sizeof(struct pngdata));
		return 1;
	} else if (! (sym >= 0xffb0 && sym <= 0xffb9) ) {
		return 0;
	}
	nsnum = sym & 0x000f;
	D printf("nsnum: %d\n", nsnum);
	if (st->cmtready == 0) {
		D printf("nsnum: %d\n", nsnum);
		st->heldkeys[st->heldkeys_len++] = nsnum;
		if (st->heldkeys_len == 3) {
			nschlst(nsimbin,
				st->heldkeys[0], st->heldkeys[1], st->heldkeys[2],
				&(st->rstr), &(st->nrstr));
			if (st->nrstr != 0) {
				st->cmtready = 1;
				D printf("cmtready set to one.");
				st->nxtpage = 0;
				do_nextrspage(st);
			} else {
				st->notfound = 1;
				st->heldkeys_len = 0;
				redraw_img_callback(&(picstructs[9][0]), 9*sizeof(struct pngdata));
			}
			return 1;
		} else if (st->heldkeys_len == 1 && st->heldkeys[0] == 0) {
			nschlst(nsimbin,
				st->heldkeys[0], 0, 0,
				&(st->rstr), &(st->nrstr));
			st->cmtready = 1;
			D printf("cmtready set to one.");
			st->nxtpage = 0;
			do_nextrspage(st);
			return 1;
		}
	}
	D printf("%d %d %d %d\n", st->cmtready, st->heldkeys_len, st->heldkeys[0], nsnum);
	if (st->cmtready == 1 && st->heldkeys_len == 1 && st->heldkeys[0] == 0 && nsnum == 9) {
		st->heldkeys[st->heldkeys_len++] = nsnum;
		nschlst(nsimbin, st->heldkeys[0], st->heldkeys[1], 0, &(st->rstr), &(st->nrstr));
		st->nxtpage = 0;
		do_nextrspage(st);
		return 1;
	}

	if (st->cmtready == 1) {
		if (nsnum == 0)   do_nextrspage(st);
		else              do_commit(im, ic, st, nsnum);
	} else {
		do_progress(st, nsnum);
	}
	st->notfound = 0;
	return 1;
}

void callback(xcb_im_t* im, xcb_im_client_t* client, xcb_im_input_context_t* ic,
              const xcb_im_packet_header_fr_t* hdr,
              void* frame, void* arg, void* user_data) {
	(void) frame; (void) client;
	D printf("Received major op code: %d\n", hdr->major_opcode);
	if (hdr->major_opcode == XCB_XIM_DISCONNECT) {
		D printf("DISCONNECTED. I wanna reset the IM state now... \n");
	} else if (hdr->major_opcode == XCB_XIM_TRIGGER_NOTIFY) {
		curimst.cmtready = 0;
		curimst.heldkeys_len = 0;
		curimst.notfound = 0;
		if (((xcb_im_trigger_notify_fr_t*)frame)->flag)  disable_callback();
		else {
			enable_callback();
			redraw_img_callback(&(picstructs[9][0]), 9*sizeof(struct pngdata));
		}
	} else if (hdr->major_opcode == XCB_XIM_FORWARD_EVENT) {
		xcb_key_press_event_t* event = arg;
		xcb_key_symbols_t* key_symbols = user_data;
		xcb_keysym_t sym = xcb_key_symbols_get_keysym(key_symbols, event->detail, 3);
		//D printf("sym: %04x\n", sym);
		fflush(stdout);
		/* When focus into gedit, it sends 56,58,54 and when focusing out it sends 56,59,54

		   56: XCB_INPUT_XI_LIST_PROPERTIES
		   58: XCB_INPUT_XI_DELETE_PROPERTY
		   59: XCB_INPUT_XI_GET_PROPERTY
		   54: XCB_INPUT_XI_PASSIVE_GRAB_DEVICE

		   See /usr/include/xcb/xinput.h
		   But checking them alone won't let me know if it is really an unfocus/focus event.
		   I guess I need to check more other arguments?
		   
		   Or I guess I can simply preserve state across all clients like what ibus does
		   by default.
		*/
		if (! dispatch_keyact(im, ic, &curimst, sym)) {
			// D printf("Not keypad. Not capturing %xd\n", sym);
			xcb_im_forward_event(im, ic, event);
		}
	}
}

static uint32_t style_array[] = {
	XCB_IM_PreeditPosition | XCB_IM_StatusArea,         //OverTheSpot
	XCB_IM_PreeditPosition | XCB_IM_StatusNothing,      //OverTheSpot
	XCB_IM_PreeditPosition | XCB_IM_StatusNone,         //OverTheSpot
	XCB_IM_PreeditNothing | XCB_IM_StatusNothing,       //Root
	XCB_IM_PreeditNothing | XCB_IM_StatusNone,          //Root
};
static char* encoding_array[] = {
	"COMPOUND_TEXT",
};

static xcb_im_encodings_t encodings = {
	1, encoding_array
};

static xcb_im_styles_t styles = {
	5, style_array
};

int start_xim()
{
	xcb_generic_event_t *e;
	int screen_default_nbr;

	setlocale(LC_CTYPE, "en_US.UTF-8");
	to_nsimbin(&nsimbin,
		   &_binary_______data_chars_db_tab_start,
		   &_binary_______data_chars_db_tab_end - &_binary_______data_chars_db_tab_start);
	
	/* After the final linking, the original offsets in the binary blob .o becomes wrong
	   for some unknown reason. So the original *_size variables set by ld are all wrong. */
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 9; ++j) {
			picstructs[i][j].bytes = picstarts[i][j];
			picstructs[i][j].n = PICSIZ(i,j);
		}
	}
		


	xcb_compound_text_init();
	
	/* Open the connection to the X server */
	c = xcb_connect (NULL, &screen_default_nbr);

	/* Check if screen is available. Fail if not.  */
	xcb_screen_t* screen = xcb_aux_get_screen(c, screen_default_nbr);
	xcb_key_symbols_t* key_symbols = xcb_key_symbols_alloc(c);
	if (!screen) return 0;
	
	w = xcb_generate_id(c);
	xcb_create_window (c,
			   XCB_COPY_FROM_PARENT,                 /* depth (same as root) */
			   w,                                    /* window Id */
			   screen->root,                         /* parent window */
			   0, 0,                                 /* x, y */
			   1, 1,                                 /* w, h */
			   1,                                    /* border_width */
			   XCB_WINDOW_CLASS_INPUT_OUTPUT,        /* class */
			   screen->root_visual,                  /* visual */
			   0,
			   NULL);
	
	xcb_im_trigger_keys_t keys;
	xcb_im_ximtriggerkey_fr_t key;
	key.keysym = ' ';
	key.modifier = 1 << 2;
	key.modifier_mask = 1 << 2;
	keys.nKeys = 1;
	keys.keys = &key;
	im = xcb_im_create(c,
			   screen_default_nbr,
			   w,
			   "nsim",
			   XCB_IM_ALL_LOCALES,
			   &styles,
			   &keys,
			   &keys,
			   &encodings,
			   0,
			   callback,
			   key_symbols);
	
	if (! xcb_im_open_im(im)) {
		printf("PANIC: some other IM servers running?\n"); abort();
	}
	D printf("winid: %u\n", w);
	
	while ( (e = xcb_wait_for_event (c)) ) {
		xcb_im_filter_event(im, e);
		free(e);
	}
	xcb_im_close_im(im);
	xcb_im_destroy(im);
	xcb_key_symbols_free(key_symbols);
	xcb_disconnect(c);
	
	return 1;
}
