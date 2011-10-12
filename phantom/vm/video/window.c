/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2010 Dmitry Zavalishin, dz@dz.ru
 *
 * Windowing system internals and housekeeping.
 *
 *
**/
#define DEBUG_MSG_PREFIX "win"
#include <debug_ext.h>
#define debug_level_flow 0
#define debug_level_error 10
#define debug_level_info 10

#include <assert.h>
#include <phantom_libc.h>
#include <event.h>
//#include <spinlock.h>

#include <hal.h>
#include <kernel/init.h>
#include "win_local.h"
#include <video/window.h>
#include <video/button.h>
#include <video/internal.h>

//static void defaultEventProcessor();


//static
queue_head_t     	allwindows = { &allwindows, &allwindows };

//static
drv_video_window_t *	focused_window = 0;

//char wild_ptr_catch[2048] = {};

#define ALLW_MUTEX 1

#if ALLW_MUTEX
static hal_mutex_t      allw_mutex;
#else
//static
hal_spinlock_t  	allw_lock = {};
#endif

//char wild_ptr_catch[2048] = {};


#if !ALLW_MUTEX
static int wie;
#endif

// use with care!
void w_lock(void)
{
#if ALLW_MUTEX
    hal_mutex_lock(&allw_mutex);
#else
    wie = hal_save_cli();
    hal_spin_lock( &allw_lock );
#endif
}

void w_unlock(void)
{
#if ALLW_MUTEX
    hal_mutex_unlock(&allw_mutex);
#else
    hal_spin_unlock( &allw_lock );
    if(wie) hal_sti();
#endif
}

void w_assert_lock(void)
{
#if ALLW_MUTEX
    ASSERT_LOCKED_MUTEX( &allw_mutex );
#else
    assert(hal_spin_locked( &allw_lock ));
#endif
}






void drv_video_init_windows(void)
{
#if ALLW_MUTEX
    hal_mutex_init( &allw_mutex, "allw" );
#else
    hal_spin_init( &allw_lock ));
#endif
}


void drv_video_window_free(drv_video_window_t *w)
{
    drv_video_window_destroy(w);
    free(w);
}



static void
common_window_init( drv_video_window_t *w,
                        int xsize, int ysize )
{
    w->state |= WSTATE_WIN_VISIBLE; // default state is visible

    w->xsize = xsize;
    w->ysize = ysize;

    w->li = w->ti = w->ri = w->bi = 0;

    //w->generation = 0;

    w->x = 0;
    w->y = 0;
    w->z = 0xFE; // quite atop

    w->bg = COLOR_BLACK;

    queue_init(&(w->events));
    w->events_count = 0;
    w->stall = 0;

    w->inKernelEventProcess = defaultWindowEventProcessor;
    w->owner = 0;

#if !VIDEO_T_IN_D
    w->w_title = 0;
#endif
    w->w_decor = 0;
    w->w_owner = 0;
}



drv_video_window_t *private_drv_video_window_create(int xsize, int ysize)
{
    drv_video_window_t *w = calloc(1,drv_video_window_bytes(xsize,ysize));
    if(w == 0)
        return 0;

    common_window_init( w, xsize, ysize );
    return w;
}



drv_video_window_t *
drv_video_window_create(
                        int xsize, int ysize,
                        int x, int y,
                        rgba_t bg, const char *title )
{
    drv_video_window_t *w = private_drv_video_window_create(xsize, ysize);
    drv_video_window_init( w, xsize, ysize, x, y, bg );
    w->title = title;
    // Repaint title
    event_q_put_win( 0, 0, UI_EVENT_WIN_REDECORATE, w );
    return w;
}



// for statically allocated ones
void
drv_video_window_init( drv_video_window_t *w,
                        int xsize, int ysize,
                        int x, int y,
                        rgba_t bg )
{
    common_window_init( w, xsize, ysize );

    w->flags = WFLAG_WIN_DECORATED;

    w->x = x;
    w->y = y;
    //w->z = 0xFE; // quite atop
    w->bg = bg;

    w->title = "?";

    w_lock();
    //drv_video_winblt_locked( w->w_owner ); // need?
    //win_make_decorations(w);
    drv_video_window_enter_allwq(w);
    win_make_decorations(w);
    w_unlock();
}


