/*---------------------------------------------------------------------------*
  Project:  TwlSDK - HW - include
  File:     mmap_main.h

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
#ifndef NITRO_HW_MMAP_MAIN_H_
#define NITRO_HW_MMAP_MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// MEMORY MAP of MAIN MEMORY
//---------------------------------------------------------------------------
//
//
//      Retail NITRO            Development NITRO 
//      +---------------+-------+---------------+--8.000M
//      | System Shared |       | System Shared |
//      +---------------+-------+---------------+
//      | User   Shared |       | User   Shared |
//      +---------------+-------+---------------+
//      | Sub Processor |       | Sub Processor |
//      | Reserved      |       | Reserved      |
//      +-+-------------+-------+---------------+--7.875M
//      |W|             |       |               |
//      |R|Main         |       | Debugger Work |
//      |A|Processor    |       |               |
//      |P|Reserved     |       +---------------+--7.000M
//      | |             |       |               |
//      |A|             |       | Main          |
//      |R|             |       | Processor     |
//      |O+-------------+--4M---+ Extra         +--4.000M
//      |U|System Shared|       | Reserved      |
//      |N+-------------+       |               |
//      |D|User   Shared|       |               |
//      | +-------------+       +               +
//      | |Sub Processor|       |               |
//      | |Reserved     |       |               |
//      +-+-------------+-------+---------------+--3.875M
//      |               |       |               |
//      | Main          |       | Main          |
//      | Processor     |       | Processor     |
//      | Reserved      |       | Reserved      |
//      |               |       |               |
//      |               |       |               |
//      |               |       |               |
//      +---------------+       +---------------+
//
//      Default setting for 4MB
//         MAIN  :3.875MB
//         SUB   :124KB
//         SHARED:  4KB
//         DTCM  :on top of SUB
//

#define HW_MAIN_MEM_SHARED_SIZE         0x001000

// HW_MAIN_MEM_MAIN_SIZE is able to be changed, but need to arrange ARM7 module.
// If you use the Mongoose as ARM7 module, maybe 0x3e0000 is almost maximum size.
#ifdef SDK_TS
#define HW_MAIN_MEM_MAIN_SIZE           0x3E0000
#else
#define HW_MAIN_MEM_MAIN_SIZE           0x380000
#endif
#define HW_MAIN_MEM_SUB_SIZE            (HW_MAIN_MEM_SIZE - HW_MAIN_MEM_MAIN_SIZE - HW_MAIN_MEM_SHARED_SIZE)

#define HW_MAIN_MEM_MAIN                (HW_MAIN_MEM)
#define HW_MAIN_MEM_MAIN_END            (HW_MAIN_MEM + HW_MAIN_MEM_MAIN_SIZE)
#define HW_MAIN_MEM_SUB                 (HW_MAIN_MEM_MAIN_END + 0x400000)
#define HW_MAIN_MEM_SUB_END             (HW_MAIN_MEM_SUB + HW_MAIN_MEM_SUB_SIZE)

#define HW_MAIN_MEM_DEBUGGER_OFFSET     0x700000
#define HW_MAIN_MEM_DEBUGGER_SIZE       0x080000
#define HW_MAIN_MEM_DEBUGGER            (HW_MAIN_MEM + HW_MAIN_MEM_DEBUGGER_OFFSET)
#define HW_MAIN_MEM_DEBUGGER_END        (HW_MAIN_MEM_DEBUGGER + HW_MAIN_MEM_DEBUGGER_SIZE)

#define HW_MAIN_MEM_SHARED              (HW_MAIN_MEM_EX_END - HW_MAIN_MEM_SHARED_SIZE)
#define HW_MAIN_MEM_SHARED_END          (HW_MAIN_MEM_EX_END - HW_MAIN_MEM_SYSTEM_SIZE)

#define HW_MAIN_MEM_IM_SHARED_SIZE      (HW_MAIN_MEM_SHARED_SIZE)
#define HW_MAIN_MEM_IM_SHARED           (HW_MAIN_MEM_IM_SHARED_END - HW_MAIN_MEM_IM_SHARED_SIZE)
#define HW_MAIN_MEM_IM_SHARED_END       (HW_MAIN_MEM + 0x00800000)

#define HW_MAIN_MEM_SYSTEM_END          (HW_MAIN_MEM_EX_END)

#ifdef __cplusplus
} /* extern "C" */
#endif
/* NITRO_HW_MMAP_MAIN_H_ */
#endif
