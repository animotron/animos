#if HAVE_UNIX

#include <unix/uufile.h>
//#include <unix/uuprocess.h>
#include <malloc.h>
#include <string.h>


uufile_t *copy_uufile( uufile_t *in )
{
    uufile_t *out = calloc( 1, sizeof(uufile_t) );
    *out = *in;
    hal_mutex_init( &out->mutex, "uufile" );
    out->pos = 0;

    if( in->name )
        out->name = strdup( in->name );
    else
        out->name = 0;

    if(in->ops && in->ops->copyimpl && in->impl)
        out->impl = in->ops->copyimpl( in->impl );

    out->refcount = 1;
    return out;
}


void set_uufile_name( uufile_t *out, const char *name )
{
    if(out->name) free( (void *)out->name );
    out->name = strdup( name );
}


static uufile_t uuf_template =
{
    // all zeros
    0, 0, 0, 0,
    0, 0, 0, { 0 }
};

uufile_t *create_uufile()
{
    return copy_uufile( &uuf_template );
}

void destroy_uufile(uufile_t *f)
{
    // ToDO close?
    if( f->name ) free((void *)f->name);
    f->name = 0;
    //if( f->impl ) free(f->impl); f->impl = 0;
    //TODO assert mutex is not locked
    free(f);
}



void link_uufile( uufile_t *in )
{
    in->refcount++;
}



void unlink_uufile( uufile_t *in )
{
    in->refcount--;
    if(in->refcount == 0)
        destroy_uufile(in);
}

#endif // HAVE_UNIX