void drv_video_window_enter_allwq( drv_video_window_t *w)
{
    w_assert_lock();
    queue_enter(&allwindows, w, drv_video_window_t *, chain);
    drv_video_window_rezorder_all();
}


void drv_video_window_destroy(drv_video_window_t *w)
{
    if( focused_window == w )
    {
        event_q_put_win( w->x, w->y, UI_EVENT_WIN_LOST_FOCUS, focused_window );
        focused_window = 0;
    }

    if(!(w->flags & WFLAG_WIN_NOTINALL))
    {
        w_lock();
        queue_remove(&allwindows, w, drv_video_window_t *, chain);
        w_unlock();
    }
    // This will inform win on its death and unlock event read loop to make
    // sure it doesnt loop in dead window
    event_q_put_win( w->x, w->y, UI_EVENT_WIN_DESTROYED, focused_window );


    if(w->buttons)
        destroy_buttons_pool(w->buttons);

    {
    video_zbuf_reset_win( w );
    rect_t wsize;
    drv_video_window_get_bounds( w, &wsize );

    ui_event_t e;
    //struct ui_event e;
    e.w.info = UI_EVENT_GLOBAL_REPAINT_RECT;
    e.w.rect = wsize;

    event_q_put_global( &e );
    }


    // drain event queue
    if( w->events_count > 0 )
    {
        struct ui_event e;

        // Wait to make sure regular event pump drv_video_window_get_event is done
        hal_sleep_msec( 200 ); 

        while( w->events_count > 0 )
            drv_video_window_get_event( w, &e, 0 );
    }

#if !VIDEO_T_IN_D
    if(w->w_title)
        drv_video_window_free(w->w_title);
    w->w_title = 0;
#endif

    if(w->w_decor)
        drv_video_window_free(w->w_decor);
    w->w_decor = 0;
}


#if !SCREEN_UPDATE_THREAD
#warning no repaint thread
void drv_video_window_repaint_all(void)
{
    // redraw all here, or ask some thread to do that
    drv_video_window_t *w;

    w_lock();

    queue_iterate(&allwindows, w, drv_video_window_t *, chain)
    {
        _drv_video_winblt_locked( w );
        if(w->flags & WFLAG_WIN_DECORATED)
            win_make_decorations(w);
    }
    w_unlock();
}
#endif



// todo in screen coordinates, wtodo in win
void repaint_win_part( drv_video_window_t *w, rect_t *wtodo, rect_t *todo  )
{
    int dst_xpos = wtodo->x + w->x; // todo->x
    int dst_ypos = wtodo->y + w->y; // todo->y

    u_int32_t flags;
    win2blt_flags( &flags, w );

    drv_video_bitblt_part(
                          w->pixel,
                          w->xsize, w->ysize,
                          wtodo->x, wtodo->y,
                          dst_xpos, dst_ypos,
                          wtodo->xsize, wtodo->ysize,
                          1, w->z, flags);

}



void repaint_all_for_square( rect_t *todo )
{
    // redraw all here, or ask some thread to do that
    drv_video_window_t *w;

    w_lock();

    queue_iterate(&allwindows, w, drv_video_window_t *, chain)
    //queue_iterate_back(&allwindows, w, drv_video_window_t *, chain)
    {
        rect_t wtodo = *todo;
        wtodo.x -= w->x;
        wtodo.y -= w->y;

        if( !rect_win_bounds( &wtodo, w ) )
        {
            repaint_win_part( w, &wtodo, todo );
            //_drv_video_winblt_locked( w );
            if(w->flags & WFLAG_WIN_DECORATED)
            {
                //win_make_decorations(w);
                win_draw_decorations(w);
            }
        }
    }

    w_unlock();
}




/*!
 *
 * Reset Z order for all the windows.
 *
 */
