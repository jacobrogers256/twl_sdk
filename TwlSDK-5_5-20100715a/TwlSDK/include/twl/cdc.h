/*---------------------------------------------------------------------------*
  Project:  TwlSDK - cdc - include
  File:     cdc.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CDC_H_
#define TWL_CDC_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

#ifdef SDK_ARM7

#include <twl/cdc/ARM7/cdc_reg.h>
#include <twl/cdc/ARM7/cdc_api.h>
#include <twl/cdc/ARM7/cdc_twlmode_access.h>
#include <twl/cdc/ARM7/cdc_dsmode_access.h>

#endif

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_CDC_H_ */
