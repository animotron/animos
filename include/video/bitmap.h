/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2011 Phantom OS team
 * Copyright (C) 2005-2011 Dmitry Zavalishin, dz@dz.ru
 *
 * Bitmaps.
 *
**/

#ifndef BITMAP_H
#define BITMAP_H

#include <video/color.h>

typedef struct drv_video_bitmap
{
    int             xsize;
    int             ysize;
    rgba_t          pixel[];
} drv_video_bitmap_t;

typedef struct drv_video_cursor
{
    int             hotx;
    int             hoty;

    drv_video_bitmap_t bitmap;
} drv_video_cursor_t;



extern drv_video_bitmap_t		close_bmp;
extern drv_video_bitmap_t		close_pressed_bmp;
extern drv_video_bitmap_t		pin_bmp;
extern drv_video_bitmap_t		rollup_bmp;
extern drv_video_bitmap_t		rollup_pressed_bmp;
extern drv_video_bitmap_t		title_brown_bmp;
extern drv_video_bitmap_t		title_green_bmp;
extern drv_video_bitmap_t		title_violet_bmp;

extern drv_video_bitmap_t		power_button_pressed_sm_bmp;
extern drv_video_bitmap_t		power_button_sm_bmp;

extern drv_video_bitmap_t		task_button_bmp;


drv_video_bitmap_t *      drv_video_get_default_mouse_bmp(void);


#endif //BITMAP_H