void drv_video_window_rezorder_all(void)
{
    drv_video_window_t *w;

    w_assert_lock();

    int next_z = 0;
    queue_iterate(&allwindows, w, drv_video_window_t *, chain)
    {
        // Actually start with 2, leave 0 for background and 1 for decorations of bottom win
        next_z += 2;

        w->state &= ~WSTATE_WIN_UNCOVERED; // mark all windows as possibly covered

        if( w->w_owner != 0 )
            continue;

        if( w->z == next_z+1 )
            continue;

        w->z = next_z+1;

#if !VIDEO_T_IN_D
        if( 0 != w->w_title )
        {
            w->w_title->z = next_z;
            video_zbuf_reset_win( w->w_title );
        }
#endif

        if( 0 != w->w_decor )
        {
            w->w_decor->z = next_z;
            video_zbuf_reset_win( w->w_decor );
        }

        video_zbuf_reset_win( w ); // TODO rezet z buf in event too?
    }

    int top_one = 1;
    // From top to bottom
    queue_iterate_back(&allwindows, w, drv_video_window_t *, chain)
    {
        // Don't touch children (decor & title)
        if( w->w_owner == 0 )
        {
            if(top_one)
            {
                top_one = 0;
                w->state |= WSTATE_WIN_UNCOVERED; // mark top one as uncovered
                if(w->w_decor)
                    w->w_decor->state |= WSTATE_WIN_UNCOVERED; // and its decor subwin too
            }

            event_q_put_win( 0, 0, UI_EVENT_WIN_REDECORATE, w );
        }
    }
}





window_handle_t drv_video_next_window(window_handle_t curr)
{
    int reasonable_tries = 1000;
    window_handle_t next = curr;

    w_lock();

    if(next == 0)
        next = (window_handle_t)queue_first(&allwindows);

    do {

        SHOW_FLOW( 3, "next = %x", (int)next );

        if(reasonable_tries-- <= 0)
        {
            SHOW_ERROR0( 0, "loop?");
            w_unlock();
            return curr;
        }

        next = (window_handle_t)queue_next(&next->chain);
        if(queue_end(&allwindows, (void *)next))
            next = (window_handle_t)queue_first(&allwindows);

        // I'm decoration, don't choose me
        if( next->w_owner )
        {
            SHOW_FLOW( 4, "next = %x has owner", (int)next );
            continue;
        }

        // Don't choose unvisible ones
        if( !(next->state & WSTATE_WIN_VISIBLE) )
        {
            SHOW_FLOW( 4, "next = %x not visible", (int)next );
            continue;
        }

        // No focus means no focus
        if( next->flags & WFLAG_WIN_NOFOCUS )
        {
            SHOW_FLOW( 4, "next = %x nofocus", (int)next );
            continue;
        }

        if( next->flags & WFLAG_WIN_NOTINALL )
        {
            SHOW_ERROR0( 0, "WFLAG_WIN_NOTINALL in allw");
            continue;
        }

        w_unlock();
        return next;
    } while(next != curr);

    w_unlock();
    // I'm alone here
    return curr;
}


static void
do_window_resize( drv_video_window_t *w, int xsize, int ysize )
{
    panic("not impl");
}



// faster, but has artefacts at top when move down
#if 0

