/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2009 Dmitry Zavalishin, dz@dz.ru
 *
 * Path manipulations and name to "inode" mapping
 *
**/
#if HAVE_UNIX

#define DEBUG_MSG_PREFIX "syscall"
#include "debug_ext.h"
#define debug_level_flow 6
#define debug_level_error 10
#define debug_level_info 10


#include <unix/uufile.h>
#include <unix/uuprocess.h>
#include <kernel/unix.h>
#include <stdio.h>

//static uufs_t *rootfs;
extern struct uufs root_fs;

// Creates uufile for given path name
uufile_t *uu_namei(const char *filename)
{
    if( filename[0] == '/' && filename[1] == 0 )
        return copy_uufile( root_fs.root(&root_fs) );

    return root_fs.namei(&root_fs, filename);
}

errno_t uu_absname( char *opath, const char *base, const char *_add )
{
    assert(opath);
    assert(base);

    const char *add = _add;
    if( !add ) add = "";

    if( strlen(base) + strlen(add) + 3 > FS_MAX_PATH_LEN )
        return ENOMEM;

    assert(base);

    opath[0] = '/';
    opath[1] = 0;
    strlcat( opath, base, FS_MAX_PATH_LEN );
    if(_add)
    {
        strlcat( opath, "/", FS_MAX_PATH_LEN );
        strlcat( opath, add, FS_MAX_PATH_LEN );
    }

    SHOW_FLOW( 11, "cat      '%s'", opath );

    // Convert multiple slashes to one

    char *p;

    for( p = opath; *p; p++ )
    {
        while( p[0] == '/' && p[1] == '/' )
        {
            //memmove( p, p+1 );
            strcpy( p, p+1 );
        }
    }

    SHOW_FLOW( 11, "kill //  '%s'", opath );

    // Kill /. at end and replace /./ with /

    for( p = opath; *p; p++ )
    {
        while( p[0] == '/' && p[1] == '.' && (p[2] == '/' || p[2] == 0 ) )
        {
            //memmove( p, p+2 );
            strcpy( p, p+2 );
        }
    }

    SHOW_FLOW( 11, "kill /./ '%s'", opath );

    // Edit off ..

    for( p = opath; *p; p++ )
    {
        while( p[0] == '/' && p[1] == '.' && p[2] == '.' && ( (p[3] == '/') || (p[3] == 0) ) )
        {
            const char *rest = p+3;

            //scan back to prev slash

            if(p > opath) p--; // go before current /
            while( (p > opath) && (*p != '/') )
                p--;

            // p is either at opath beginnig or at prev slash
            //memmove( p, rest );
            SHOW_FLOW( 11, "mv '%s' -> '%s' (p %p, opath %p)", rest, p, p, opath );
            strcpy( p, rest );

            // Now we will retry from the place we copied rest of path to
        }
        // Check before for's p++
        // We killed all, nothing to do
        if( *p == 0 )
            break;
    }

    // Killed all? :)
    if( *opath == 0 )
    {
        opath[0] = '/';
        opath[1] = 0;
    }

    SHOW_FLOW( 11, "done     '%s'", opath );
    SHOW_FLOW( 10, "/ + '%s' + / + '%s' -> '%s'", base, add, opath );
    return 0;
}


int uu_break_path( const char *in, int maxpart, const char *oname[], int olen[] )
{
    int npart = 0;

    if( *in == '/' ) in++;

    const char *p = in;

    while(npart < maxpart)
    {
        oname[npart] = p;

        const char *end = strchr( p, '/' );
        if( end )
        {
            olen[npart] = end-p;
            p = end+1;
            npart++;
            continue;
        }
        else
        {
            olen[npart] = strlen(p);
            npart++;
            break;
        }

    }

    return npart;
}


#endif // HAVE_UNIX

