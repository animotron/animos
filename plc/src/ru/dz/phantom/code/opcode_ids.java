package ru.dz.phantom.code;

public class opcode_ids {

  protected static final byte opcode_nop           = 0x00;
  protected static final byte opcode_debug         = 0x01;
  protected static final byte opcode_off_skipz         = 0x02;
  protected static final byte opcode_off_skipnz        = 0x03;
  protected static final byte opcode_djnz          = 0x04;
  protected static final byte opcode_jz            = 0x05;
  protected static final byte opcode_jmp           = 0x06;
  protected static final byte opcode_switch        = 0x07;
  protected static final byte opcode_ret           = 0x08;
  protected static final byte opcode_short_call_0 = (byte)0x09; // 0 parameters shortcut calls
  protected static final byte opcode_short_call_1 = (byte)0x0A;
  protected static final byte opcode_short_call_2 = (byte)0x0B;
  protected static final byte opcode_short_call_3 = (byte)0x0C;
  protected static final byte opcode_call_8bit     = 0x0D;
  protected static final byte opcode_call_32bit    = 0x0E;


  protected static final byte opcode_is_dup        = 0x10;
  protected static final byte opcode_is_drop       = 0x11;
  protected static final byte opcode_os_dup        = 0x12;
  protected static final byte opcode_os_drop       = 0x13;
  protected static final byte opcode_os_load8      = 0x14;
  protected static final byte opcode_os_save8      = 0x15;
  protected static final byte opcode_os_load32     = 0x16;
  protected static final byte opcode_os_save32     = 0x17;

  protected static final byte opcode_new           = 0x18;
  protected static final byte opcode_copy          = 0x19;

  protected static final byte opcode_os_compose32  = 0x1A;
  protected static final byte opcode_os_decompose  = 0x1B;

  protected static final byte opcode_os_pull32     = 0x1C;

  protected static final byte opcode_os_get32      = 0x1E;
  protected static final byte opcode_os_set32      = 0x1F;

  protected static final byte opcode_iconst_0      = 0x20;
  protected static final byte opcode_iconst_1      = 0x21;
  protected static final byte opcode_iconst_8bit   = 0x22;
  protected static final byte opcode_iconst_32bit  = 0x23;
  protected static final byte opcode_sconst_bin    = 0x24;

  protected static final byte opcode_is_get32 = (byte)0x26;	// get value from stack absolute-addressed slot, push on top
  protected static final byte opcode_is_set32 = (byte)0x27;	// pop stack top, set value in stack absolute-addressed slot
  
  protected static final byte opcode_push_catcher  = 0x2D;
  protected static final byte opcode_pop_catcher   = 0x2E;
  protected static final byte opcode_throw         = 0x2F;


  protected static final byte opcode_summon_thread = 0x30;
  protected static final byte opcode_summon_this   = 0x31;

  protected static final byte opcode_summon_null = (byte)0x37; // null object

  protected static final byte opcode_summon_class_class       = 0x38;
  protected static final byte opcode_summon_int_class         = 0x39;
  protected static final byte opcode_summon_string_class      = 0x3A;
  protected static final byte opcode_summon_interface_class   = 0x3B;
  protected static final byte opcode_summon_code_class        = 0x3C;
  protected static final byte opcode_summon_array_class       = (byte)0x3D;

  protected static final byte opcode_summon_by_name           = 0x3F;


  protected static final byte opcode_i2o           = 0x40;
  protected static final byte opcode_o2i           = 0x41;
  protected static final byte opcode_isum          = 0x42;
  protected static final byte opcode_imul          = 0x43;
  protected static final byte opcode_isubul        = 0x44;
  protected static final byte opcode_isublu        = 0x45;
  protected static final byte opcode_idivul        = 0x46;
  protected static final byte opcode_idivlu        = 0x47;

  protected static final byte opcode_ior = 0x48;
  protected static final byte opcode_iand = 0x49;
  protected static final byte opcode_ixor = 0x4A;
  protected static final byte opcode_inot = 0x4B;

  protected static final byte opcode_log_or = 0x4C;
  protected static final byte opcode_log_and = 0x4D;
  protected static final byte opcode_log_xor = 0x4E;
  protected static final byte opcode_log_not = 0x4F;

  protected static final byte opcode_is_load8 = (byte)0x50; // load (push) this object's field on stack top
  protected static final byte opcode_is_save8 = (byte)0x51; // save (pop) stack top to this object's field

  protected static final byte opcode_ige = (byte)0x52; // >=
  protected static final byte opcode_ile = (byte)0x53; // <=
  protected static final byte opcode_igt = (byte)0x54; // >
  protected static final byte opcode_ilt = (byte)0x55; // <

// Compare two object pointers
  protected static final byte opcode_os_eq = 0x58; // pointers are equal
  protected static final byte opcode_os_neq = 0x59; // pointers are not equal
  protected static final byte opcode_os_isnull = 0x5A; // pointer is null
  protected static final byte opcode_os_push_null = 0x5B; // push null on stack


  protected static final byte opcode_call_00 = (byte)0xA0; // shortcut for call 0
  protected static final byte opcode_call_01 = (byte)0xA1;
  protected static final byte opcode_call_02 = (byte)0xA2;
  protected static final byte opcode_call_03 = (byte)0xA3;
  protected static final byte opcode_call_04 = (byte)0xA4;
  protected static final byte opcode_call_05 = (byte)0xA5;
  protected static final byte opcode_call_06 = (byte)0xA6;
  protected static final byte opcode_call_07 = (byte)0xA7;
  protected static final byte opcode_call_08 = (byte)0xA8;
  protected static final byte opcode_call_09 = (byte)0xA9;
  protected static final byte opcode_call_0A = (byte)0xAA;
  protected static final byte opcode_call_0B = (byte)0xAB;
  protected static final byte opcode_call_0C = (byte)0xAC;
  protected static final byte opcode_call_0D = (byte)0xAD;
  protected static final byte opcode_call_0E = (byte)0xAE;
  protected static final byte opcode_call_0F = (byte)0xAF;


  protected static final byte opcode_call_10 = (byte)0xB0; // shortcut for call 16
  protected static final byte opcode_call_11 = (byte)0xB1;
  protected static final byte opcode_call_12 = (byte)0xB2;
  protected static final byte opcode_call_13 = (byte)0xB3;
  protected static final byte opcode_call_14 = (byte)0xB4;
  protected static final byte opcode_call_15 = (byte)0xB5;
  protected static final byte opcode_call_16 = (byte)0xB6;
  protected static final byte opcode_call_17 = (byte)0xB7;
  protected static final byte opcode_call_18 = (byte)0xB8;
  protected static final byte opcode_call_19 = (byte)0xB9;
  protected static final byte opcode_call_1A = (byte)0xBA;
  protected static final byte opcode_call_1B = (byte)0xBB;
  protected static final byte opcode_call_1C = (byte)0xBC;
  protected static final byte opcode_call_1D = (byte)0xBD;
  protected static final byte opcode_call_1E = (byte)0xBE;
  protected static final byte opcode_call_1F = (byte)0xBF;


}
