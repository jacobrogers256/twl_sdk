/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d - src - load
  File:     g2d_NFT_load.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nnsys/g2d/g2d_Load.h>
#include <nnsys/g2d/load/g2d_NFT_load.h>
#include <nnsys/g2d/g2d_config.h>

//#define NNS_G2D_FONT_SHOW_CMAP_LIST



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetUnpackedFont

  Description:  Expands the NFTR file to a NITRO font.

  Arguments:    pNftrFile:  Pointer to the NFTR file data.
                ppFont:     Pointer to the buffer that stores the pointer to the NITRO font.
                            

  Returns:      Returns a value other than FALSE if the expansion was a success.
 *---------------------------------------------------------------------------*/
BOOL NNSi_G2dGetUnpackedFont( void* pNftrFile, NNSG2dFontInformation** ppRes )
{
#ifdef NNS_G2D_FONT_USE_OLD_RESOURCE
    BOOL isOld = FALSE;
#endif // NNS_G2D_FONT_USE_OLD_RESOURCE
    NNS_G2D_POINTER_ASSERT( pNftrFile );
    NNS_G2D_ALIGN_ASSERT( pNftrFile, 4 );
    NNS_G2D_POINTER_ASSERT( ppRes );
#ifdef NNS_G2D_FONT_USE_OLD_RESOURCE
    if( ! NNS_G2dIsBinFileValid(pNftrFile, NNS_G2D_BINFILE_SIG_FONTDATA, NNS_G2D_NFTR_VER) )
    {
        if( ! NNS_G2dIsBinFileValid(pNftrFile, NNS_G2D_BINFILE_SIG_FONTDATA, NNS_G2D_NFTR_VER_1_1) )
        {
            if( ! NNS_G2dIsBinFileValid(pNftrFile, NNS_G2D_BINFILE_SIG_FONTDATA, NNS_G2D_NFTR_VER_1_0) )
            {
                OS_Panic("Input file is invalid or obsolete. Please use latest fontcvtr.");
            }

            isOld = TRUE;
        }
    }
#else
    NNS_G2D_ASSERTMSG( NNS_G2dIsBinFileValid( pNftrFile, NNS_G2D_BINFILE_SIG_FONTDATA,
                                                     BIN_FILE_VERSION( NFTR ) ), "Input file is obsolete. Please use latest fontcvtr." );
#endif // NNS_G2D_FONT_USE_OLD_RESOURCE

    {
        NNSG2dBinaryFileHeader* pBinFile = (NNSG2dBinaryFileHeader*)pNftrFile;
        NNSG2dBinaryBlockHeader* pBinBlock;

        NNSi_G2dUnpackNFT( pBinFile );

        pBinBlock = NNS_G2dFindBinaryBlock(pBinFile, NNS_G2D_BINBLK_SIG_FINFDATA);

        if( pBinBlock == NULL )
        {
            *ppRes = NULL;
            return FALSE;
        }

        *ppRes = (NNSG2dFontInformation*)((u8*)pBinBlock + sizeof(*pBinBlock));
#ifdef NNS_G2D_FONT_USE_OLD_RESOURCE
        if( isOld )
        {
             // No support for vertical writing, vertical holding
            (*ppRes)->pGlyph->flags = 0;
        }
#endif
#ifndef NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT
        NNS_G2D_WARNING((*ppRes)->pGlyph->flags == 0, "Loading NFTR requires DIRECTION_SUPPORT, but it disabled.");
#endif
    }

    return TRUE;
}



