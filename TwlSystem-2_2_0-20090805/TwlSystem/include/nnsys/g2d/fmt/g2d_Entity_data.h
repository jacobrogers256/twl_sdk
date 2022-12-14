/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d - fmt
  File:     g2d_Entity_data.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_G2D_ENTITY_DATA_H_
#define NNS_G2D_ENTITY_DATA_H_

#include <nitro/types.h>
#include <nnsys/g2d/fmt/g2d_Common_data.h>
#include <nnsys/g2d/fmt/g2d_Anim_data.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NNS_G2D_BINFILE_SIG_ENTITY        (u32)'NENR'
#define NNS_G2D_BLKSIG_ENTITYBANK         (u32)'ENBK'

#define NNS_G2D_BINFILE_EXT_ENTITY          "NENR"

//
// Version information
// Ver         Changes
// -------------------------------------
// 1.0         Initial version
//
#define NNS_G2D_NENR_MAJOR_VER              1
#define NNS_G2D_NENR_MINOR_VER              0


//------------------------------------------------------------------------------
// Entity Type
typedef enum NNSG2dEntityType
{
    NNS_G2D_ENTITYTYPE_INVALID      = 0,
    NNS_G2D_ENTITYTYPE_CELL         ,
    NNS_G2D_ENTITYTYPE_MULTICELL    ,
    NNS_G2D_ENTITYTYPE_MAX          

}NNSG2dEntityType;

//------------------------------------------------------------------------------
// Animation Data Array for Entity
typedef struct NNSG2dEntityAnimData
{
    u16         numAnimSequence;
    u16         pad16;
    u16*        pAnimSequenceIdxArray;      // NNSG2dAnimSequence number array
    
}NNSG2dEntityAnimData;

//------------------------------------------------------------------------------
// Entity Settings Information
typedef struct NNSG2dEntityData
{
    
    NNSG2dEntityAnimData          animData;      // Animation Data Array for Entity
    NNSG2dEntityType              type;          // Rendering instance type
    
    // const NNSG2dAnimBankData*     pAnimDataBank; // related AnimBank (set during unpack at runtime)
    // 
    // This information is maintained by the instance.
    // 
    // This change is not the best selection for NNSG2dEntityData.
    // (There are few NNSG2dEntityData, but many instances that share it.)
    // This change was made for consistency with NNSG2dMultiCellDataBank.
    // There is not a major concern about the cost here.
    // 
    // The comment of NNSG2dMultiCellDataBank declaration should be referenced.
    // 
    
}NNSG2dEntityData;


//------------------------------------------------------------------------------
// Entity Settings Information Bank
typedef struct NNSG2dEntityDataBank
{
    u16                     numEntityDatas;
    u16                     pad16;
    NNSG2dEntityData*       pEntityDataArray;
    u16*                    pAnimSequenceIdxArray;
    void*                   pStringBank;
    void*                   pExtendedData;        // offset addr (if it exists)
    
}NNSG2dEntityDataBank;


typedef struct NNSG2dEntityDataBankBlock
{
    
    NNSG2dBinaryBlockHeader     blockHeader;
    NNSG2dEntityDataBank        entityDataBank;
    
}NNSG2dEntityDataBankBlock;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_G2D_ENTITY_DATA_H_