void
drv_video_window_move( drv_video_window_t *w, int x, int y )
{
    ui_event_t e1, e2;
    rect_t oldw;
    rect_t neww;

    //w->state &= ~WSTATE_WIN_UNCOVERED; // mark as possibly covered

    e1.w.info = UI_EVENT_GLOBAL_REPAINT_RECT;
    e2.w.info = UI_EVENT_GLOBAL_REPAINT_RECT;

    w_lock();

    oldw.x = w->x;
    oldw.y = w->y;
    oldw.xsize = w->xsize;
    oldw.ysize = w->ysize;

    // Suppose that decor overlaps us. If not - use add.
    if( w->w_decor )
    {
        oldw.x = w->w_decor->x;
        oldw.y = w->w_decor->y;
        oldw.xsize = w->w_decor->xsize;
        oldw.ysize = w->w_decor->ysize;
        video_zbuf_reset_square( w->w_decor->x, w->w_decor->y, w->w_decor->xsize, w->w_decor->ysize );
    }
    else
        video_zbuf_reset_square( w->x, w->y, w->xsize, w->ysize );

    w->x = x;
    w->y = y;
    win_move_decorations(w);

    neww.x = w->x;
    neww.y = w->y;
    neww.xsize = w->xsize;
    neww.ysize = w->ysize;

    // Suppose that decor overlaps us. If not - use add.
    if( w->w_decor )
    {
        neww.x = w->w_decor->x;
        neww.y = w->w_decor->y;
        neww.xsize = w->w_decor->xsize;
        neww.ysize = w->w_decor->ysize;
    }

    int o2 = rect_sub( &e1.w.rect, &e2.w.rect, &oldw, &neww );

    video_zbuf_reset_square( e1.w.rect.x, e1.w.rect.y, e1.w.rect.xsize, e1.w.rect.ysize );
    event_q_put_global( &e1 );

    //if( o2 )
    {
        video_zbuf_reset_square( e2.w.rect.x, e2.w.rect.y, e2.w.rect.xsize, e2.w.rect.ysize );
        event_q_put_global( &e2 );
    }

    event_q_put_win( 0, 0, UI_EVENT_WIN_REDECORATE, w );

    w_unlock();
}

#else


void
drv_video_window_move( drv_video_window_t *w, int x, int y )
{
    ui_event_t e1, e2, e3;

    w_lock();

    //w->state &= ~WSTATE_WIN_UNCOVERED; // mark as possibly covered

    e1.w.info = UI_EVENT_GLOBAL_REPAINT_RECT;
    e2.w.info = UI_EVENT_GLOBAL_REPAINT_RECT;
    e3.w.info = UI_EVENT_GLOBAL_REPAINT_RECT;

    e1.w.rect.x = w->x;
    e1.w.rect.y = w->y;
    e1.w.rect.xsize = w->xsize;
    e1.w.rect.ysize = w->ysize;

#if !VIDEO_T_IN_D
    if( w->w_title )
    {
        e2.w.rect.x = w->w_title->x;
        e2.w.rect.y = w->w_title->y;
        e2.w.rect.xsize = w->w_title->xsize;
        e2.w.rect.ysize = w->w_title->ysize;
    }
#endif

    if( w->w_decor )
    {
        e3.w.rect.x = w->w_decor->x;
        e3.w.rect.y = w->w_decor->y;
        e3.w.rect.xsize = w->w_decor->xsize;
        e3.w.rect.ysize = w->w_decor->ysize;
    }


    //int ox = w->x;
    //int oy = w->x;

    video_zbuf_reset_square( w->x, w->y, w->xsize, w->ysize );
#if !VIDEO_T_IN_D
    if( w->w_title )
        video_zbuf_reset_square( w->w_title->x, w->w_title->y, w->w_title->xsize, w->w_title->ysize );
#endif
    if( w->w_decor )
        video_zbuf_reset_square( w->w_decor->x, w->w_decor->y, w->w_decor->xsize, w->w_decor->ysize );

    w->x = x;
    w->y = y;
    win_move_decorations(w);


#if 1



    event_q_put_global( &e1 );

#if !VIDEO_T_IN_D
    if( w->w_title )
        event_q_put_global( &e2 );
#endif

    if( w->w_decor )
        event_q_put_global( &e3 );

#else
    drv_video_window_repaint_all();
#endif

    //event_q_put_win( 0, 0, UI_EVENT_WIN_REPAINT, w );
    event_q_put_win( 0, 0, UI_EVENT_WIN_REDECORATE, w );

    w_unlock();
}
#endif