/*---------------------------------------------------------------------------*
  Name:         ResolveOffset

  Description:  Resolves the offset pointer inside the NITRO font.

  Arguments:    ppOffset:   Pointer to the buffer whether the offset pointer is stored.
                            
                pBase:      Address that is the offset starting point

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void NNS_G2D_INLINE ResolveOffset(void** ppOffset, void* pBase)
{
    *ppOffset = (void*)(*(u32*)ppOffset + (u32)pBase);
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dUnpackNFT

  Description:  Expands the NFTR file data into a format that can be used by the library.
                This only needs to be executed once on each file data before that data is used in the library.
                

  Arguments:    pHeader:    Pointer to the binary file header of the NFTR file.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNSi_G2dUnpackNFT(NNSG2dBinaryFileHeader* pHeader)
{
    NNSG2dBinaryBlockHeader* pBlock;
    NNSG2dFontInformation* pInfo = NULL;

    NNS_G2D_POINTER_ASSERT( pHeader );

    {
        int nBlocks = 0;
        pBlock = (NNSG2dBinaryBlockHeader*)((u8*)pHeader + pHeader->headerSize);

        while( nBlocks < pHeader->dataBlocks )
        {
            switch( pBlock->kind )
            {
            //--------------------------------------------------
            // INFO block
            case NNS_G2D_BINBLK_SIG_FINFDATA:
                // There must be 1 INFO block
                NNS_G2D_ASSERT( pInfo == NULL );
                {
                    pInfo = (NNSG2dFontInformation*)((u8*)pBlock + sizeof(*pBlock));

                    NNS_G2D_ASSERT( pInfo->fontType == NNS_G2D_FONTTYPE_GLYPH );

                    // pGlyph is required
                    NNS_G2D_NULL_ASSERT( pInfo->pGlyph );
                    ResolveOffset((void**)&(pInfo->pGlyph), pHeader);
                    NNS_G2D_POINTER_ASSERT( pInfo->pGlyph )

                    // OK if there is no pWidth and pMap
                    if( pInfo->pWidth != NULL )
                    {
                        ResolveOffset((void**)&(pInfo->pWidth), pHeader);
                        NNS_G2D_POINTER_ASSERT( pInfo->pWidth )
                    }
                    if( pInfo->pMap != NULL )
                    {
                        ResolveOffset((void**)&(pInfo->pMap), pHeader);
                        NNS_G2D_POINTER_ASSERT( pInfo->pMap )
                    }
                }
                break;

            //--------------------------------------------------
            // CGLP block
            case NNS_G2D_BINBLK_SIG_CGLPDATA:
                // There must also be 1 CGLP block, but it will not be fatal if multiple exist.
                {
                    NNSG2dFontGlyph* pGlyph = (NNSG2dFontGlyph*)((u8*)pBlock + sizeof(*pBlock));
                    NNS_G2D_MIN_ASSERT( pGlyph->cellWidth, 1 );
                    NNS_G2D_MIN_ASSERT( pGlyph->cellHeight, 1 );
                    NNS_G2D_MIN_ASSERT( pGlyph->cellSize, 1 );
                    NNS_G2D_MINMAX_ASSERT( pGlyph->bpp, 1, 8 );
                }
                break;

            //--------------------------------------------------
            // CWDH block
            case NNS_G2D_BINBLK_SIG_CWDHDATA:
                {
                    NNSG2dFontWidth* pWidth = (NNSG2dFontWidth*)((u8*)pBlock + sizeof(*pBlock));

                    NNS_G2D_ASSERT( pWidth->indexBegin <= pWidth->indexEnd );

                    if( pWidth->pNext != NULL )
                    {
                        ResolveOffset((void**)&(pWidth->pNext), pHeader);
                        NNS_G2D_POINTER_ASSERT( pWidth->pNext )
                    }
                }
                break;

            //--------------------------------------------------
            // CMAP block
            case NNS_G2D_BINBLK_SIG_CMAPDATA:
                {
                    NNSG2dFontCodeMap* pMap = (NNSG2dFontCodeMap*)((u8*)pBlock + sizeof(*pBlock));

                    NNS_G2D_ASSERT( pMap->ccodeBegin <= pMap->ccodeEnd );
                    NNS_G2D_ASSERT( (pMap->mappingMethod == NNS_G2D_MAPMETHOD_DIRECT)
                                    || (pMap->mappingMethod == NNS_G2D_MAPMETHOD_TABLE)
                                    || (pMap->mappingMethod == NNS_G2D_MAPMETHOD_SCAN) );

                    if( pMap->pNext != NULL )
                    {
                        ResolveOffset((void**)&(pMap->pNext), pHeader);
                        NNS_G2D_POINTER_ASSERT( pMap->pNext )
                    }
                }
                break;

            //--------------------------------------------------
            // unknown
            default:
                NNS_G2D_ASSERTMSG(FALSE, "The font has unknown block.");
                break;
            }

            pBlock = (NNSG2dBinaryBlockHeader*)((u8*)pBlock + pBlock->size);
            nBlocks++;
        }
    }

    NNS_G2D_ASSERTMSG( pInfo != NULL,  "The font has no Information Block.");
    NNS_G2D_ASSERTMSG( pInfo->pGlyph != NULL,  "The font has no Glyph Block.");

    NNS_G2D_ASSERT( pInfo->alterCharIndex < NNS_G2D_FONT_MAX_GLYPH_INDEX((NNSG2dFont*)&pInfo) );
}




//------------------------------------------------------------------------------
#ifndef SDK_FINALROM

/*---------------------------------------------------------------------------*
  Name:         PrintFontInformation

  Description:  Performs debug print for the INFO block of the NITRO font.

  Arguments:    pInfo:  Pointer to INFO block

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintFontInformation(const NNSG2dFontInformation* pInfo)
{
    const static char* fontTypeString[] =
    {
        "NNS_G2D_FONTTYPE_GLYPH",
        "NNS_G2D_FONTTYPE_SHEET"
    };
    const static char* encodingString[] =
    {
        "NNS_G2D_FONT_ENCODING_UTF8",
        "NNS_G2D_FONT_ENCODING_UTF16",
        "NNS_G2D_FONT_ENCODING_SJIS",
        "NNS_G2D_FONT_ENCODING_CP1252",
    };

    NNS_G2D_POINTER_ASSERT( pInfo );
    NNS_G2D_MINMAX_ASSERT( pInfo->fontType, 0, NNS_G2D_NUM_OF_FONTTYPE - 1);
    NNS_G2D_MINMAX_ASSERT( pInfo->encoding, 0, NNS_G2D_NUM_OF_ENCODING - 1);

    OS_Printf(" NNSG2dFontInformation\n");
    OS_Printf(" fontType                = %s\n",     fontTypeString[pInfo->fontType]);
    OS_Printf(" linefeed                = %d px\n",  pInfo->linefeed);
    OS_Printf(" alterCharIndex          = 0x%04X\n", pInfo->alterCharIndex);
    OS_Printf(" defaultWidth.left       = %d px\n",  pInfo->defaultWidth.left);
    OS_Printf(" defaultWidth.glyphWidth = %d px\n",  pInfo->defaultWidth.glyphWidth);
    OS_Printf(" defaultWidth.charWidth  = %d px\n",  pInfo->defaultWidth.charWidth);
    OS_Printf(" encoding                = %s\n",     encodingString[pInfo->encoding]);
}



/*---------------------------------------------------------------------------*
  Name:         PrintFontGlyph

  Description:  Performs debug print for the CGLP block of the NITRO font.

  Arguments:    pGlyph: Pointer to the CGLP block

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintFontGlyph(const NNSG2dFontGlyph* pGlyph)
{
    NNS_G2D_POINTER_ASSERT( pGlyph );

    OS_Printf(" NNSG2dFontGlyph\n");
    OS_Printf(" cellWidth          = %d px\n",      pGlyph->cellWidth);
    OS_Printf(" cellHeight         = %d px\n",      pGlyph->cellHeight);
    OS_Printf(" cellSize           = %d byte\n",    pGlyph->cellSize);
    OS_Printf(" baselinePos        = %d px\n",      pGlyph->baselinePos);
    OS_Printf(" maxCharWidth       = %d px\n",      pGlyph->maxCharWidth);
    OS_Printf(" bpp                = %d bpp\n",     pGlyph->bpp);
    OS_Printf(" flags              = ");
    if( (pGlyph->flags & NNS_G2D_FONT_FLAG_TBRL) != 0 )
    {
        OS_Printf("NNS_G2D_FONT_FLAG_TBRL ");
    }

    switch( pGlyph->flags & NNS_G2D_FONT_FLAG_ROT_MASK )
    {
    case NNS_G2D_FONT_FLAG_ROT_0:   OS_Printf("NNS_G2D_FONT_FLAG_ROT_0\n");   break;
    case NNS_G2D_FONT_FLAG_ROT_90:  OS_Printf("NNS_G2D_FONT_FLAG_ROT_90\n");  break;
    case NNS_G2D_FONT_FLAG_ROT_180: OS_Printf("NNS_G2D_FONT_FLAG_ROT_180\n"); break;
    case NNS_G2D_FONT_FLAG_ROT_270: OS_Printf("NNS_G2D_FONT_FLAG_ROT_270\n"); break;
    }
}



/*---------------------------------------------------------------------------*
  Name:         PrintFontWidth

  Description:  Performs debug print for the CWDH block of the NITRO font.

  Arguments:    pGlyph: Pointer to the CWDH block

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintFontWidth(const NNSG2dFontWidth* pWidth)
{
    NNS_G2D_POINTER_ASSERT( pWidth );

    OS_Printf(" NNSG2dFontWidth\n");
    OS_Printf(" indexBegin         = 0x%04X\n", pWidth->indexBegin);
    OS_Printf(" indexEnd           = 0x%04X\n", pWidth->indexEnd);
}



/*---------------------------------------------------------------------------*
  Name:         PrintFontCMapTable

  Description:  Performs a debug print for the NNS_G2D_MAPMETHOD_TABLE-type mapping table.
                

  Arguments:    table:          Pointer to the mapping table
                num:            Number of entries contained in the mapping table
                ccodeOffset:    Character code offset

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintFontCMapTable(const u16* table, int num, int ccodeOffset)
{
    int i;

    NNS_G2D_POINTER_ASSERT( table );

    for( i = 0; i < num; i++ )
    {
        OS_Printf("   %4d: 0x%04X -> %d\n", i,
            i + ccodeOffset, table[i] );
    }
}



/*---------------------------------------------------------------------------*
  Name:         PrintFontCMapScanList

  Description:  Performs a debug print for the NNS_G2D_MAPMETHOD_SCAN-type mapping table.
                

  Arguments:    scan:   Pointer to the mapping information

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintFontCMapScanList(const NNSG2dCMapInfoScan* scan)
{
    int i;

    NNS_G2D_POINTER_ASSERT( scan );

    for( i = 0; i < scan->num; i++ )
    {
        OS_Printf("   %4d: 0x%04X -> %d\n", i,
            scan->entries[i].ccode, scan->entries[i].index );
    }
}



/*---------------------------------------------------------------------------*
  Name:         PrintFontCMapTable

  Description:  Performs debug print for the CMAP block of the NITRO font.

  Arguments:    pGlyph: Pointer to the CMAP block

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintFontCMap(const NNSG2dFontCodeMap* pCMap)
{
    const static char* mappingMethodString[] =
    {
        "NNS_G2D_MAPMETHOD_DIRECT",
        "NNS_G2D_MAPMETHOD_TABLE",
        "NNS_G2D_MAPMETHOD_SCAN",
        "NNS_G2D_NUM_OF_MAPMETHOD"
    };

    NNS_G2D_POINTER_ASSERT( pCMap );
    NNS_G2D_MINMAX_ASSERT( pCMap->mappingMethod, 0, NNS_G2D_NUM_OF_MAPMETHOD - 1 );

    OS_Printf(" NNSG2dFontCodeMap\n");
    OS_Printf(" ccodeBegin         = 0x%04X\n", pCMap->ccodeBegin);
    OS_Printf(" ccodeEnd           = 0x%04X\n", pCMap->ccodeEnd);
    OS_Printf(" mappingMethod      = %s\n", mappingMethodString[pCMap->mappingMethod]);

    switch( pCMap->mappingMethod )
    {
    //---------------------------------------------------------
    // Calculate offset
    case NNS_G2D_MAPMETHOD_DIRECT:
        {
            OS_Printf(" indexOffset        = 0x%04X\n", pCMap->mapInfo[0]);
        }
        break;

    //---------------------------------------------------------
    // Table reference
    case NNS_G2D_MAPMETHOD_TABLE:
        {
#ifdef NNS_G2D_FONT_SHOW_CMAP_LIST
            int ccodeOffset = pCMap->ccodeBegin;
            int num = pCMap->ccodeEnd - pCMap->ccodeBegin + 1;

            PrintFontCMapTable(pCMap->mapInfo, num, ccodeOffset);
#endif
        }
        break;

    //---------------------------------------------------------
    // Binary search
    case NNS_G2D_MAPMETHOD_SCAN:
        {
            const NNSG2dCMapInfoScan* const ws = (NNSG2dCMapInfoScan*)(pCMap->mapInfo);
            OS_Printf(" num                = %d\n", ws->num);
#ifdef NNS_G2D_FONT_SHOW_CMAP_LIST
            PrintFontCMapScanList(ws);
#endif
        }
        break;

    //---------------------------------------------------------
    // unknown
    default:
        OS_Panic("unknwon MAPMETHOD");
    }
}



/*---------------------------------------------------------------------------*
  Name:         PrintSpliter

  Description:  Outputs a debug print dividing line.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static NNS_G2D_INLINE void PrintSpliter(void)
{
    OS_Printf( "---------------------------------------------\n" );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dPrintFont

  Description:  Outputs the NITRO font information to debug output.

  Arguments:    pFont:  Pointer to the NITRO font.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NNS_G2dPrintFont( const NNSG2dFont* pFont )
{
    NNS_G2D_POINTER_ASSERT( pFont );

    {
        const NNSG2dFontWidth* pWidth = pFont->pRes->pWidth;
        const NNSG2dFontCodeMap* pCMap = pFont->pRes->pMap;

        PrintSpliter();
        PrintFontInformation(pFont->pRes);

        PrintSpliter();
        PrintFontGlyph(pFont->pRes->pGlyph);

        while( pWidth != NULL )
        {
            PrintSpliter();
            PrintFontWidth(pWidth);

            pWidth = pWidth->pNext;
        }

        while( pCMap != NULL )
        {
            PrintSpliter();
            PrintFontCMap(pCMap);

            pCMap = pCMap->pNext;
        }

        PrintSpliter();
    }
}

#endif // SDK_FINALROM

