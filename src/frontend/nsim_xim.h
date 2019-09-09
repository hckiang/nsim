#pragma once

#ifdef __cplusplus
extern "C" {
#endif
	struct pngdata {
		uint8_t *bytes;
		size_t  n;
	};

	int start_xim();
	int xknum_to_idx(int xknum);
	int idx_to_xknum(int idx);
	int is_nullchar_placeholder (char *str, size_t n);

	extern void redraw_callback(char *, size_t);
	extern void redraw_img_callback(struct pngdata *, size_t);
	extern void disable_callback();
	extern void enable_callback();
#ifdef __cplusplus
}
#endif

