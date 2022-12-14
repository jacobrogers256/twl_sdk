/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - snd
  File:     sndarc_loader.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/snd/sndarc_loader.h>

#include <nitro/types.h>
#include <nnsys/snd/seqdata.h>
#include <nnsys/snd/sndarc.h>

#include <nitro/snd.h>
#include <nnsys/misc.h>
#include <nnsys/snd/heap.h>

/******************************************************************************
	Macro Definitions
 ******************************************************************************/

#define RESERVED_AREASIZE 32

#define ROUNDUP( value, align ) ( ( (value) + ( (align) - 1 ) ) & ~( (align) - 1 ) )
#define SNDHEAP_ALIGN( value ) ( ROUNDUP( value + RESERVED_AREASIZE, 32 ) + 32 )

/******************************************************************************
    static function declarations
 ******************************************************************************/

static NNSSndSeqData* LoadSeq( u32 fileId, NNSSndHeapHandle heap, BOOL bSetAddr );
static NNSSndSeqArc*  LoadSeqArc( u32 fileId, NNSSndHeapHandle heap, BOOL bSetAddr );
static SNDBankData*   LoadBank( u32 fileId, NNSSndHeapHandle heap, BOOL bSetAddr );
static SNDWaveArc*    LoadWaveArc( u32 fileId, NNSSndHeapHandle heap, BOOL bSetAddr );
static SNDWaveArc*    LoadWaveArcTable( u32 fileId, NNSSndHeapHandle heap, BOOL bSetAddr );

static BOOL LoadSingleWaves( SNDWaveArc* waveArc, const SNDBankData* bank, int waveArcNo, u32 fileId, NNSSndHeapHandle heap );
static BOOL LoadSingleWave( SNDWaveArc* waveArc, int waveNo, u32 fileId, NNSSndHeapHandle heap );

static void DisposeCallback( void* mem, NNSSndArc* arc, u32 fileId );
static void SeqDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 );
static void BankDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 );
static void WaveArcDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 );
static void WaveArcTableDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 );
static void SingleWaveDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 );

/******************************************************************************
    public functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcLoadGroup

  Description:  Loads the group

  Arguments:    groupNo - Group number
                heap    - The heap that stores the data

  Returns:      Whether the load succeeded.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcLoadGroup( int groupNo, NNSSndHeapHandle heap )
{
    NNSSndArcLoadResult result;
    
    result = NNSi_SndArcLoadGroup( groupNo, heap );
    
    return result == NNS_SND_ARC_LOAD_SUCESS ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcLoadSeq

  Description:  Loads the sequence data

  Arguments:    seqNo      - sequence number
                heap    - The heap that stores the data

  Returns:      Whether the load succeeded.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcLoadSeq( int seqNo, NNSSndHeapHandle heap )
{
    NNSSndArcLoadResult result;

    result = NNSi_SndArcLoadSeq( seqNo, NNS_SND_ARC_LOAD_ALL, heap, TRUE, NULL );
    
    return result == NNS_SND_ARC_LOAD_SUCESS ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcLoadSeqArc

  Description:  Loads the sequence archive

  Arguments:    seqArcNo   - sequence archive number
                heap    - The heap that stores the data

  Returns:      Whether the load succeeded.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcLoadSeqArc( int seqArcNo, NNSSndHeapHandle heap )
{
    NNSSndArcLoadResult result;

    result = NNSi_SndArcLoadSeqArc( seqArcNo, NNS_SND_ARC_LOAD_ALL, heap, TRUE, NULL );
    
    return result == NNS_SND_ARC_LOAD_SUCESS ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcLoadBank

  Description:  Loads the bank data

  Arguments:    bankNo    - bank number
                heap    - The heap that stores the data

  Returns:      Whether the load succeeded.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcLoadBank( int bankNo, NNSSndHeapHandle heap )
{
    NNSSndArcLoadResult result;

    result = NNSi_SndArcLoadBank( bankNo, NNS_SND_ARC_LOAD_ALL, heap, TRUE, NULL );
    
    return result == NNS_SND_ARC_LOAD_SUCESS ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcLoadWaveArc

  Description:  Loads the waveform archive.

  Arguments:    waveArcNo - waveform archive number
                heap    - The heap that stores the data

  Returns:      Whether the load succeeded.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcLoadWaveArc( int waveArcNo, NNSSndHeapHandle heap )
{
    NNSSndArcLoadResult result;
    
    result = NNSi_SndArcLoadWaveArc( waveArcNo, NNS_SND_ARC_LOAD_ALL, heap, TRUE, NULL );
    
    return result == NNS_SND_ARC_LOAD_SUCESS ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcLoadSeqEx

  Description:  Loads the sequence data

  Arguments:    seqNo      - sequence number
                loadFlag - The data specification flag to load
                heap    - The heap that stores the data

  Returns:      Whether the load succeeded.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcLoadSeqEx( int seqNo, u32 loadFlag, NNSSndHeapHandle heap )
{
    NNSSndArcLoadResult result;
    
    result = NNSi_SndArcLoadSeq( seqNo, loadFlag, heap, TRUE, NULL );
    
    return result == NNS_SND_ARC_LOAD_SUCESS ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_SndArcLoadBankEx

  Description:  Loads the bank data

  Arguments:    bankNo    - bank number
                loadFlag - The data specification flag to load
                heap    - The heap that stores the data

  Returns:      Whether the load succeeded.
 *---------------------------------------------------------------------------*/
