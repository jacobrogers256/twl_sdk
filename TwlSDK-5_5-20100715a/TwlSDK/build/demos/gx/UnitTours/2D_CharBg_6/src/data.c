/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_CharBg_6
  File:     data.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include "data.h"

/*---------------------- Palette Data  ---------------------------*/
const unsigned int d_64_256_bg_sclDT[8 * 16] = {
    0x628a7ffe, 0x6f0d6acb, 0x6aec6f2f, 0x77757352,     // 0000h
    0x51e6460d, 0x66f05a27, 0x41845a8f, 0x49c53943,
    0x41c73567, 0x4e2c5668, 0x62ad41cb, 0x566c2924,     // 0010h
    0x5eb1524f, 0x45e93566, 0x1ca16f55, 0x24e26b13,
    0x31442d02, 0x10607798, 0x7bba39aa, 0x7bdc779b,     // 0020h
    0x66c90820, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0030h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0040h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0050h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0060h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0070h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0080h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0090h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 00a0h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 00b0h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 00c0h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 00d0h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 00e0h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 00f0h
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

/*---------------------- Character Data  -------------------------*/
const unsigned int d_64_256_bg_schDT[16 * 128] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0000h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0001h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x0f000000,
    0x00000000, 0x090d0000, 0x00000000, 0x0f0f0d0f,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0002h
    0x00000000, 0x01010000, 0x09000000, 0x0201010a,
    0x0f0e0d00, 0x0101010a, 0x090f0f0f, 0x1101010a,
    0x0a090f09, 0x11010101, 0x010a090f, 0x11010109,
    0x00000000, 0x00000000, 0x02010000, 0x04030302,     // 0003h
    0x05020201, 0x04040403, 0x03050101, 0x06040404,
    0x04121110, 0x06060604, 0x17111016, 0x0b070603,
    0x1b111a1a, 0x0815171b, 0x17131b1b, 0x08080817,
    0x00000000, 0x00000000, 0x04030304, 0x00000304,     // 0004h
    0x06070706, 0x06070706, 0x07060b04, 0x07070707,
    0x06151413, 0x06070b06, 0x0c080808, 0x18181318,
    0x0c180808, 0x18180c18, 0x0c18080c, 0x0c0c0c18,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0005h
    0x00000808, 0x00000000, 0x0b0c0b06, 0x00000006,
    0x0707060b, 0x00080b07, 0x07060c18, 0x13190b07,
    0x07060c18, 0x180b1c07, 0x0b0c1919, 0x1c060707,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0006h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x0000000c, 0x00000000,
    0x00000b06, 0x00000000, 0x0b0b0707, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0007h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0008h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0009h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 000ah
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 000bh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 000ch
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 000dh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 000eh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 000fh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0010h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x0d000000, 0x00000000, 0x23000000,
    0x00000000, 0x1d0e0000, 0x00000000, 0x0e0e0e00,
    0x1d000000, 0x0f090d0d, 0x0f0f0000, 0x0a0a0f0d,     // 0011h
    0x1f0e0f00, 0x0a0a0d1d, 0x201d0e0f, 0x21090f0d,
    0x0f1d0e0f, 0x16200f0f, 0x0f0e0e20, 0x201f160d,
    0x0f090f0e, 0x161f2016, 0x0d0f090d, 0x1f1f0e1f,
    0x01010a0d, 0x1b020201, 0x0101010a, 0x03030502,     // 0012h
    0x0201010a, 0x04030201, 0x01010a21, 0x04040111,
    0x11112121, 0x1317021b, 0x1a1a2121, 0x1b241b16,
    0x16090a09, 0x1b101b10, 0x1a020201, 0x151b1b1b,
    0x0604171b, 0x0c180c06, 0x0606171b, 0x180b0b06,     // 0013h
    0x06041313, 0x0b0c180b, 0x0c0c130b, 0x0b0c0c0c,
    0x08150b17, 0x180c1813, 0x13041314, 0x19181308,
    0x0c0c141b, 0x19181813, 0x17141b15, 0x0c181808,
    0x180c0b0b, 0x190c0c0c, 0x0c0c070b, 0x070c0c0c,     // 0014h
    0x0c181e07, 0x0707190c, 0x0c0c0b1e, 0x0722071e,
    0x19180c18, 0x1c220707, 0x1e190c0b, 0x1c1c0707,
    0x1c070707, 0x25071c07, 0x07070707, 0x1c1c0722,
    0x190c0c1e, 0x07070706, 0x1919191e, 0x19191e1e,     // 0015h
    0x1e19191e, 0x1919191e, 0x1c1e1e1c, 0x1e19191e,
    0x221e1e1e, 0x1e1e1e22, 0x221c1c1c, 0x1c220722,
    0x221c1c25, 0x22222225, 0x22262525, 0x25252525,
    0x0707071c, 0x0000000b, 0x071c190c, 0x00000b06,     // 0016h
    0x07061919, 0x000b061c, 0x1e1e1e1e, 0x0b0b0707,
    0x071e1e1e, 0x06060707, 0x07071e1c, 0x1c07071c,
    0x1e070722, 0x1c071e19, 0x1e1c2222, 0x0707070b,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0017h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0000000c, 0x00000000, 0x00000006, 0x00000000,
    0x00000c0b, 0x00000000, 0x000c0b06, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0018h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0019h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 001ah
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 001bh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 001ch
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 001dh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 001eh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 001fh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x20200e00, 0x00000000, 0x200e0e1d,     // 0020h
    0x00000000, 0x0d0e201d, 0x0e000000, 0x0e201f20,
    0x20000000, 0x0d0e0e0d, 0x20000000, 0x0f0f0d0e,
    0x1f200000, 0x0d0f0f20, 0x200e0000, 0x200d0e1d,
    0x1f0d090d, 0x0921211f, 0x2009090d, 0x0a211621,     // 0021h
    0x0d09090f, 0x0a201f21, 0x0909090f, 0x090d0d0f,
    0x0f090f0f, 0x1f1a090f, 0x0a0f0f09, 0x110a0909,
    0x0a0d0d09, 0x01110a01, 0x0a210f0f, 0x0a090102,
    0x1a020502, 0x0b1b101b, 0x1a120505, 0x04131010,     // 0022h
    0x11161103, 0x0404131a, 0x1111110a, 0x04060b1a,
    0x1a1b0a21, 0x0c03151b, 0x101b020a, 0x24111a10,
    0x1b120105, 0x0b171313, 0x1b110102, 0x0b131b15,
    0x0c141506, 0x19181819, 0x0c140b06, 0x07190c0b,     // 0023h
    0x14140b06, 0x071c1e06, 0x08081904, 0x07070618,
    0x13142414, 0x07071813, 0x130c1414, 0x1c1e1918,
    0x0c040604, 0x0c180808, 0x14080b0b, 0x0c181808,
    0x221e1e1c, 0x221c1c1c, 0x221e191e, 0x25221c22,     // 0024h
    0x221e191e, 0x25252225, 0x221e1919, 0x25221c22,
    0x1c1e1e07, 0x25221c07, 0x1e1e1c07, 0x221c1c1e,
    0x2207071e, 0x221c1c22, 0x1e19190c, 0x22221e1e,
    0x25252525, 0x25252626, 0x27272525, 0x22262627,     // 0025h
    0x27272725, 0x22262727, 0x27272725, 0x22262727,
    0x27272725, 0x25252727, 0x27272525, 0x25252527,
    0x25252525, 0x25252525, 0x25252522, 0x25252525,
    0x07222222, 0x07070707, 0x22222222, 0x07070722,     // 0026h
    0x22221c1c, 0x07071e07, 0x22071c1c, 0x190c0c19,
    0x1c071c22, 0x180c191e, 0x07222222, 0x180c1e1e,
    0x07070722, 0x18191c1e, 0x071c1c22, 0x191e1c07,
    0x000c0c1e, 0x00000000, 0x13131819, 0x00000000,     // 0027h
    0x17180c06, 0x00000000, 0x14180b1e, 0x00000014,
    0x08130b0b, 0x00000014, 0x130c0b0b, 0x00000008,
    0x080c0b19, 0x00001308, 0x080c060b, 0x00001408,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0028h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0029h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 002ah
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 002bh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 002ch
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 002dh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 002eh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 002fh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x201d0000, 0x090f0e23, 0x201f0000, 0x090f0f20,     // 0030h
    0x0d0d2000, 0x090f0f0d, 0x0d0d0e00, 0x090f0f0f,
    0x0d0d2000, 0x0f090f0d, 0x0d0e1d00, 0x0e090f0f,
    0x0e1d1d00, 0x200f0f0f, 0x1d1f2000, 0x0d0e0f0f,
    0x0f200d09, 0x02020202, 0x0f0e0d0a, 0x0201010a,     // 0031h
    0x090f090a, 0x020a010a, 0x0d210d0f, 0x0502010a,
    0x1f1f231d, 0x0a0a0a20, 0x20201d23, 0x1f20161d,
    0x1f201d1d, 0x201f1d1d, 0x1f1f0e0e, 0x211d1f1f,
    0x1b111102, 0x1311241b, 0x11112112, 0x1b1b1110,     // 0032h
    0x1a111b02, 0x141b1116, 0x10111103, 0x13150b13,
    0x15101a11, 0x04040404, 0x1b1a1a1f, 0x06040415,
    0x101a1a1a, 0x04040413, 0x1b112111, 0x24241b1b,
    0x14141b14, 0x190c0c13, 0x08131414, 0x18180c08,     // 0033h
    0x08142414, 0x180c0b0c, 0x08131713, 0x0c0b070b,
    0x17131315, 0x0b06070b, 0x15241404, 0x080c0406,
    0x13141b0b, 0x0814130c, 0x1414241b, 0x13081314,
    0x1919190c, 0x25071e19, 0x190c1818, 0x22221e19,     // 0034h
    0x19191813, 0x22221c19, 0x0c190c0c, 0x07220719,
    0x0b18180c, 0x07070706, 0x07190c18, 0x07070707,
    0x1e181813, 0x07070707, 0x180c1813, 0x07070618,
    0x25222222, 0x22252525, 0x22072222, 0x07222222,     // 0035h
    0x22072222, 0x1e071e1c, 0x1c070707, 0x1e071c1e,
    0x07070707, 0x07070707, 0x07070707, 0x071e061e,
    0x07070707, 0x1c1e0b1e, 0x07070707, 0x1c1e0707,
    0x07071c07, 0x06061c07, 0x071e1e07, 0x06061c1c,     // 0036h
    0x0719191e, 0x06061c06, 0x1e1e0619, 0x1c1c060b,
    0x180c0707, 0x06071c19, 0x18181e22, 0x06061c0c,
    0x18190707, 0x0606060c, 0x06060707, 0x06060604,
    0x0b06060b, 0x00001717, 0x0b0c0b0c, 0x00000403,     // 0037h
    0x1914180b, 0x00130b04, 0x15170606, 0x00130c0c,
    0x0b0b0606, 0x00131b14, 0x06060606, 0x00041413,
    0x030b0606, 0x000b1413, 0x24241706, 0x000b1414,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0038h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0039h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 003ah
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 003bh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 003ch
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 003dh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 003eh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 003fh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x20200e00, 0x0f0d090f, 0x0e1d2000, 0x2021210f,     // 0040h
    0x0d201d00, 0x1f0f0f0f, 0x0d0e2000, 0x0f090d0d,
    0x0d0d0e00, 0x090f0d0d, 0x0d0d0e00, 0x0f090f0f,
    0x0f0e0000, 0x1f0f0f0f, 0x20230000, 0x230e0f0f,
    0x1d0e0909, 0x11211f1d, 0x0d09090d, 0x090d200d,     // 0041h
    0x0a0a0d1f, 0x020a090a, 0x0a09201f, 0x0201010a,
    0x09090d0e, 0x01020a0f, 0x0909090d, 0x020a0d0f,
    0x0a090f1d, 0x09200e0a, 0x090f0920, 0x161f210f,
    0x1b1b1a1a, 0x1b111010, 0x05151201, 0x17030117,     // 0042h
    0x05030502, 0x04041701, 0x02010502, 0x04041717,
    0x05010101, 0x03151112, 0x02010102, 0x151b1112,
    0x17120a0a, 0x1b161202, 0x11111a1f, 0x12161112,
    0x1314141b, 0x08131313, 0x1314141b, 0x13140813,     // 0043h
    0x24241013, 0x17241b1b, 0x13131504, 0x17241414,
    0x13130404, 0x04132413, 0x04040404, 0x04031703,
    0x04060404, 0x04040404, 0x04040604, 0x03030b04,
    0x0b060c08, 0x07070b0b, 0x0604060b, 0x0604060c,     // 0044h
    0x0b040606, 0x0b0b0617, 0x0b040606, 0x0b040404,
    0x0b0b0406, 0x0b0b190b, 0x15030304, 0x0b15130b,
    0x03030304, 0x1b0b0b0b, 0x03030b0b, 0x240b0b03,
    0x071c061c, 0x06060707, 0x06060606, 0x06060606,     // 0045h
    0x0606040b, 0x0c0b0404, 0x06060b0c, 0x170c0406,
    0x06040b03, 0x0404040b, 0x0b041513, 0x040b150b,
    0x0b040b1b, 0x15151503, 0x030b0414, 0x03170303,
    0x0606071c, 0x0b0b0b0b, 0x0604040b, 0x04030b0b,     // 0046h
    0x0b040b15, 0x1504040b, 0x0b150b0b, 0x0b040603,
    0x0b17150b, 0x04040b04, 0x0b150b03, 0x04151503,
    0x15040404, 0x04030303, 0x15040404, 0x03121215,
    0x13241413, 0x00051b1b, 0x04131b17, 0x00151503,     // 0047h
    0x040b1b17, 0x00171704, 0x13150b17, 0x00010117,
    0x10111504, 0x00010512, 0x10111504, 0x0001051b,
    0x11130504, 0x00000312, 0x13111703, 0x00000104,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0048h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0049h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 004ah
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 004bh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 004ch
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 004dh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 004eh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 004fh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1d280000, 0x1d231d1d, 0x23280000, 0x0e231d1d,     // 0050h
    0x20000000, 0x0f1d1d0e, 0x20000000, 0x090d0d0e,
    0x0e000000, 0x090f0e0d, 0x00000000, 0x0f0d200e,
    0x00000000, 0x0e0d0d0e, 0x00000000, 0x23200e00,
    0x200f0a0f, 0x0d201d1d, 0x0d0a0909, 0x011f1f20,     // 0051h
    0x0a21200a, 0x0f0d0d0f, 0x0a1f0e09, 0x0f210f01,
    0x0a0a0e0d, 0x2020090a, 0x1d0d0d0d, 0x211d200e,
    0x231d1d1d, 0x0a1d231d, 0x23282323, 0x011f1f0f,
    0x1a1a1209, 0x03131111, 0x16120305, 0x121b101a,     // 0052h
    0x21111209, 0x12161111, 0x1221090a, 0x1b160911,
    0x0101120f, 0x11161a01, 0x0212090a, 0x121a1101,
    0x1e180102, 0x120a1212, 0x14150202, 0x12091b16,
    0x03050303, 0x03031703, 0x0403171b, 0x15151504,     // 0053h
    0x04030505, 0x17150304, 0x04030505, 0x03030304,
    0x03030502, 0x02020404, 0x03051201, 0x17050403,
    0x03051201, 0x05050303, 0x05021212, 0x02050305,
    0x15031515, 0x17171515, 0x03151b1b, 0x04040b15,     // 0054h
    0x12101b17, 0x04040315, 0x1b130504, 0x03040312,
    0x03040101, 0x03030305, 0x0305171b, 0x05030303,
    0x03050502, 0x05030303, 0x03051212, 0x01050303,
    0x15050415, 0x04150517, 0x15151504, 0x04151512,     // 0055h
    0x01050304, 0x04041513, 0x05150303, 0x04040305,
    0x05050503, 0x03040217, 0x05050502, 0x03030117,
    0x03011b02, 0x05030303, 0x03051112, 0x01050503,
    0x03040404, 0x17101715, 0x03040404, 0x121b1205,     // 0056h
    0x05030304, 0x12020215, 0x05040403, 0x01121705,
    0x05030303, 0x01010205, 0x01050505, 0x1b05111a,
    0x1b030505, 0x12011111, 0x01121212, 0x01111205,
    0x031b1203, 0x00001205, 0x05111705, 0x00002111,     // 0057h
    0x1b1b0205, 0x00000016, 0x11010502, 0x00000016,
    0x02011212, 0x00000012, 0x2912011b, 0x00000000,
    0x12120111, 0x00000000, 0x00091112, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0058h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0059h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 005ah
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 005bh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 005ch
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 005dh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 005eh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 005fh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x23281d00, 0x00000000, 0x23280000,     // 0060h
    0x00000000, 0x23000000, 0x00000000, 0x23000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1f231d23, 0x091d0f0e, 0x1f1d1d1d, 0x201d201f,     // 0061h
    0x1d1d1d23, 0x20231d1d, 0x1d1d2323, 0x0e201f23,
    0x1f1d201d, 0x200f1d23, 0x1f1f2300, 0x1d0d1623,
    0x1f230000, 0x1d1d1f23, 0x1d000000, 0x1f1d1f1d,
    0x1f160a02, 0x1a16211d, 0x1f200e0a, 0x1f23231f,     // 0062h
    0x1d0e0e0e, 0x1d1d1d1d, 0x1d1f1d1d, 0x1f1f1d1d,
    0x1d201d1d, 0x0d0f201f, 0x201d1d1d, 0x2902201d,
    0x1d0e2323, 0x090a0d23, 0x231d1d1f, 0x0f0a0a1d,
    0x031b1611, 0x05020503, 0x0112161f, 0x05052902,     // 0063h
    0x12291a1f, 0x09010212, 0x09291220, 0x1f0d0a12,
    0x09290512, 0x0d1f110a, 0x29292929, 0x29090a01,
    0x29120a0a, 0x01292929, 0x2929090f, 0x01091229,
    0x05111f12, 0x12011205, 0x111f1105, 0x11011201,     // 0064h
    0x1f1f1a09, 0x0a120511, 0x16211f1f, 0x01122911,
    0x09090a11, 0x0a290202, 0x29292929, 0x1f12091b,
    0x0d120229, 0x0d090916, 0x1d0d0a29, 0x120a0d1f,
    0x05030212, 0x01020205, 0x05030212, 0x05020205,     // 0065h
    0x03050501, 0x03291b12, 0x02050505, 0x03011111,
    0x05050501, 0x29122929, 0x05022921, 0x01120105,
    0x02022909, 0x09090205, 0x02020229, 0x09120205,
    0x01091f21, 0x161f0d09, 0x12161601, 0x1f16111b,     // 0066h
    0x12110504, 0x21160d01, 0x29010102, 0x12160f01,
    0x01020201, 0x021b1202, 0x12021229, 0x0005020a,
    0x12121201, 0x00002912, 0x2909110a, 0x00000029,
    0x00201101, 0x00000000, 0x00002912, 0x00000000,     // 0067h
    0x00000001, 0x00000000, 0x00000029, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0068h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0069h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 006ah
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 006bh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 006ch
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 006dh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 006eh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 006fh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0070h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x200d2123, 0x00000000, 0x0d0f0000,     // 0071h
    0x00000000, 0x0d000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1d231d1d, 0x0d0d0f1f, 0x20201f20, 0x1d1f0a0d,     // 0072h
    0x0909090d, 0x0e090a0a, 0x20200d00, 0x0a0a0a0e,
    0x23000000, 0x0a091d23, 0x00000000, 0x09090000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x090a090d, 0x12090d0d, 0x0e0e200f, 0x01010e20,     // 0073h
    0x290d1d0d, 0x0d110d0a, 0x010e231d, 0x230d0502,
    0x1d1d200e, 0x23210a0e, 0x200e090a, 0x1d231f1d,
    0x090a0000, 0x1d1f2020, 0x00000000, 0x00000000,
    0x1f1d0d29, 0x200d201d, 0x211f2112, 0x20112120,     // 0074h
    0x1621201f, 0x09161d0d, 0x1d21201f, 0x1d1d1d23,
    0x0d202121, 0x201f210d, 0x0f0d200d, 0x1f1d200f,
    0x0d201d1d, 0x00001d1d, 0x00000000, 0x00000000,
    0x2912090f, 0x0a292929, 0x121d2021, 0x090a0912,     // 0075h
    0x091d210a, 0x09091112, 0x0f1d1111, 0x00090a0f,
    0x091d1d1d, 0x00000009, 0x00001d1d, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0912120a, 0x00000000, 0x00001201, 0x00000000,     // 0076h
    0x0000000a, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0077h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0078h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 0079h
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 007ah
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 007bh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 007ch
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 007dh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 007eh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,     // 007fh
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};


/* EOF */
