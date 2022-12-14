/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     cache.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-04#$
  $Rev: 9197 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/
#ifndef	NITRO_MI_CACHE_H_
#define	NITRO_MI_CACHE_H_


#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/platform.h>
#include <nitro/mi/device.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

/* The various pages' information that comprise the memory cache. */
typedef struct MICachePage
{
    struct MICachePage *next;   /* The next element (used for the list) */
    u32                 offset; /* The page header location (in page size units) */
    u8                 *buffer; /* The page buffer */
}
MICachePage;

/* Memory cache information structure */
typedef struct MICache
{
    u32             pagewidth;
    MICachePage    *valid;
    MICachePage    *invalid;
    MICachePage    *loading;
    int             valid_total;
    int             invalid_total;
    int             loading_total;
}
MICache;

/* Macro to calculate the needed buffer size to prepare the specified page size and number of pages */
#define MI_CACHE_BUFFER_WORKSIZE(page, total) \
        ((sizeof(MICachePage) + (page)) * (total))


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         MI_InitCache

  Description:  Initializes the memory cache.

  Arguments:    cache            The MICache structure to be initialized.
                page             The buffer size per page.
                                 Must be a power of 2 if four or more pages.
                buffer           The buffer used for page management information.
                length           The buffer size.
                                 This is divided into a number of page lists calculated as follows: length / (sizeof(MICachePage) + page)
                                 
                                 It is guaranteed that the starting address in the buffer will be (buffer + N * page) for each page (N = 0, 1,...).
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MI_InitCache(MICache *cache, u32 page, void *buffer, u32 length);

/*---------------------------------------------------------------------------*
  Name:         MI_ReadCache

  Description:  Reads data from a cache.
                Pages that are hit will be moved to the start of the valid list.
                Pages that aren't hit are added to the load request list.

  Arguments:    cache            The MICache structure.
                buffer           The transfer target memory.
                                 If NULL is specified, simply request that the entire corresponding cache range be prepared without loading any data.
                                 
                offset           The transfer source offset.
                length           The transfer size.

  Returns:      TRUE if the entire region is hit by the cache.
 *---------------------------------------------------------------------------*/
BOOL    MI_ReadCache(MICache *cache, void *buffer, u32 offset, u32 length);

/*---------------------------------------------------------------------------*
  Name:         MI_LoadCache

  Description:  Runs the load process for all pages that exist in the load request list.
                When the load request list is empty, control will be returned immediately without any processing. If a load request is added to the list during the function call, it will also be processed.
                

  Note:         Appropriate timing is required when calling this function from a context that allows devices to block.
                
                Therefore, it must not be called from an interrupt handler, etc.

  Arguments:    cache            The MICache structure.
                device           The device to be loaded.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MI_LoadCache(MICache *cache, MIDevice *device);

/*---------------------------------------------------------------------------*
  Name:         MI_IsCacheLoading

  Description:  Determines whether the load request list is empty.
                Load requests are added to the list by calls to the MI_ReadCache function and emptied by calls to the MI_LoadCache function.
                

  Arguments:    cache            The MICache structure.

  Returns:      TRUE if the load request list is not empty.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
BOOL    MI_IsCacheLoading(const MICache *cache)
{
    return (cache->loading != NULL);
}


/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_MI_CACHE_H_ */
