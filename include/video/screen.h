/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2011 Dmitry Zavalishin, dz@dz.ru
 *
 * Video subsystem - screen structure and ops.
 *
**/

#ifndef SCREEN_H
#define SCREEN_H

#include <video/color.h>
#include <video/zbuf.h>
#include <video/bitmap.h>

/**
 *
 * Represents a videodriver.
 *
 */

struct drv_video_screen_t
{
    const char  *name; // Driver name

    int         xsize;
    int 	ysize;
    //! Bits per pixel
    int 	bpp;

    int 	mouse_x;
    int         mouse_y;
    int         mouse_flags;

    char        *screen;

    // Probe/init/finish

    int         (*probe) (void); // Returns true if hardware present, sets xsize/ysize.
    int         (*start) (void); // Start driver, switch to graphics mode.
    int         (*stop)  (void); // Stop driver, switch to text mode. Can be called in unstable kernel state, keep it simple.

    // Main interface

    void 	(*update) (void);

    void 	(*bitblt) (const rgba_t *from, int xpos, int ypos, int xsize, int ysize, zbuf_t zpos);
#if NEW_WINDOWS
    void 	(*winblt) ( window_handle_t from, rect_t src, int src_stride, int dest_xpos, int dest_ypos, zbuf_t zpos);
#else
    void 	(*winblt) ( const drv_video_window_t *from, int xpos, int ypos, zbuf_t zpos);
#endif
    void 	(*readblt) ( rgba_t *to, int xpos, int ypos, int xsize, int ysize);

    // Callbacks - to be filled by OS before driver init - BUG - kill!

    void        (*mouse)  (void); // mouse activity detected - callback from driver

    // Mouse cursor

    void        (*redraw_mouse_cursor)(void);
    void        (*set_mouse_cursor)(drv_video_bitmap_t *cursor);
    void        (*mouse_disable)(void);
    void        (*mouse_enable)(void);

};

//
extern struct drv_video_screen_t        drv_video_win32;

extern struct drv_video_screen_t        *video_drv;

extern void drv_video_null(void);





extern void drv_video_bitblt_forw(const rgba_t *from, int xpos, int ypos, int xsize, int ysize, zbuf_t zpos );
extern void drv_video_bitblt_rev(const rgba_t *from, int xpos, int ypos, int xsize, int ysize, zbuf_t zpos );

#if !NEW_WINDOWS
extern void drv_video_win_winblt(const drv_video_window_t *from, int xpos, int ypos, zbuf_t zpos);
extern void drv_video_win_winblt_rev(const drv_video_window_t *from, int xpos, int ypos, zbuf_t zpos);
#endif

void drv_video_readblt_forw( rgba_t *to, int xpos, int ypos, int xsize, int ysize);
void drv_video_readblt_rev( rgba_t *to, int xpos, int ypos, int xsize, int ysize);


// RGB videospace access workers
void drv_video_bitblt_reader(rgba_t *to, int xpos, int ypos, int xsize, int ysize, int reverse);
void drv_video_bitblt_worker(const struct rgba_t *from, int xpos, int ypos, int xsize, int ysize, int reverse, zbuf_t zpos);



void mouse_disable_p(struct drv_video_screen_t *video_drv, int xpos, int ypos, int xsize, int ysize );
void mouse_enable_p(struct drv_video_screen_t *video_drv, int xpos, int ypos, int xsize, int ysize );


// -----------------------------------------------------------------------
// Mode switch
// -----------------------------------------------------------------------

//! Switch video bitblt functions, used to read/write videomem, to 32 bit mode. Default is 24 bit mode.
void switch_screen_bitblt_to_32bpp(int use32bpp );

// TODO call from text vga console drvr?
// Can be called from any driver (or anywhere) to reset VGA to text mode
extern void video_drv_basic_vga_set_text_mode(void);

void phantom_enforce_video_driver(struct drv_video_screen_t *vd);
void set_video_driver_bios_vesa_pa( physaddr_t pa, size_t size );
void set_video_driver_bios_vesa_mode( u_int16_t mode );

// If 1, VESA will be used if found, even if other driver is found
// If 0, VESA will fight for itself as usual driver
// Now using 1, kernel traps if trying to do VM86 too late in boot process
#define VESA_ENFORCE 1

void setTextVideoMode(void); // Using int10
int setVesaMode( u_int16_t mode ); // Using int10


// -----------------------------------------------------------------------
// Drivers
// -----------------------------------------------------------------------

extern struct drv_video_screen_t        video_driver_bochs_vesa_emulator;
extern struct drv_video_screen_t        video_driver_basic_vga;
extern struct drv_video_screen_t        video_driver_direct_vesa;
extern struct drv_video_screen_t        video_driver_bios_vesa;
extern struct drv_video_screen_t        video_driver_cirrus;
extern struct drv_video_screen_t        video_driver_gen_clone;


// -----------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------

#define drv_video_update() video_drv->update()
#define drv_video_readblt(from, xpos, ypos, xsize,ysize) ( video_drv->mouse_disable(), video_drv->readblt(from, xpos, ypos, xsize,ysize), video_drv->mouse_enable() )


#define drv_video_bitblt(___from, xpos, ypos, xsize, ysize, zpos)  ( \
    mouse_disable_p(video_drv, xpos, ypos, xsize, ysize), \
    video_drv->bitblt((___from), xpos, ypos, xsize, ysize, zpos), \
    mouse_enable_p(video_drv, xpos, ypos, xsize, ysize ) )



// These are special for mouse pointer code - they're not try to disable mouse
#define drv_video_readblt_ms(from, xpos, ypos, xsize,ysize) video_drv->readblt(from, xpos, ypos, xsize,ysize )

#define drv_video_bitblt_ms(from, xpos, ypos, xsize, ysize) video_drv->bitblt(from, xpos, ypos, xsize, ysize, 0xFF)

#define drv_video_set_mouse_cursor(nc) 		video_drv->set_mouse_cursor(nc)
#define drv_video_draw_mouse()            	video_drv->redraw_mouse_cursor()
#define drv_video_mouse_off()             	video_drv->mouse_disable()
#define drv_video_mouse_on()              	video_drv->mouse_enable()




#endif // SCREEN_H