void
drv_video_window_resize( drv_video_window_t *w, int xsize, int ysize )
{
    rect_t oldsize;
    rect_t newsize;
    rect_t maxsize;

    //w->state &= ~WSTATE_WIN_UNCOVERED; // mark as possibly covered

    drv_video_window_get_bounds( w, &oldsize );
    do_window_resize(w, xsize, ysize);
    drv_video_window_get_bounds( w, &newsize );

    rect_add( &maxsize, &oldsize, &newsize );

    video_zbuf_reset_square( maxsize.x, maxsize.y, maxsize.xsize, maxsize.ysize );

    {
    ui_event_t e;
    //struct ui_event e;
    e.w.info = UI_EVENT_GLOBAL_REPAINT_RECT;
    e.w.rect = maxsize;

    event_q_put_global( &e );
    }
}


void
drv_video_window_get_bounds( drv_video_window_t *w, rect_t *out )
{
    assert(out);

    out->x = w->x;
    out->y = w->y;
    out->xsize = w->xsize;
    out->ysize = w->ysize;
}


void drv_video_window_set_title( drv_video_window_t *w, const char *title )
{
    w->title = title;
    //event_q_put_win( 0, 0, UI_EVENT_WIN_REPAINT, w );
    event_q_put_win( 0, 0, UI_EVENT_WIN_REDECORATE, w );
}


int mouse_intersects(struct drv_video_screen_t *video_drv, int xpos, int ypos, int xsize, int ysize )
{
    rect_t r;

    r.x = xpos;
    r.y = ypos;
    r.xsize = xsize;
    r.ysize = ysize;

    rect_t m;

    r.x = video_drv->mouse_x;
    r.y = video_drv->mouse_y;
    r.xsize = 16; // BUG mouse size hardcode
    r.ysize = 16;

    rect_t o;

    return rect_mul( &o, &r, &m );
}




void mouse_disable_p(struct drv_video_screen_t *video_drv, int xpos, int ypos, int xsize, int ysize )
{
    if( mouse_intersects(video_drv, xpos, ypos, xsize, ysize ) )
        video_drv->mouse_disable();
}


void mouse_enable_p(struct drv_video_screen_t *video_drv, int xpos, int ypos, int xsize, int ysize )
{
    //if( mouse_intersects(video_drv, xpos, ypos, xsize, ysize ) )
        video_drv->mouse_enable();
}




void drv_video_window_to_bottom(drv_video_window_t *w)
{
    w_lock();

    w->state &= ~WSTATE_WIN_UNCOVERED; // mark as possibly covered

    queue_remove(&allwindows, w, drv_video_window_t *, chain);
    queue_enter_first(&allwindows, w, drv_video_window_t *, chain);
    drv_video_window_rezorder_all();

    w_unlock();

    drv_video_window_repaint_all();
}

void drv_video_window_to_top(drv_video_window_t *w)
{
    w_lock();

    w->state &= ~WSTATE_WIN_UNCOVERED; // mark as possibly covered

    queue_remove(&allwindows, w, drv_video_window_t *, chain);
    queue_enter(&allwindows, w, drv_video_window_t *, chain);
    drv_video_window_rezorder_all();

    w_unlock();

    drv_video_window_repaint_all();
}


#include <kernel/debug.h>

void phantom_dump_windows_buf(char *bp, int len)
{
    drv_video_window_t *w;

    w_lock();
    queue_iterate(&allwindows, w, drv_video_window_t *, chain)
    {
        int pn = snprintf( bp, len, "%s%3dx%3d @%3dx%3d z %2d fl%b st%b %s%.10s\x1b[37m\n",
                           (w->state & WSTATE_WIN_FOCUSED) ? "\x1b[32m" : "",
                           w->xsize, w->ysize, w->x, w->y, w->z,
                           w->flags, "\020\1Decor\2NotInAll\3NoFocus\4NoPixels",
                           w->state, "\020\1Focused\2Dragged\3Visible",
                           (w->stall ? "STALL " : ""),
                           //w->events_count, w->owner,
                           (w->title ? w->title : "??")
                         );
        len -= pn;
        bp += pn;
    }
    w_unlock();
}


