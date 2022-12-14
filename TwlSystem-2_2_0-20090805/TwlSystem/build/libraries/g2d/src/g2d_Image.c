/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_Image.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nnsys/g2d/g2d_Image.h>


static void InitializeVRamLocation_ ( NNSG2dVRamLocation* pVramLocation );
static void SetVramLocation_        ( NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type, u32 addr );
static u32  GetVramLocation_        ( const NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type );
static BOOL IsImageReadyToUse_      ( const NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type );



//------------------------------------------------------------------------------
// You have to call this method before using a new NNSG2dVRamLocation instance.
// Otherwise, assertion failure will happen in many methods in the module. 
static NNS_G2D_INLINE void InitializeVRamLocation_( NNSG2dVRamLocation* pVramLocation )
{
    int i;
    for( i = 0; i < NNS_G2D_VRAM_TYPE_MAX; i++ )
    {
        pVramLocation->baseAddrOfVram[ i ] = NNS_G2D_VRAM_ADDR_NOT_INITIALIZED;
    }
} 

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void SetVramLocation_( NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type, u32 addr )
{
    NNS_G2D_NULL_ASSERT( pVramLocation );
    NNS_G2D_VRAM_TYPE_VALID( type );

    if( IsImageReadyToUse_( pVramLocation, type ) )
    {
        OS_Warning("Invalid setting is detected.\n Make sure to initialize NNSG2dVRamLocation instance before using it.");
    }
        
    pVramLocation->baseAddrOfVram[ type ] = addr;
}
//------------------------------------------------------------------------------
static NNS_G2D_INLINE u32 GetVramLocation_( const NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type )
{
    NNS_G2D_NULL_ASSERT( pVramLocation );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    return pVramLocation->baseAddrOfVram[ type ];
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE BOOL IsImageReadyToUse_( const NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type )
{
    NNS_G2D_NULL_ASSERT( pVramLocation );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    return ( GetVramLocation_( pVramLocation, type ) != NNS_G2D_VRAM_ADDR_NOT_INITIALIZED ) ?
        TRUE : FALSE;
}





//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static NNS_G2D_INLINE int          GetPow_( u16 num )
{
    switch( num )
    {
    case 1:return GX_TEXSIZE_S8;
    case 2:return GX_TEXSIZE_S16;
    case 4:return GX_TEXSIZE_S32;
    case 8:return GX_TEXSIZE_S64;
    case 16:return GX_TEXSIZE_S128;
    case 32:return GX_TEXSIZE_S256;
    default:
        OS_Warning("unexpected img size !");
        return GX_TEXSIZE_S8;
    }
}


//------------------------------------------------------------------------------
static NNS_G2D_INLINE void CopyCharDataToImageAttr_
( 
    const NNSG2dCharacterData*  pSrc, 
    NNSG2dImageAttr*            pDst 
)
{
    NNS_G2D_NULL_ASSERT( pSrc );
    NNS_G2D_NULL_ASSERT( pDst );
    //
    // TODO: Set the first time only.
    // If this is not the first time, check to see if the value is the same.
    // Initialize with some standard value so a determination can be made whether this is the first time.
    //
    if( pSrc->mapingType == GX_OBJVRAMMODE_CHAR_2D )
    {
        pDst->sizeS         = (GXTexSizeS)(GetPow_( pSrc->W ));
        pDst->sizeT         = (GXTexSizeT)(GetPow_( pSrc->H ));
    }else{
        // This information has no meaning in 1D mapping mode.
        // To prevent incorrect use, illegal data (NNS_G2D_1D_MAPPING_CHAR_SIZE) is inserted by the converter to pSrc->W H.
        // 
        NNS_G2D_ASSERT( (pSrc->W == NNS_G2D_1D_MAPPING_CHAR_SIZE) && 
                    (pSrc->H == NNS_G2D_1D_MAPPING_CHAR_SIZE) );
        
        pDst->sizeS         = (GXTexSizeS)pSrc->W;
        pDst->sizeT         = (GXTexSizeT)pSrc->H;
    }
    
    pDst->fmt           = pSrc->pixelFmt;
    pDst->bExtendedPlt  = FALSE;                
    pDst->plttUse       = GX_TEXPLTTCOLOR0_TRNS;
    pDst->mappingType   = pSrc->mapingType;
}

//------------------------------------------------------------------------------
// Performs actual load of character data to VRAM.
static NNS_G2D_INLINE void DoLoadingToVram_
( 
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr,
    NNS_G2D_VRAM_TYPE           type
)
{
    const NNSG2dCharacterFmt charFmt = NNSi_G2dGetCharacterFmtType( pSrcData->characterFmt );
    
    //
    // The VRAM transfer manager performs the transfer, so don't need to load on own.
    //
    NNS_G2D_ASSERTMSG( !NNSi_G2dIsCharacterVramTransfered( pSrcData->characterFmt ),
        "You don't have to transfer a image data to VRAM. When you use the VRAM Transfer format image." );

    //
    // Runs an actual load process that matches the type of VRAM that is specified.
    // Will fail with assert if the format of the data to be loaded is not appropriate.
    //
    DC_FlushRange( pSrcData->pRawData, pSrcData->szByte );
    switch( type )
    {
    case NNS_G2D_VRAM_TYPE_3DMAIN:
        NNS_G2D_ASSERT( charFmt == NNS_G2D_CHARACTER_FMT_BMP );
        GX_BeginLoadTex();
            GX_LoadTex( (void*)pSrcData->pRawData, baseAddr, pSrcData->szByte );
        GX_EndLoadTex();    
        break;
    case NNS_G2D_VRAM_TYPE_2DMAIN:
        NNS_G2D_ASSERT( charFmt == NNS_G2D_CHARACTER_FMT_CHAR );
        GX_LoadOBJ( (void*)pSrcData->pRawData, baseAddr, pSrcData->szByte );
        break;
    case NNS_G2D_VRAM_TYPE_2DSUB:
        NNS_G2D_ASSERT( charFmt == NNS_G2D_CHARACTER_FMT_CHAR );
        GXS_LoadOBJ( (void*)pSrcData->pRawData, baseAddr, pSrcData->szByte );
        break;
    default:
        NNS_G2D_ASSERT( FALSE );


    }
}

//------------------------------------------------------------------------------
// Determine whether the 1D mapping mode type is valid.
static BOOL IsValid1DMappingType_( NNS_G2D_VRAM_TYPE vramType, GXOBJVRamModeChar mappingType )
{
    switch( vramType )
    {
    case NNS_G2D_VRAM_TYPE_3DMAIN:
    case NNS_G2D_VRAM_TYPE_2DMAIN:
       return (BOOL)( GX_OBJVRAMMODE_CHAR_1D_32K <= mappingType && 
                      mappingType <= GX_OBJVRAMMODE_CHAR_1D_256K );
       break;
    case NNS_G2D_VRAM_TYPE_2DSUB:
       //
       // Note that GX_OBJVRAMMODE_CHAR_1D_256K cannot be used.
       //
       return (BOOL)( GX_OBJVRAMMODE_CHAR_1D_32K <= mappingType && 
                      mappingType <= GX_OBJVRAMMODE_CHAR_1D_128K );
       break;
    default:
       return FALSE;
    }
}

//------------------------------------------------------------------------------
// Determine whether entered source data is appropriate in size.
static BOOL IsValidDataSize_
( 
    const NNSG2dCharacterData*  pSrcData, 
    NNS_G2D_VRAM_TYPE           vramType 
)
{
    if( vramType == NNS_G2D_VRAM_TYPE_3DMAIN )
    {
       // No special size check is done for 3D texture data.
       return TRUE;
    }else{
       switch( pSrcData->mapingType )
       {
       case GX_OBJVRAMMODE_CHAR_2D:
           return (BOOL)( pSrcData->szByte <= 32 * 1024);
       case GX_OBJVRAMMODE_CHAR_1D_32K:
           return (BOOL)( pSrcData->szByte <= 32 * 1024);
       case GX_OBJVRAMMODE_CHAR_1D_64K:
           return (BOOL)( pSrcData->szByte <= 64 * 1024);
       case GX_OBJVRAMMODE_CHAR_1D_128K:
           return (BOOL)( pSrcData->szByte <= 128 * 1024);
       case GX_OBJVRAMMODE_CHAR_1D_256K:
           return (BOOL)( pSrcData->szByte <= 256 * 1024);
       default:
           return FALSE;
       }
    }
}

//------------------------------------------------------------------------------
// Uses GX functions to make register settings related to character mapping mode.
static NNS_G2D_INLINE void SetOBJVRamModeCharacterMapping_
( 
    NNS_G2D_VRAM_TYPE   vramType, 
    GXOBJVRamModeChar   vramMode 
)
{
    GX_OBJVRAMMODE_CHAR_ASSERT( vramMode );
    
    switch( vramType )
    {
    case NNS_G2D_VRAM_TYPE_3DMAIN:
        // Do nothing.
        break;
    case NNS_G2D_VRAM_TYPE_2DMAIN:
        GX_SetOBJVRamModeChar(vramMode);
        break;
    case NNS_G2D_VRAM_TYPE_2DSUB:
        GXS_SetOBJVRamModeChar(vramMode);
        break;   
    default:
        NNS_G2D_ASSERT( FALSE );
    }
}

//------------------------------------------------------------------------------
// Configures image proxy parameters.
static NNS_G2D_INLINE void SetupImageProxyPrams_
(
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr, 
    NNS_G2D_VRAM_TYPE           type, 
    NNSG2dImageProxy*           pImgProxy
)
{
    // parameter type copy
    // from NNSG2dCharacterData to NNSG2dImageProxy
    CopyCharDataToImageAttr_( pSrcData, &pImgProxy->attr );
    // VRAM location (address) setting
    NNS_G2dSetImageLocation( pImgProxy, type, baseAddr );
}


//------------------------------------------------------------------------------
// Transfers to actual palette data VRAM.
// The function used to transfer will differ according to the type of VRAM.
//
static NNS_G2D_INLINE void DoLoadingPalette_
(
    const NNSG2dPaletteData*    pSrcData,
    u32                         srcOffset,
    u32                         addr,
    u32                         szByte,
    NNS_G2D_VRAM_TYPE           type
)
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    {
       const void*   pSrcRawData =  (const void*)( (const u8*)pSrcData->pRawData + srcOffset);
       
       DC_FlushRange( pSrcData->pRawData, pSrcData->szByte );
       
       switch( type )
       {
       case NNS_G2D_VRAM_TYPE_2DMAIN:
           // 2D
           if( pSrcData->bExtendedPlt )
           {
              GX_BeginLoadOBJExtPltt();
                  GX_LoadOBJExtPltt( pSrcRawData, addr, szByte );
              GX_EndLoadOBJExtPltt();                      
           }else{
              GX_LoadOBJPltt( pSrcRawData, addr, szByte );
           }
           break;
       case NNS_G2D_VRAM_TYPE_2DSUB:
           // 2D
           if( pSrcData->bExtendedPlt )
           {
              GXS_BeginLoadOBJExtPltt();
                  GXS_LoadOBJExtPltt( pSrcRawData, addr, szByte );
              GXS_EndLoadOBJExtPltt();
           }else{
              GXS_LoadOBJPltt( pSrcRawData, addr, szByte );
           }
           break;
       case NNS_G2D_VRAM_TYPE_3DMAIN:        
           // 3D
           GX_BeginLoadTexPltt();
              GX_LoadTexPltt( pSrcRawData, addr, szByte );
           GX_EndLoadTexPltt();
           break;
       default:
           NNS_G2D_ASSERT(FALSE);// TODO: msg
           break;
       }
    }
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE u32 CalcSizePerOnePltt_( const NNSG2dPaletteData*    pSrcData )
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    
    if( pSrcData->fmt == GX_TEXFMT_PLTT16 )
    {
        return 16 * sizeof( u16 );
        
    }else{
        NNS_G2D_ASSERT( pSrcData->fmt == GX_TEXFMT_PLTT256 );// only two types should exist
        return 256 * sizeof( u16 );
    }
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE u16 GetCompressedPlttOriginalIndex_( const NNSG2dPaletteCompressInfo* pCmpInfo, u16 idx )
{
    NNS_G2D_NULL_ASSERT( pCmpInfo );
    
    return *(((u16*)pCmpInfo->pPlttIdxTbl) + idx );
}

//------------------------------------------------------------------------------
// Loads data stored in compressed format to VRAM.
static NNS_G2D_INLINE void DoLoadingPaletteEx_
(
    const NNSG2dPaletteData*            pSrcData,
    const NNSG2dPaletteCompressInfo*    pCmpInfo,
    u32                                 addr,
    NNS_G2D_VRAM_TYPE                   type
)
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    NNS_G2D_NULL_ASSERT( pCmpInfo );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    //
    // Splits it up and sends in chunks.
    //
    {
        u16             i;
        u32             offsetAddr;
        const u32       szOnePltt   = CalcSizePerOnePltt_( pSrcData );
        const u16       numIdx      = pCmpInfo->numPalette;
    
        
        for( i = 0; i < numIdx; i++ )
        {
            offsetAddr  = GetCompressedPlttOriginalIndex_( pCmpInfo, i ) * szOnePltt;
            
            DoLoadingPalette_( pSrcData, szOnePltt * i, addr + offsetAddr, szOnePltt, type );
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitImageProxy

  Description:  Initializes NNSG2dImageProxy.
                
                
  Arguments:    pImg:         [OUT] instance of NNSG2dImageProxy
               
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitImageProxy( NNSG2dImageProxy* pImg )
{
    NNS_G2D_NULL_ASSERT( pImg );
    InitializeVRamLocation_( &pImg->vramLocation );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetImageLocation

  Description:  Sets the real address where image is stored.
                
                
  Arguments:    pImg:         [OUT] instance of NNSG2dImageProxy
                type:         [IN]  VRAM type
                addr:         [IN]  actual address
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetImageLocation( NNSG2dImageProxy* pImg, NNS_G2D_VRAM_TYPE type, u32 addr )
{
    NNS_G2D_NULL_ASSERT( pImg );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    SetVramLocation_( &pImg->vramLocation, type, addr );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetImageLocation

  Description:  Gets the real address where image is stored.
                
                
  Arguments:    pImg:         [OUT] instance of NNSG2dImageProxy
                type:         [IN]  VRAM type
                
  Returns:      actual address
  
 *---------------------------------------------------------------------------*/
u32 NNS_G2dGetImageLocation( const NNSG2dImageProxy* pImg, NNS_G2D_VRAM_TYPE type )
{
    NNS_G2D_NULL_ASSERT( pImg );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    return GetVramLocation_( &pImg->vramLocation, type );
}
/*---------------------------------------------------------------------------*
  Name:         NNS_G2dIsImageReadyToUse

  Description:  Determines whether an image is available.
                
                
  Arguments:    pImg:         [OUT] instance of NNSG2dImageProxy
                type:         [IN]  VRAM type
                
  Returns:      whether available
  
 *---------------------------------------------------------------------------*/
BOOL NNS_G2dIsImageReadyToUse( const NNSG2dImageProxy* pImg, NNS_G2D_VRAM_TYPE type )
{
    NNS_G2D_NULL_ASSERT( pImg );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    return IsImageReadyToUse_( &pImg->vramLocation, type );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitImagePaletteProxy

  Description:  Initializes the image palette.
                
  Arguments:    pImg:         [OUT] instance of NNSG2dImagePaletteProxy
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitImagePaletteProxy( NNSG2dImagePaletteProxy* pImg )
{
    NNS_G2D_NULL_ASSERT( pImg );
    InitializeVRamLocation_( &pImg->vramLocation );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetImagePaletteLocation

  Description:  Gets the real address where image palette is stored.
                
                
  Arguments:    pImg:         [OUT] instance of NNSG2dImagePaletteProxy
                type:         [IN]  VRAM type
                addr:         [IN]  actual address
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetImagePaletteLocation( NNSG2dImagePaletteProxy* pImg, NNS_G2D_VRAM_TYPE type, u32 addr )
{
    NNS_G2D_NULL_ASSERT( pImg );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    SetVramLocation_( &pImg->vramLocation, type, addr );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetImagePaletteLocation

  Description:  Gets the real address where image palette is stored.
                
                
  Arguments:    pImg:         [OUT] instance of NNSG2dImagePaletteProxy
                type:         [IN]  VRAM type
                
  Returns:      actual address
  
 *---------------------------------------------------------------------------*/
u32 NNS_G2dGetImagePaletteLocation( const NNSG2dImagePaletteProxy* pImg, NNS_G2D_VRAM_TYPE type )
{
    NNS_G2D_NULL_ASSERT( pImg );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    return GetVramLocation_( &pImg->vramLocation, type );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dIsImagePaletteReadyToUse

  Description:  Determines whether an image palette is available.
                
  Arguments:    pImg:         [OUT] instance of NNSG2dImagePaletteProxy
                type:         [IN]  VRAM type
                
  Returns:      whether available
  
 *---------------------------------------------------------------------------*/
BOOL NNS_G2dIsImagePaletteReadyToUse( const NNSG2dImagePaletteProxy* pImg, NNS_G2D_VRAM_TYPE type )
{
    NNS_G2D_NULL_ASSERT( pImg );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    return IsImageReadyToUse_( &pImg->vramLocation, type );
}







/*---------------------------------------------------------------------------*
  Name:         NNS_G2dLoadImage1DMapping

  Description:  Loads image data to the hardware and configures the load results to the image proxy, and the image format.
                
                This is a special function for image elements when mode is 1D mapping.
                When VRAM for the 2D graphics engine is specified, the 2D graphics engine's character mode is internally set to 1D mapping mode.
                

                
  Arguments:    pSrcData        [IN]  input image data source 
                baseAddr        [IN]  target address for load 
                type            [IN]  type of VRAM used (3D, 2D-Main, 2D-Sub) 
                pImgProxy       [OUT] image proxy storing the load result 

  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dLoadImage1DMapping
( 
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr, 
    NNS_G2D_VRAM_TYPE           type, 
    NNSG2dImageProxy*           pImgProxy 
)
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    NNS_G2D_NULL_ASSERT( pImgProxy );
    NNS_G2D_ASSERTMSG( IsValid1DMappingType_( type, pSrcData->mapingType ), 
                       "Invalid mapping-type.");
    NNS_G2D_ASSERTMSG( IsValidDataSize_( pSrcData, type ), 
                       "Invalid data size for the mapping-type.");
    
                       
                       
    
    SetOBJVRamModeCharacterMapping_( type, pSrcData->mapingType );
    
    DoLoadingToVram_( pSrcData, baseAddr, type );
    
    SetupImageProxyPrams_( pSrcData, baseAddr, type, pImgProxy );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dLoadImage2DMapping

  Description:  Loads image data to the hardware and configures the load results to the image proxy, and the image format.
                
                
                This is a special function for 2D mapping mode image elements.
                When VRAM for the 2D graphics engine is specified, the 2D graphics engine's character mode is internally set to 2D mapping mode.
                

                
  Arguments:    pSrcData        [IN]  input image data source 
                baseAddr        [IN]  target address for load 
                type            [IN]  type of VRAM used (3D, 2D-Main, 2D-Sub) 
                pImgProxy       [OUT] image proxy storing the load result 

  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dLoadImage2DMapping
( 
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr, 
    NNS_G2D_VRAM_TYPE           type, 
    NNSG2dImageProxy*           pImgProxy 
)
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    NNS_G2D_NULL_ASSERT( pImgProxy );    
    NNS_G2D_ASSERT( pSrcData->mapingType == GX_OBJVRAMMODE_CHAR_2D );
    NNS_G2D_ASSERTMSG( IsValidDataSize_( pSrcData, type ), 
                       "Invalid data size for the mapping-type.");
    

    SetOBJVRamModeCharacterMapping_( type, pSrcData->mapingType );

    
    DoLoadingToVram_( pSrcData, baseAddr, type );
    
    SetupImageProxyPrams_( pSrcData, baseAddr, type, pImgProxy );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dLoadImageVramTransfer

  Description:  Sets the load result and image format in the image proxy.
                This is a special function for image elements for VRAM transfers.
                This function differs from the other NNS_G2dLoadImageXXX() functions in that the image data is not actually loaded to the hardware.
                The actual load takes place during the game loop, which is performed by the VRAM transfer module.
                When VRAM for the 2D Graphics Engine is specified, the 2D graphics engine's character mode is internally set to 1D mapping mode.
                
                (For NCGR and NCBR files, the image elements for VRAM transfers are always stored in 1D mapping mode.) 
                
  Arguments:    pSrcData        [IN]  input image data source 
                baseAddr        [IN]  target address for load 
                type            [IN]  type of VRAM used (3D, 2D-Main, 2D-Sub) 
                pImgProxy       [OUT] image proxy storing the load result 

  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dLoadImageVramTransfer
( 
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr, 
    NNS_G2D_VRAM_TYPE           type, 
    NNSG2dImageProxy*           pImgProxy 
)
{   
    NNS_G2D_MINMAX_ASSERT( pSrcData->mapingType, 
                       GX_OBJVRAMMODE_CHAR_1D_32K, 
                       GX_OBJVRAMMODE_CHAR_1D_256K );
                       
    NNS_G2D_ASSERTMSG( NNSi_G2dIsCharacterVramTransfered( pSrcData->characterFmt ),
        "NNS_G2dSetVramTransferImageProxy() must be used only for the vram transfer format image data." );
    
    DC_FlushRange( pSrcData->pRawData, pSrcData->szByte );
    
    SetOBJVRamModeCharacterMapping_( type, pSrcData->mapingType );
    
    CopyCharDataToImageAttr_( pSrcData, &pImgProxy->attr );
    
    NNS_G2dSetImageLocation( pImgProxy, type, baseAddr );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dLoadPalette

  Description:  Loads image palette data to the hardware and configures the load results to the image palette proxy, and the image format.
                

                
                
  Arguments:    pSrcData        [IN]  input image data source 
                addr            [IN]  target address for load 
                type            [IN]  type of VRAM used (3D, 2D-Main, 2D-Sub) 
                pPltProxt       [OUT] image palette proxy storing the load result 

  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dLoadPalette
(
    const NNSG2dPaletteData*    pSrcData,
    u32                         addr,
    NNS_G2D_VRAM_TYPE           type, 
    NNSG2dImagePaletteProxy*    pPltProxy
)
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    NNS_G2D_VRAM_TYPE_VALID( type );
    NNS_G2D_NULL_ASSERT( pPltProxy );
    
    DoLoadingPalette_( pSrcData, 0, addr, pSrcData->szByte, type );                                      
    
    pPltProxy->fmt          = pSrcData->fmt;          
    pPltProxy->bExtendedPlt = pSrcData->bExtendedPlt;
    
    NNS_G2dSetImagePaletteLocation( pPltProxy, type, addr );
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dLoadPaletteEx

  Description:  Loads image palette data to the hardware and configures the load results to the image palette proxy, and the image format.
                

                
                
  Arguments:    pSrcData        [IN]  input image data source 
                pCmpInfo        [IN]  compression information
                addr            [IN]  target address for load 
                type            [IN]  type of VRAM used (3D, 2D-Main, 2D-Sub) 
                pPltProxy       [OUT] image palette proxy storing the load result 

  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dLoadPaletteEx
(
    const NNSG2dPaletteData*            pSrcData,
    const NNSG2dPaletteCompressInfo*    pCmpInfo,
    u32                                 addr,
    NNS_G2D_VRAM_TYPE                   type, 
    NNSG2dImagePaletteProxy*            pPltProxy
)
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    NNS_G2D_NULL_ASSERT( pCmpInfo );
    NNS_G2D_VRAM_TYPE_VALID( type );
    NNS_G2D_NULL_ASSERT( pPltProxy );
    
    //
    // Splits it up and loads in chunks.
    //
    DoLoadingPaletteEx_( pSrcData, pCmpInfo, addr, type );
    
    // 
    // The other settings are not different from normal loads.
    // Be extra careful that the image proxy settings are the same.
    //
    pPltProxy->fmt          = pSrcData->fmt;          
    pPltProxy->bExtendedPlt = pSrcData->bExtendedPlt;
    
    NNS_G2dSetImagePaletteLocation( pPltProxy, type, addr );
}


//------------------------------------------------------------------------------
// wrapper group for disclosing private functions to external parties
// internally public in the library
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void NNSi_G2dInitializeVRamLocation( NNSG2dVRamLocation* pVramLocation )
{
    InitializeVRamLocation_( pVramLocation );
} 

//------------------------------------------------------------------------------
void NNSi_G2dSetVramLocation( NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type, u32 addr )
{
    NNS_G2D_NULL_ASSERT( pVramLocation );
    NNS_G2D_VRAM_TYPE_VALID( type );

    SetVramLocation_( pVramLocation, type, addr );
}
//------------------------------------------------------------------------------
u32 NNSi_G2dGetVramLocation( const NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type )
{
    NNS_G2D_NULL_ASSERT( pVramLocation );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    return GetVramLocation_( pVramLocation, type );
}

//------------------------------------------------------------------------------
BOOL NNSi_G2dIsVramLocationReadyToUse( const NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type )
{
    NNS_G2D_NULL_ASSERT( pVramLocation );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    return IsImageReadyToUse_( pVramLocation, type );
}

//------------------------------------------------------------------------------
// Runs the process that actually loads image data to VRAM.
void NNSi_G2dDoImageLoadingToVram
( 
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr,
    NNS_G2D_VRAM_TYPE           type
)
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    DoLoadingToVram_( pSrcData, baseAddr, type );
}


//------------------------------------------------------------------------------
// Configures image proxy parameters.
void NNSi_G2dSetupImageProxyPrams
(
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr, 
    NNS_G2D_VRAM_TYPE           type, 
    NNSG2dImageProxy*           pImgProxy
)
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    NNS_G2D_VRAM_TYPE_VALID( type );
    NNS_G2D_NULL_ASSERT( pImgProxy );
    
    SetupImageProxyPrams_( pSrcData, baseAddr, type, pImgProxy );
}

//------------------------------------------------------------------------------
// Runs the process that actually loads palette data to VRAM.
void NNSi_G2dDoLoadingPaletteToVram
(
    const NNSG2dPaletteData*    pSrcData,
    u32                         addr,
    NNS_G2D_VRAM_TYPE           type
)
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    
    DoLoadingPalette_( pSrcData, 0, addr, pSrcData->szByte, type );
}

//------------------------------------------------------------------------------
// Runs the process that actually loads palette data to VRAM.
// This function is used for palette data that has been stored in a compressed format.
void NNSi_G2dDoLoadingPaletteToVramEx
(
    const NNSG2dPaletteData*            pSrcData,
    const NNSG2dPaletteCompressInfo*    pCmpInfo,
    u32                                 addr,
    NNS_G2D_VRAM_TYPE                   type
)
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    NNS_G2D_NULL_ASSERT( pCmpInfo );
    NNS_G2D_VRAM_TYPE_VALID( type );
    
    
    DoLoadingPaletteEx_( pSrcData, pCmpInfo, addr, type );
}

//------------------------------------------------------------------------------
// Configures palette proxy parameter settings.
void NNSi_G2dSetupPaletteProxyPrams
(
    const NNSG2dPaletteData*            pSrcData,
    u32                                 addr,
    NNS_G2D_VRAM_TYPE                   type, 
    NNSG2dImagePaletteProxy*            pPltProxy
)
{
    NNS_G2D_NULL_ASSERT( pSrcData );
    NNS_G2D_VRAM_TYPE_VALID( type );
    NNS_G2D_NULL_ASSERT( pPltProxy );
    
    pPltProxy->fmt          = pSrcData->fmt;          
    pPltProxy->bExtendedPlt = pSrcData->bExtendedPlt;
    
    NNS_G2dSetImagePaletteLocation( pPltProxy, type, addr );
}

