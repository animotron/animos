/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2011 Dmitry Zavalishin, dz@dz.ru
 *
 * Kolibri emulation.
 *
**/

#ifndef KOLIBRI_H
#define KOLIBRI_H

#include <phantom_types.h>
#include <errno.h>
#include <string.h>
#include <kernel/sem.h>
#include <video/window.h>

/*
 db 'MENUET01'   ; 1. �����䨪��� �ᯮ��塞��� 䠩�� (8 ����)
 dd 0x01	  ; 2. ����� �ଠ� ��������� �ᯮ��塞��� 䠩��
 dd START	  ; 3. ����, �� ����� ��⥬� ��।��� �ࠢ�����
 dd I_END	  ; 4. ࠧ��� �ਫ������
 dd I_END+0x1000 ; 5. ���� ����室���� �ਫ������ �����
 dd I_END+0x1000 ; 6. ���設� �⥪� � ��������� �����, 㪠������ ���
 dd 0x0	  ; 7. 㪠��⥫� �� ��ப� � ��ࠬ��ࠬ�.
 dd 0x0	  ; 8. 㪠��⥫� �� ��ப�, � ������ ����ᠭ ���� ����᪠
*/

struct kolibri_exe_hdr
{
    char        	ident[8];
    u_int32_t           version;
    u_int32_t           start;
    u_int32_t           code_end;
    u_int32_t           data_end;
    u_int32_t           stack_end;
    u_int32_t           params;
    u_int32_t           exe_name; 	// icon??
};

typedef struct kolibri_exe_hdr kolibri_exe_hdr_t;

static inline errno_t is_not_kolibri_exe( kolibri_exe_hdr_t *exe )
{
    if( strncmp( "MENUET01", exe->ident, 8 ) )
        return ENOEXEC;

    // seems to mean nothing
    //if( exe->version != 1 )        return ENOEXEC;

    return 0;
}

struct kolibri_process_state
{
    hal_sem_t           event; // syscall 23 event
    u_int32_t           event_mask;

    window_handle_t     win;
};


#endif // KOLIBRI_H


