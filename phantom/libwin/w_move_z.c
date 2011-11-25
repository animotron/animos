/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2009 Dmitry Zavalishin, dz@dz.ru
 *
 * Windowing system - top/bottom.
 *
**/

#define DEBUG_MSG_PREFIX "win"
#include <debug_ext.h>
#define debug_level_flow 0
#define debug_level_error 10
#define debug_level_info 10

#include <phantom_libc.h>

#include <video/window.h>
#include <video/internal.h>






void w_to_bottom(drv_video_window_t *w)
{
    w_lock();

    w->state &= ~WSTATE_WIN_UNCOVERED; // mark as possibly covered

    queue_remove(&allwindows, w, drv_video_window_t *, chain);
    queue_enter_first(&allwindows, w, drv_video_window_t *, chain);
    drv_video_window_rezorder_all();

    w_unlock();

    scr_repaint_all();
}

void w_to_top(drv_video_window_t *w)
{
    w_lock();

    w->state &= ~WSTATE_WIN_UNCOVERED; // mark as possibly covered

    queue_remove(&allwindows, w, drv_video_window_t *, chain);
#if 0
    queue_enter(&allwindows, w, drv_video_window_t *, chain);
#else
    if( (w->flags & WFLAG_WIN_ONTOP) || (queue_empty(&allwindows)) )
    {
        // Trivial case - our window has 'ontop' flag too and can be topmost, or queue is empty
        queue_enter(&allwindows, w, drv_video_window_t *, chain);
    }
    else
    {
        // Our window has no 'ontop' flag and must go under topmost ones and queue is not empty
        drv_video_window_t *iw;
        queue_iterate_back(&allwindows, iw, drv_video_window_t *, chain)
        {
            if( ! (iw->flags & WFLAG_WIN_ONTOP) )
            {
                // Found window with no WFLAG_WIN_ONTOP flag - come on top of it
                queue_enter_after(&allwindows, iw, w, drv_video_window_t *, chain);
                goto inserted;
            }
        }

        // must go to most bottom pos?? near to unreal...
        SHOW_ERROR0( 0, "insert at bottom");
        queue_enter_first(&allwindows, w, drv_video_window_t *, chain);
    }
inserted:
#endif
    drv_video_window_rezorder_all();

    w_unlock();

    scr_repaint_all();
}