BOOL NNS_SndArcLoadBankEx( int bankNo, u32 loadFlag, NNSSndHeapHandle heap )
{
    NNSSndArcLoadResult result;
    
    result = NNSi_SndArcLoadBank( bankNo, loadFlag, heap, TRUE, NULL );
    
    return result == NNS_SND_ARC_LOAD_SUCESS ? TRUE : FALSE;
}

/******************************************************************************
	Private function
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndArcLoadGroup

  Description:  Loads the group

  Arguments:    groupNo - Group number
                heap    - The heap that stores the data

  Returns:      Whether the load succeeded.
 *---------------------------------------------------------------------------*/
NNSSndArcLoadResult NNSi_SndArcLoadGroup( int groupNo, NNSSndHeapHandle heap )
{
    const NNSSndArcGroupInfo* info;
    const NNSSndArcGroupItem* item;
    NNSSndArcLoadResult result;
    int itemNo;
    
    info = NNS_SndArcGetGroupInfo( groupNo );
    if ( info == NULL ) return NNS_SND_ARC_LOAD_ERROR_INVALID_GROUP_NO;
    
    for( itemNo = 0; itemNo < info->count ; itemNo++ )
    {
        item = & info->item[ itemNo ];
        
        switch( item->type ) {
        case NNS_SNDARC_SNDTYPE_SEQ:
            result = NNSi_SndArcLoadSeq( (int)( item->index ), item->loadFlag, heap, TRUE, NULL );
            if ( result != NNS_SND_ARC_LOAD_SUCESS ) return result;
            break;
        case NNS_SNDARC_SNDTYPE_SEQARC:
            result = NNSi_SndArcLoadSeqArc( (int)( item->index ), item->loadFlag, heap, TRUE, NULL );
            if ( result != NNS_SND_ARC_LOAD_SUCESS ) return result;
            break;
        case NNS_SNDARC_SNDTYPE_BANK:
            result = NNSi_SndArcLoadBank( (int)( item->index ), item->loadFlag, heap, TRUE, NULL );
            if ( result != NNS_SND_ARC_LOAD_SUCESS ) return result;
            break;
        case NNS_SNDARC_SNDTYPE_WAVEARC:
            result = NNSi_SndArcLoadWaveArc( (int)( item->index ), item->loadFlag, heap, TRUE, NULL );
            if ( result != NNS_SND_ARC_LOAD_SUCESS ) return result;
            break;
        default:
            NNS_ASSERT( FALSE );
            break;
        }
    }
    
    return NNS_SND_ARC_LOAD_SUCESS;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndArcLoadSeq

  Description:  Loads the sequence data

  Arguments:    seqNo      - sequence number
                loadFlag - The data specification flag to load
                heap    - The heap that stores the data
                bSetAddr - Whether to set address with NNS_SndArcSetFileAddress
                pData    - Pointer for getting the position where data is loaded

  Returns:      Result code
 *---------------------------------------------------------------------------*/
NNSSndArcLoadResult NNSi_SndArcLoadSeq( int seqNo, u32 loadFlag, NNSSndHeapHandle heap, BOOL bSetAddr, struct NNSSndSeqData** pData )
{
    const NNSSndArcSeqInfo* seqInfo;
    NNSSndSeqData* seqData = NULL;
    SNDBankData* bank = NULL;
    NNSSndArcLoadResult result;
    
    // Get the information
    seqInfo = NNS_SndArcGetSeqInfo( seqNo );
    if ( seqInfo == NULL ) return NNS_SND_ARC_LOAD_ERROR_INVALID_SEQ_NO;
    
    // Load bank (waveform data)
    result = NNSi_SndArcLoadBank( seqInfo->param.bankNo, loadFlag, heap, bSetAddr, NULL );
    if ( result != NNS_SND_ARC_LOAD_SUCESS ) return result;
    
    // Load sequence data
    if ( loadFlag & NNS_SND_ARC_LOAD_SEQ )
    {
        seqData = LoadSeq( seqInfo->fileId, heap, bSetAddr );
        if ( seqData == NULL ) {
            return NNS_SND_ARC_LOAD_ERROR_FAILED_LOAD_SEQ;
        }
    }
    else
    {
        seqData = (NNSSndSeqData*)NNS_SndArcGetFileAddress( seqInfo->fileId );
    }
    
    if ( pData != NULL ) *pData = seqData;
    
    return NNS_SND_ARC_LOAD_SUCESS;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndArcLoadSeqArc

  Description:  Loads the sequence archive

  Arguments:    seqArcNo   - sequence archive number
                loadFlag - The data specification flag to load
                heap    - The heap that stores the data
                bSetAddr - Whether to set address with NNS_SndArcSetFileAddress
                pData    - Pointer for getting the position where data is loaded

  Returns:      Result code
 *---------------------------------------------------------------------------*/
NNSSndArcLoadResult NNSi_SndArcLoadSeqArc( int seqArcNo, u32 loadFlag, NNSSndHeapHandle heap, BOOL bSetAddr, struct NNSSndSeqArc** pData )
{
    const NNSSndArcSeqArcInfo* seqArcInfo;
    NNSSndSeqArc* seqArc = NULL;
    
    // Get the information
    seqArcInfo = NNS_SndArcGetSeqArcInfo( seqArcNo );
    if ( seqArcInfo == NULL ) return NNS_SND_ARC_LOAD_ERROR_INVALID_SEQARC_NO;
    
    // Load sequence archive
    if ( loadFlag & NNS_SND_ARC_LOAD_SEQARC )
    {
        seqArc = LoadSeqArc( seqArcInfo->fileId, heap, bSetAddr );
        if ( seqArc == NULL ) {
            return NNS_SND_ARC_LOAD_ERROR_FAILED_LOAD_SEQARC;
        }
    }
    else
    {
        seqArc = (NNSSndSeqArc*)NNS_SndArcGetFileAddress( seqArcInfo->fileId );
    }
    
    if ( pData != NULL ) *pData = seqArc;
                             
    return NNS_SND_ARC_LOAD_SUCESS;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndArcLoadBank

  Description:  Loads the bank data

  Arguments:    bankNo    - bank number
                loadFlag - The data specification flag to load
                heap    - The heap that stores the data
                bSetAddr - Whether to set address with NNS_SndArcSetFileAddress
                pData    - Pointer for getting the position where data is loaded

  Returns:      Result code
 *---------------------------------------------------------------------------*/
NNSSndArcLoadResult NNSi_SndArcLoadBank(
    int bankNo,
    u32 loadFlag,
    NNSSndHeapHandle heap,
    BOOL bSetAddr,
    struct SNDBankData** pData
)
{
    const NNSSndArcBankInfo* bankInfo;
    const NNSSndArcWaveArcInfo* waveArcInfo;
    SNDBankData* bank = NULL;
    SNDWaveArc* waveArc;
    NNSSndArcLoadResult result;
    int i;

    // Get the information
    bankInfo = NNS_SndArcGetBankInfo( bankNo );
    if ( bankInfo == NULL ) return NNS_SND_ARC_LOAD_ERROR_INVALID_BANK_NO;
    
    // Load bank
    if ( loadFlag & NNS_SND_ARC_LOAD_BANK )
    {
        bank = LoadBank( bankInfo->fileId, heap, bSetAddr );
        if ( bank == NULL ) {
            return NNS_SND_ARC_LOAD_ERROR_FAILED_LOAD_BANK;
        }
    }
    else
    {
        bank = (SNDBankData*)NNS_SndArcGetFileAddress( bankInfo->fileId );
    }
    
    // Load waveform data
    for( i = 0; i < NNS_SND_ARC_BANK_TO_WAVEARC_NUM ; i++ )
    {
        if ( bankInfo->waveArcNo[i] == NNS_SND_ARC_INVALID_WAVEARC_NO ) continue;
            
        // Gets the waveform archive information
        waveArcInfo = NNS_SndArcGetWaveArcInfo( bankInfo->waveArcNo[i] );
        if ( waveArcInfo == NULL ) return NNS_SND_ARC_LOAD_ERROR_INVALID_WAVEARC_NO;
        
        // Load waveform archive
        result = NNSi_SndArcLoadWaveArc( bankInfo->waveArcNo[i], loadFlag, heap, bSetAddr, &waveArc );
        if ( result != NNS_SND_ARC_LOAD_SUCESS ) return result;
                
        if ( waveArcInfo->flags & NNS_SND_ARC_WAVEARC_SINGLE_LOAD )
        {
            // Individual load of waveform data
            if ( loadFlag & NNS_SND_ARC_LOAD_WAVE )
            {
                if ( ! LoadSingleWaves( waveArc, bank, i, waveArcInfo->fileId, heap ) ) {
                    return NNS_SND_ARC_LOAD_ERROR_FAILED_LOAD_WAVE;
                }
            }
        }
            
        // Association of waveform and bank
        if ( bank != NULL && waveArc != NULL ) {
            SND_AssignWaveArc( bank, i, waveArc );
        }
    }
    
    if ( pData != NULL ) *pData = bank;
    
    return NNS_SND_ARC_LOAD_SUCESS;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndArcLoadWaveArc

  Description:  Loads waveform data

  Arguments:    waveArcNo - waveform archive number
                loadFlag - The data specification flag to load
                heap    - The heap that stores the data
                bSetAddr - Whether to set address with NNS_SndArcSetFileAddress
                pData    - Pointer for getting the position where data is loaded

  Returns:      Result code
 *---------------------------------------------------------------------------*/
NNSSndArcLoadResult NNSi_SndArcLoadWaveArc(
    int waveArcNo,
    u32 loadFlag,
    NNSSndHeapHandle heap,
    BOOL bSetAddr,
    struct SNDWaveArc** pData
)
{
    const NNSSndArcWaveArcInfo* waveArcInfo;
    SNDWaveArc* waveArc = NULL;
    
    waveArcInfo = NNS_SndArcGetWaveArcInfo( waveArcNo );
    if ( waveArcInfo == NULL ) return NNS_SND_ARC_LOAD_ERROR_INVALID_WAVEARC_NO;
    
    if ( loadFlag & NNS_SND_ARC_LOAD_WAVE )
    {
        if ( waveArcInfo->flags & NNS_SND_ARC_WAVEARC_SINGLE_LOAD ) {
            // Load individual waveform data
            waveArc = LoadWaveArcTable( waveArcInfo->fileId, heap, bSetAddr );
        }
        else {
            // Load waveforms together
            waveArc = LoadWaveArc( waveArcInfo->fileId, heap, bSetAddr );
        }
        
        if ( waveArc == NULL ){
            return NNS_SND_ARC_LOAD_ERROR_FAILED_LOAD_WAVE;
        }
    }
    else
    {
        waveArc = (SNDWaveArc*)NNS_SndArcGetFileAddress( waveArcInfo->fileId );
    }
    
    if ( pData != NULL ) *pData = waveArc;
    
    return NNS_SND_ARC_LOAD_SUCESS;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_SndArcLoadFile

  Description:  Loads a file

  Arguments:    fileId - file ID
                callback - Callback that is called when memory area is discarded
                data1    - User data
                data2    - User data
                heap    - The heap that stores the data

  Returns:      Memory address where loaded;
                If load fails, returns NULL
 *---------------------------------------------------------------------------*/
void* NNSi_SndArcLoadFile(
    u32 fileId,
    NNSSndHeapDisposeCallback callback,
    u32 data1,
    u32 data2,
    NNSSndHeapHandle heap
)
{
    void* buffer;
    u32 len;
    
    len = NNS_SndArcGetFileSize( fileId );
    if ( len == 0 ) return NULL;
    
    if ( heap == NNS_SND_HEAP_INVALID_HANDLE ) return NULL;
    
    buffer = NNS_SndHeapAlloc( heap, len + RESERVED_AREASIZE , callback, data1, data2 ); // NOTE: reserved 32byte
    if ( buffer == NULL ) return NULL;
    
    if ( NNS_SndArcReadFile( fileId, buffer, (s32)len, 0 ) != len ) {
        return NULL;
    }
    
    DC_StoreRange( buffer, len );
    
    return buffer;
}

/******************************************************************************
    Static function
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         LoadSeq

  Description:  Loads the sequence data

  Arguments:    fileId - file ID
                heap    - The heap that stores the data
                bSetAddr - Whether to set address with NNS_SndArcSetFileAddress

  Returns:      Address where the sequence was loaded
 *---------------------------------------------------------------------------*/
static NNSSndSeqData* LoadSeq( u32 fileId, NNSSndHeapHandle heap, BOOL bSetAddr )
{
    void* buffer;
    
    buffer = NNS_SndArcGetFileAddress( fileId );
    if ( buffer == NULL )
    {
        buffer = NNSi_SndArcLoadFile(
            fileId,
            SeqDisposeCallback,
            bSetAddr ? (u32)NNS_SndArcGetCurrent() : 0,
            fileId,
            heap
        );
        
        if ( bSetAddr && buffer != NULL ) {
            NNS_SndArcSetFileAddress( fileId, buffer );
        }
    }
    
    return (NNSSndSeqData*)buffer;
}

/*---------------------------------------------------------------------------*
  Name:         LoadSeqArc

  Description:  Loads the sequence archive

  Arguments:    fileId - file ID
                heap    - The heap that stores the data
                bSetAddr - Whether to set address with NNS_SndArcSetFileAddress

  Returns:      Address where the sequence was loaded
 *---------------------------------------------------------------------------*/
static NNSSndSeqArc* LoadSeqArc( u32 fileId, NNSSndHeapHandle heap, BOOL bSetAddr )
{
    void* buffer;
    
    buffer = NNS_SndArcGetFileAddress( fileId );
    if ( buffer == NULL )
    {
        buffer = NNSi_SndArcLoadFile(
            fileId,
            SeqDisposeCallback,
            bSetAddr ? (u32)NNS_SndArcGetCurrent() : 0,
            fileId,
            heap
        );
        
        if ( bSetAddr && buffer != NULL ) {
            NNS_SndArcSetFileAddress( fileId, buffer );            
        }
    }
    
    return (NNSSndSeqArc*)buffer;
}

/*---------------------------------------------------------------------------*
  Name:         LoadBank

  Description:  Loads the bank data

  Arguments:    fileId - file ID
                heap    - The heap that stores the data
                bSetAddr - Whether to set address with NNS_SndArcSetFileAddress

  Returns:      Address where the sequence was loaded
 *---------------------------------------------------------------------------*/
static SNDBankData* LoadBank( u32 fileId, NNSSndHeapHandle heap, BOOL bSetAddr )
{
    void* buffer;
    
    buffer = NNS_SndArcGetFileAddress( fileId );
    if ( buffer == NULL )
    {
        buffer = NNSi_SndArcLoadFile(
            fileId,
            BankDisposeCallback,
            bSetAddr ? (u32)NNS_SndArcGetCurrent() : 0,
            fileId,
            heap
        );
        
        if ( bSetAddr && buffer != NULL ) {
            NNS_SndArcSetFileAddress( fileId, buffer );            
        }
    }
    
    return (SNDBankData*)buffer;
}


/*---------------------------------------------------------------------------*
  Name:         LoadWaveArc

  Description:  Loads a waveform archive

  Arguments:    fileId - file ID
                heap    - The heap that stores the data
                bSetAddr - Whether to set address with NNS_SndArcSetFileAddress

  Returns:      Address where the sequence was loaded
 *---------------------------------------------------------------------------*/
static SNDWaveArc* LoadWaveArc( u32 fileId, NNSSndHeapHandle heap, BOOL bSetAddr )
{
    void* buffer;
    
    buffer = NNS_SndArcGetFileAddress( fileId );
    if ( buffer == NULL )
    {
        buffer = NNSi_SndArcLoadFile(
            fileId,
            WaveArcDisposeCallback,
            bSetAddr ? (u32)NNS_SndArcGetCurrent() : 0,
            fileId,
            heap
        );
        
        if ( bSetAddr && buffer != NULL ) {
            NNS_SndArcSetFileAddress( fileId, buffer );            
        }
    }
    
    return (SNDWaveArc*)buffer;
}

/*---------------------------------------------------------------------------*
  Name:         LoadWaveArcTable

  Description:  Loads only the table section of the waveform archive

  Arguments:    fileId - File ID of the waveform archive
                heap - Sound heap handle
                bSetAddr - Whether to set address with NNS_SndArcSetFileAddress

  Returns:      Address where the sequence was loaded
 *---------------------------------------------------------------------------*/
static SNDWaveArc*  LoadWaveArcTable( u32 fileId, NNSSndHeapHandle heap, BOOL bSetAddr )
{
    static SNDWaveArc waveArcHeader;
    SNDWaveArc* waveArc;
    u32 len;
    u32 tableSize;
    s32 readSize;
    
    waveArc = (SNDWaveArc*)NNS_SndArcGetFileAddress( fileId );
    if ( waveArc == NULL )
    {
        // Load header
        readSize = NNS_SndArcReadFile( fileId, &waveArcHeader, sizeof(waveArcHeader), 0 );
        if ( readSize != sizeof(waveArcHeader) ) {
            // Load failed
            return NULL;
        }
        
        tableSize = sizeof(u32) * waveArcHeader.waveCount;
        len = sizeof(waveArcHeader) + tableSize * 2;
        
        if ( heap == NNS_SND_HEAP_INVALID_HANDLE ) {
            return NULL;
        }

        // Memory allocation
        waveArc = (SNDWaveArc*)NNS_SndHeapAlloc(
            heap,
            len + RESERVED_AREASIZE , // NOTE: reserved 32byte
            WaveArcTableDisposeCallback,
            bSetAddr ? (u32)NNS_SndArcGetCurrent() : 0,
            fileId
        );
        if ( waveArc == NULL ) {
            // Insufficient memory
            return NULL;
        }
        
        // Load head and table
        readSize = NNS_SndArcReadFile(
            fileId,
            waveArc,
            (s32)( sizeof(waveArcHeader) + tableSize ),
            0
        );
        if ( readSize != sizeof(waveArcHeader) + tableSize ) {
            // Load failed
            // Note: The allocated memory cannot be released
            return NULL;
        }
        
        // Copy table
        MI_CpuCopy8( waveArc->waveOffset, &waveArc->waveOffset[ waveArc->waveCount ], tableSize );
        
        // Clear table
        MI_CpuClear8( waveArc->waveOffset, tableSize );

        //
        DC_StoreRange( waveArc, len );
        
        // Register file address
        if ( bSetAddr ) {
            NNS_SndArcSetFileAddress( fileId, waveArc );
        }
    }
    
    return waveArc;
}

/*---------------------------------------------------------------------------*
  Name:         DisposeCallback

  Description:  The shared process of the callback called when getting rid of the sound data

  Arguments:    mem    - The address of the memory block
                arc    - The sound archive.
                fileId - file ID

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DisposeCallback( void* mem, NNSSndArc* arc, u32 fileId )
{
    NNSSndArc* oldArc;
    OSIntrMode old;
    
    if ( arc == NULL ) return;
    
    old = OS_DisableInterrupts();
    
    oldArc = NNS_SndArcSetCurrent( arc );
    
    if ( mem == NNS_SndArcGetFileAddress( fileId ) )
    {
        NNS_SndArcSetFileAddress( fileId, NULL );
    }
    
    (void)NNS_SndArcSetCurrent( oldArc );
    
    (void)OS_RestoreInterrupts( old );
}

/*---------------------------------------------------------------------------*
  Name:         SeqDisposeCallback

  Description:  The callback called when getting rid of the sequence data

  Arguments:    mem    - The address of the memory block
                size  - The size of the memory block
                data1 - User data (pointer to sound archive)
                data2 - The user data (The file ID)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SeqDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 )
{
    NNSSndArc* arc = (NNSSndArc*)data1;
    u32 fileId = data2;
    
    DisposeCallback( mem, arc, fileId );
    SND_InvalidateSeqData( mem, (u8*)mem + size );
}

/*---------------------------------------------------------------------------*
  Name:         BankDisposeCallback

  Description:  The callback invoked when getting rid of bank data

  Arguments:    mem    - The address of the memory block
                size  - The size of the memory block
                data1 - User data (pointer to sound archive)
                data2 - The user data (The file ID)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void BankDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 )
{
    SNDBankData* bank = (SNDBankData*)mem;
    NNSSndArc* arc = (NNSSndArc*)data1;
    u32 fileId = data2;
    
    DisposeCallback( mem, arc, fileId );
    SND_InvalidateBankData( mem, (u8*)mem + size );
    
    SND_DestroyBank( bank );
}

/*---------------------------------------------------------------------------*
  Name:         WaveArcDisposeCallback

  Description:  The callback called when getting rid of the waveform archive data

  Arguments:    mem    - The address of the memory block
                size  - The size of the memory block
                data1 - User data (pointer to sound archive)
                data2 - The user data (The file ID)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WaveArcDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 )
{
    SNDWaveArc* waveArc = (SNDWaveArc*)mem;
    NNSSndArc* arc = (NNSSndArc*)data1;
    u32 fileId = data2;
    
    DisposeCallback( mem, arc, fileId );
    SND_InvalidateWaveData( mem, (u8*)mem + size );
    
    SND_DestroyWaveArc( waveArc );
}

/*---------------------------------------------------------------------------*
  Name:         WaveArcTableDisposeCallback

  Description:  Callback called when destroying waveform archive table

  Arguments:    mem    - The address of the memory block
                size  - The size of the memory block
                data1 - User data (pointer to sound archive)
                data2 - The user data (The file ID)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WaveArcTableDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 )
{
    SNDWaveArc* waveArc = (SNDWaveArc*)mem;
    NNSSndArc* arc = (NNSSndArc*)data1;
    u32 fileId = data2;
       
    (void)size;

    DisposeCallback( mem, arc, fileId );
    
    SND_DestroyWaveArc( waveArc );
}

/*---------------------------------------------------------------------------*
  Name:         SingleWaveisposeCallback

  Description:  Callback called when destroying individual waveform data

  Arguments:    mem    - The address of the memory block
                size  - The size of the memory block
                data1 - User data (pointer to the waveform archive)
                data2 - User data (waveform index)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SingleWaveDisposeCallback( void* mem, u32 size, u32 data1, u32 data2 )
{
    SNDWaveArc* waveArc = (SNDWaveArc*)data1;
    u32 waveNo = data2;
    
    if ( mem == SND_GetWaveDataAddress( waveArc, (int)waveNo ) ) {
        SND_SetWaveDataAddress( waveArc, (int)waveNo, NULL );
    }
    
    SND_InvalidateWaveData( mem, (u8*)mem + size );
}

/*---------------------------------------------------------------------------*
  Name:         LoadSingleWave

  Description:  Loads the waveform in the waveform archive

  Arguments:    waveArc - waveform archive
                waveNo  - waveform index number
                fileId - File ID of the waveform archive
                heap - Sound heap handle

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
static BOOL LoadSingleWave( SNDWaveArc* waveArc, int waveNo, u32 fileId, NNSSndHeapHandle heap )
{
    SNDWaveData* buffer;
    u32 len;
    u32 begin;
    u32 end;
    u32 waveCount;
    
    NNS_MINMAX_ASSERT( waveNo, 0, SND_GetWaveDataCount( waveArc )-1 );
    
    if ( SND_GetWaveDataAddress( waveArc, waveNo ) != NULL ) {
        // Finished loading
        return TRUE;
    }
    
    waveCount = SND_GetWaveDataCount( waveArc );
    
    begin = waveArc->waveOffset[ waveArc->waveCount + waveNo ];
    if ( waveNo < waveCount-1 ) {
        end = waveArc->waveOffset[ waveArc->waveCount + waveNo + 1 ];
    }
    else {
        end = waveArc->fileHeader.fileSize;
    }
    len = end - begin;
    
    if ( heap == NNS_SND_HEAP_INVALID_HANDLE ) {
        return FALSE;
    }
    
    buffer = (SNDWaveData*)NNS_SndHeapAlloc(
        heap,
        len + RESERVED_AREASIZE,
        SingleWaveDisposeCallback,
        (u32)waveArc,
        (u32)waveNo
    );
    if ( buffer == NULL ) return FALSE;
    
    if ( NNS_SndArcReadFile( fileId, buffer, (s32)len, (s32)begin ) != len ) {
        return FALSE;
    }
    
    DC_StoreRange( buffer, len );
    
    SND_SetWaveDataAddress( waveArc, waveNo, buffer );
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         LoadSingleWaves

  Description:  Loads the waveform in the waveform archive necessary for the bank

  Arguments:    waveArc - waveform archive
                bank - Bank
                waveArcNo - Waveform archive number in the bank
                fileId - File ID of the waveform archive
                heap - Sound heap handle

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL LoadSingleWaves( SNDWaveArc* waveArc, const SNDBankData* bank, int waveArcNo, u32 fileId, NNSSndHeapHandle heap )
{
    SNDInstPos pos = SND_GetFirstInstDataPos( bank );
    SNDInstData inst;
    
    NNS_NULL_ASSERT( waveArc );
    
    if ( bank == NULL ) {
        NNS_WARNING( bank != NULL, "Cannot load single wave data because the bank data is not on Memory." );
        return FALSE;
    }
    
    while( SND_GetNextInstData( bank, &inst, &pos ) ) {
        if ( inst.type == SND_INST_PCM && waveArcNo == inst.param.wave[1]  ) {
            if ( ! LoadSingleWave( waveArc, inst.param.wave[0], fileId, heap ) ) {
                return FALSE;
            }
        }
    }
    
    return TRUE;
}

/*====== End of heap.c ======*/

