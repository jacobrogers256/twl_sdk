/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - 
  File:     mi_compress.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro/types.h>
#include <nitro/mi/uncompress.h>
#include <nitro/mi/compress.h>

static u32 SearchLZ(const u8 *startp, const u8 *nextp, u32 remainSize, u16 *offset, u32 maxLength );

//===========================================================================
//  LZ Encoding
//===========================================================================

/*---------------------------------------------------------------------------*
  Name:         MI_CompressLZ

  Description:  Performs LZ77 compression.

  Arguments:    srcp:            Pointer to compression source data
                size:            Size of compression source data
                dstp            Pointer to compressed data
                                The buffer must be larger than the size of the compression source data.

  Returns:      The data size after compression.
                If compressed data is larger than original data, compression is terminated and 0 is returned.
 *---------------------------------------------------------------------------*/
u32 MI_CompressLZImpl(const u8 *srcp, u32 size, u8 *dstp, BOOL exFormat)
{
    u32     LZDstCount;                // Number of bytes of compressed data
    u32     LZSrcCount;                // Processed data volume of the compression target data (in bytes)
    const u8 *srcStartp;
    u8      LZCompFlags;               // Flag sequence indicating whether there is a compression
    u8     *LZCompFlagsp;              // Point to memory regions storing LZCompFlags
    u16     lastOffset;                // Offset to matching data (the longest matching data at the time) 
    u32     lastLength;                // Length of matching data (the longest matching data at the time)
    u8      i;
    u32     dstMax;
    const u32 MAX_LENGTH = (exFormat)? (0xFFFF + 0xFF + 0xF + 3U) : (0xF + 3U);

    SDK_ALIGN2_ASSERT(srcp);

    *(u32 *)dstp = size << 8 | MI_COMPRESSION_LZ | (exFormat? 1 : 0); // Data header
    dstp += 4;
    LZDstCount = 4;
    LZSrcCount = 0;
    srcStartp = srcp;
    dstMax = size;

    while (size > 0)
    {
        LZCompFlags = 0;
        LZCompFlagsp = dstp++;         // Destination for storing flag sequence
        LZDstCount++;

        // Since flag sequence is stored as 8-bit data, loop eight times
        for (i = 0; i < 8; i++)
        {
            LZCompFlags <<= 1;         // No meaning for the first time (i=0)
            if (size <= 0)
            {
                // When reached the end, quit after shifting flag to the end
                continue;
            }

            if ((lastLength = SearchLZ(srcStartp, srcp, size, &lastOffset, MAX_LENGTH)) != 0)
            {
                u32 length;
                // Enable flag if compression is possible
                LZCompFlags |= 0x1;

                if (LZDstCount + 2 >= dstMax)   // Quit on error if size becomes larger than source
                {
                    return 0;
                }

                if ( exFormat )
                {
                    if ( lastLength >= 0xFF + 0xF + 3 )
                    {
                        length  = (u32)( lastLength - 0xFF - 0xF - 3 );
                        *dstp++ = (u8)( 0x10 | (length >> 12) );
                        *dstp++ = (u8)( length >> 4 );
                        LZDstCount += 2;
                    }
                    else if ( lastLength >= 0xF + 2 )
                    {
                        length  = (u32)( lastLength - 0xF - 2 );
                        *dstp++ = (u8)( length >> 4 );
                        LZDstCount += 1;
                    }
                    else
                    {
                        length = (u32)( lastLength - 1 );
                    }
                }
                else
                {
                    length = (u32)( lastLength - 3 );
                }

                // Divide offset into upper 4 bits and lower 8 bits and store
                *dstp++ = (u8)( length << 4 | (lastOffset - 1) >> 8 );
                *dstp++ = (u8)((lastOffset - 1) & 0xff);
                LZDstCount += 2;
                srcp += lastLength;
                size -= lastLength;
            }
            else
            {
                // No compression
                if (LZDstCount + 1 >= dstMax)   // Quit on error if size becomes larger than source
                {
                    return 0;
                }
                *dstp++ = *srcp++;
                size--;
                LZDstCount++;
            }
        }                              // Completed eight loops
        *LZCompFlagsp = LZCompFlags;   // Store flag series
    }

    // 4-byte boundary alignment
    //   Does not include Data0 used for alignment in data size
    i = 0;
    while ((LZDstCount + i) & 0x3)
    {
        *dstp++ = 0;
        i++;
    }

    return LZDstCount;
}

//--------------------------------------------------------
// With LZ77 Compression, searches for the longest matching string from the slide window.
//  Arguments:    startp                 Pointer to starting position of data
//                nextp                  Pointer to data where search will start
//                remainSize             Size of remaining data
//                offset                 Pointer to region storing matched offset
//  Return:    TRUE if matching string is found
//                FALSE if not found.
//--------------------------------------------------------
static u32 SearchLZ(const u8 *startp, const u8 *nextp, u32 remainSize, u16 *offset, u32 maxLength )
{
    const u8 *searchp;
    const u8 *headp, *searchHeadp;
    u16     maxOffset;
    u32     currLength = 2;
    u32     tmpLength;

    if (remainSize < 3)
    {
        return 0;
    }

    // VRAM is accessed in units of 2 bytes (becuase sometimes data is read from VRAM), so the search must start 2 bytes prior to the search target
    //  
    // 
    // Because the offset is stored in 12 bits, the value is 4096 or less
    searchp = nextp - 4096;
    if (searchp < startp)
    {
        searchp = startp;
    }

    while (nextp - searchp >= 2)
    {
        headp = nextp;
        searchHeadp = searchp;
        tmpLength = 0;

        while ((*headp != *searchp) || (*(headp + 1) != *(searchp + 1))
               || (*(headp + 2) != *(searchp + 2)))
        {
            searchp++;
            if (nextp - searchp < 2)
            {
                goto search_end;
            }
        }
        searchHeadp = searchp + 3;
        headp += 3;
        tmpLength = 3;

        // Increments the compression size until the data ends or different data is encountered
        while (((u32)(headp - nextp) < remainSize) && (*headp == *searchHeadp))
        {
            headp++;
            searchHeadp++;
            tmpLength++;

            // Because the data length is stored in 4 bits, the value is 18 or less (3 is added)
            if (tmpLength == maxLength)
            {
                break;
            }
        }

        if (tmpLength > currLength)
        {
            // Update the maximum-length offset
            currLength = tmpLength;
            maxOffset = (u16)(nextp - searchp);
            if (currLength == maxLength || currLength == remainSize)
            {
                // This is the longest matching length, so end search
                break;
            }
        }

        searchp++;
    }

  search_end:
    if (currLength < 3)
    {
        return 0;
    }
    *offset = maxOffset;
    return currLength;
}


//===========================================================================
//  LZ encoding (high-speed version)
//===========================================================================

// Temporary information for LZ high-speed encoding
typedef struct
{
    u16     windowPos;                 // Initial position of the history window
    u16     windowLen;                 // Length of the history window

    s16    *LZOffsetTable;             // Offset buffer of the history window
    s16    *LZByteTable;               // Pointer to the most recent character history
    s16    *LZEndTable;                // Pointer to the oldest character history
}
LZCompressInfo;

static void LZInitTable(LZCompressInfo * info, u8 *work);
static void SlideByte(LZCompressInfo * info, const u8 *srcp);
static inline void LZSlide(LZCompressInfo * info, const u8 *srcp, u32 n);
static u32 SearchLZFast(LZCompressInfo * info, const u8 *nextp, u32 remainSize, u16 *offset, u32 maxLength );


//--------------------------------------------------------
// Initialize the dictionary index
//--------------------------------------------------------
static void LZInitTable(LZCompressInfo * info, u8 *work)
{
    u16     i;

    info->LZOffsetTable = (s16 *)work;
    info->LZByteTable = (s16 *)work + 4096;
    info->LZEndTable = (s16 *)work + 4096 + 256;

    for (i = 0; i < 256; i++)
    {
        info->LZByteTable[i] = -1;
        info->LZEndTable[i] = -1;
    }
    info->windowPos = 0;
    info->windowLen = 0;
}

//--------------------------------------------------------
// Slide the dictionary 1 byte
//--------------------------------------------------------
static void SlideByte(LZCompressInfo * info, const u8 *srcp)
{
    s16     offset;
    u8      in_data = *srcp;
    u16     insert_offset;

    s16    *const LZByteTable = info->LZByteTable;
    s16    *const LZOffsetTable = info->LZOffsetTable;
    s16    *const LZEndTable = info->LZEndTable;
    const u16 windowPos = info->windowPos;
    const u16 windowLen = info->windowLen;

    if (windowLen == 4096)
    {
        u8      out_data = *(srcp - 4096);
        if ((LZByteTable[out_data] = LZOffsetTable[LZByteTable[out_data]]) == -1)
        {
            LZEndTable[out_data] = -1;
        }
        insert_offset = windowPos;
    }
    else
    {
        insert_offset = windowLen;
    }

    offset = LZEndTable[in_data];
    if (offset == -1)
    {
        LZByteTable[in_data] = (s16)insert_offset;
    }
    else
    {
        LZOffsetTable[offset] = (s16)insert_offset;
    }
    LZEndTable[in_data] = (s16)insert_offset;
    LZOffsetTable[insert_offset] = -1;

    if (windowLen == 4096)
    {
        info->windowPos = (u16)((windowPos + 1) % 0x1000);
    }
    else
    {
        info->windowLen++;
    }
}

//--------------------------------------------------------
// Slide the dictionary n bytes
//--------------------------------------------------------
static inline void LZSlide(LZCompressInfo * info, const u8 *srcp, u32 n)
{
    u32     i;

    for (i = 0; i < n; i++)
    {
        SlideByte(info, srcp++);
    }
}


/*---------------------------------------------------------------------------*
  Name:         MI_CompressLZFastImpl

  Description:  Performs LZ77 compression.

  Arguments:    srcp:            Pointer to compression source data
                size:            Size of compression source data
                dstp            Pointer to compressed data
                                The buffer must be larger than the size of the compression source data.
                work            Temporary buffer for compression
                                An area of size MI_LZ_FAST_COMPRESS_WORK_SIZE is necessary.

  Returns:      The data size after compression.
                If compressed data is larger than original data, compression is terminated and 0 is returned.
 *---------------------------------------------------------------------------*/
u32 MI_CompressLZFastImpl(const u8 *srcp, u32 size, u8 *dstp, u8 *work, BOOL exFormat)
{
    u32     LZDstCount;                // Number of bytes of compressed data
    u8      LZCompFlags;               // Flag sequence indicating whether there is a compression
    u8     *LZCompFlagsp;              // Point to memory regions storing LZCompFlags
    u16     lastOffset;                // Offset to matching data (the longest matching data at the time) 
    u32     lastLength;                // Length of matching data (the longest matching data at the time)
    u8      i;
    u32     dstMax;
    LZCompressInfo info;               // Temporary LZ compression information
    const u32 MAX_LENGTH = (exFormat)? (0xFFFF + 0xFF + 0xF + 3U) : (0xF + 3U);

    SDK_ALIGN2_ASSERT(srcp);
    SDK_NULL_ASSERT(work);

    *(u32 *)dstp = size << 8 | MI_COMPRESSION_LZ | (exFormat? 1 : 0); // Data header
    dstp += 4;
    LZDstCount = 4;
    dstMax = size;
    LZInitTable(&info, work);

    while (size > 0)
    {
        LZCompFlags = 0;
        LZCompFlagsp = dstp++;         // Destination for storing flag sequence
        LZDstCount++;

        // Because flag sequence is stored as 8-bit data, loop eight times
        for (i = 0; i < 8; i++)
        {
            LZCompFlags <<= 1;         // No meaning for the first time (i=0)
            if (size <= 0)
            {
                // When reached the end, quit after shifting flag to the end
                continue;
            }

            if ( (lastLength = SearchLZFast(&info, srcp, size, &lastOffset, MAX_LENGTH)) != 0 )
            {
                u32 length;
                // Enable flag if compression is possible
                LZCompFlags |= 0x1;

                if (LZDstCount + 2 >= dstMax)   // Quit on error if size becomes larger than source
                {
                    return 0;
                }

                if ( exFormat )
                {
                    if ( lastLength >= 0xFF + 0xF + 3 )
                    {
                        length  = (u32)( lastLength - 0xFF - 0xF - 3 );
                        *dstp++ = (u8)( 0x10 | (length >> 12) );
                        *dstp++ = (u8)( length >> 4 );
                        LZDstCount += 2;
                    }
                    else if ( lastLength >= 0xF + 2 )
                    {
                        length  = (u32)( lastLength - 0xF - 2 );
                        *dstp++ = (u8)( length >> 4 );
                        LZDstCount += 1;
                    }
                    else
                    {
                        length = (u32)( lastLength - 1 );
                    }
                }
                else
                {
                    length = (u32)( lastLength - 3 );
                }

                // Divide offset into upper 4 bits and lower 8 bits and store
                *dstp++ = (u8)(length << 4 | (lastOffset - 1) >> 8);
                *dstp++ = (u8)((lastOffset - 1) & 0xff);
                LZDstCount += 2;
                LZSlide(&info, srcp, lastLength);
                srcp += lastLength;
                size -= lastLength;
            }
            else
            {
                // No compression
                if (LZDstCount + 1 >= dstMax)   // Quit on error if size becomes larger than source
                {
                    return 0;
                }
                LZSlide(&info, srcp, 1);
                *dstp++ = *srcp++;
                size--;
                LZDstCount++;
            }
        }                              // Completed eight loops
        *LZCompFlagsp = LZCompFlags;   // Store flag series
    }

    // 4-byte boundary alignment
    //   Does not include Data0 used for alignment in data size
    i = 0;
    while ((LZDstCount + i) & 0x3)
    {
        *dstp++ = 0;
        i++;
    }

    return LZDstCount;
}

//--------------------------------------------------------
// With LZ77 Compression, searches for the longest matching string from the slide window.
//  Arguments:    startp                 Pointer to starting position of data
//                nextp                  Pointer to data where search will start
//                remainSize             Size of remaining data
//                offset                 Pointer to region storing matched offset
//  Return:    TRUE if matching string is found
//                FALSE if not found.
//--------------------------------------------------------
static u32 SearchLZFast(LZCompressInfo * info, const u8 *nextp, u32 remainSize, u16 *offset, u32 maxLength )
{
    const u8 *searchp;
    const u8 *headp, *searchHeadp;
    u16     maxOffset;
    u32     currLength = 2;
    u32     tmpLength;
    s32     w_offset;
    s16    *const LZOffsetTable = info->LZOffsetTable;
    const u16 windowPos = info->windowPos;
    const u16 windowLen = info->windowLen;

    if (remainSize < 3)
    {
        return 0;
    }

    w_offset = info->LZByteTable[*nextp];

    while (w_offset != -1)
    {
        if (w_offset < windowPos)
        {
            searchp = nextp - windowPos + w_offset;
        }
        else
        {
            searchp = nextp - windowLen - windowPos + w_offset;
        }

        /* This isn't needed, but it seems to make it a little faster */
        if (*(searchp + 1) != *(nextp + 1) || *(searchp + 2) != *(nextp + 2))
        {
            w_offset = LZOffsetTable[w_offset];
            continue;
        }

        if (nextp - searchp < 2)
        {
            // VRAM is accessed in units of 2 bytes (because sometimes data is read from VRAM), so the search must start 2 bytes prior to the search target
            //  
            // 
            // Because the offset is stored in 12 bits, the value is 4096 or less
            break;
        }
        tmpLength = 3;
        searchHeadp = searchp + 3;
        headp = nextp + 3;

        // Increments the compression size until the data ends or different data is encountered
        while (((u32)(headp - nextp) < remainSize) && (*headp == *searchHeadp))
        {
            headp++;
            searchHeadp++;
            tmpLength++;

            // Because the data length is stored in 4 bits, the value is 18 or less (3 is added)
            if (tmpLength == maxLength)
            {
                break;
            }
        }

        if (tmpLength > currLength)
        {
            // Update the maximum-length offset
            currLength = tmpLength;
            maxOffset = (u16)(nextp - searchp);
            if (currLength == maxLength || currLength == remainSize)
            {
                // This is the longest matching length, so end search
                break;
            }
        }
        w_offset = LZOffsetTable[w_offset];
    }

    if (currLength < 3)
    {
        return 0;
    }
    *offset = maxOffset;
    return currLength;
}


//===========================================================================
//  Run-Length Encoding
//===========================================================================

/*---------------------------------------------------------------------------*
  Name:         MI_CompressRL

  Description:  Performs run-length compression.

  Arguments:    srcp:            Pointer to compression source data
                size:            Size of compression source data
                dstp            Pointer to compressed data
                                The buffer must be larger than the size of the compression source data.

  Returns:      The data size after compression.
                If compressed data is larger than original data, compression is terminated and 0 is returned.
 *---------------------------------------------------------------------------*/
u32 MI_CompressRL(const u8 *srcp, u32 size, u8 *dstp)
{
    u32     RLDstCount;                // Number of bytes of compressed data
    u32     RLSrcCount;                // Processed data volume of the compression target data (in bytes)
    u8      RLCompFlag;                // 1 if performing run-length encoding
    u8      runLength;                 // Run length
    u8      rawDataLength;             // Length of data not run
    u32     i;

    const u8 *startp;                  // Point to the start of compression target data for each process loop

    //  Data header        (The size after decompression)
    *(u32 *)dstp = size << 8 | MI_COMPRESSION_RL;       // Data header
    RLDstCount = 4;

    RLSrcCount = 0;
    rawDataLength = 0;
    RLCompFlag = 0;

    while (RLSrcCount < size)
    {
        startp = &srcp[RLSrcCount];    // Set compression target data

        for (i = 0; i < 128; i++)      // Data volume that can be expressed in 7 bits is 0 to 127
        {
            // Reach the end of the compression target data
            if (RLSrcCount + rawDataLength >= size)
            {
                rawDataLength = (u8)(size - RLSrcCount);
                break;
            }

            if (RLSrcCount + rawDataLength + 2 < size)
            {
                if (startp[i] == startp[i + 1] && startp[i] == startp[i + 2])
                {
                    RLCompFlag = 1;
                    break;
                }
            }
            rawDataLength++;
        }

        // Store data that will not be encoded
        // If the 8th bit of the data length storage byte is 0, the data sequence that is not encoded.
        // The data length is x - 1, so 0-127 becomes 1-128.
        if (rawDataLength)
        {
            if (RLDstCount + rawDataLength + 1 >= size) // Quit on error if size becomes larger than source
            {
                return 0;
            }
            dstp[RLDstCount++] = (u8)(rawDataLength - 1);       // Store "data length - 1" (7 bits)
            for (i = 0; i < rawDataLength; i++)
            {
                dstp[RLDstCount++] = srcp[RLSrcCount++];
            }
            rawDataLength = 0;
        }

        // Run-Length Encoding
        if (RLCompFlag)
        {
            runLength = 3;
            for (i = 3; i < 128 + 2; i++)
            {
                // Reach the end of the data for compression
                if (RLSrcCount + runLength >= size)
                {
                    runLength = (u8)(size - RLSrcCount);
                    break;
                }

                // If run is interrupted
                if (srcp[RLSrcCount] != srcp[RLSrcCount + runLength])
                {
                    break;
                }
                // Run continues
                runLength++;
            }

            // If the 8th bit of the data length storage byte is 1, the data sequence that is encoded
            if (RLDstCount + 2 >= size) // Quit on error if size becomes larger than source
            {
                return 0;
            }
            dstp[RLDstCount++] = (u8)(0x80 | (runLength - 3));  // Add 3, and store from 3 to 130
            dstp[RLDstCount++] = srcp[RLSrcCount];
            RLSrcCount += runLength;
            RLCompFlag = 0;
        }
    }

    // 4-byte boundary alignment
    //   Does not include Data0 used for alignment in data size
    i = 0;
    while ((RLDstCount + i) & 0x3)
    {
        dstp[RLDstCount + i] = 0;
        i++;
    }
    return RLDstCount;
}


//===========================================================================
//  Huffman encoding
//===========================================================================
#define HUFF_END_L  0x80
#define HUFF_END_R  0x40

static void HuffUpdateParentDepth(u16 leftNo, u16 rightNo);
static void HuffMakeCode(u16 nodeNo, u32 paHuffCode);
static u8 HuffCountHWord(u16 nodeNo);
static void HuffMakeHuffTree(u16 rootNo);
static void HuffMakeSubsetHuffTree(u16 huffTreeNo, u8 rightNodeFlag);
static u8 HuffRemainingNodeCanSetOffset(u8 costHWord);
static void HuffSetOneNodeOffset(u16 huffTreeNo, u8 rightNodeFlag);

typedef struct
{
    u32     Freq;                      // Frequency of occurrence
    u16     No;                        // Data number
    s16     PaNo;                      // Parent number 
    s16     ChNo[2];                   // Child Number (0: left side, 1: right side)
    u16     PaDepth;                   // Parent node depth
    u16     LeafDepth;                 // Depth to leaf
    u32     HuffCode;                  // Huffman code
    u8      Bit;                       // Node's bit data
    u8      _padding;
    u16     HWord;                     // For each intermediate node, the amount of memory needed to store in HuffTree the subtree that has that node as its root
}
HuffData;                              // Total of 24 bytes

static HuffData *HuffTable;            // [512] 12288B
static const HuffData HuffTableInitData = { 0, 0, 0, {-1, -1}, 0, 0, 0, 0, 0 };

static u8 HuffTreeTop;                 // The number for HuffTreeTop
static u8 *HuffTree;                   // [256][2] 512B

typedef struct
{
    u8      leftOffsetNeed;            // 1 if offset to left child node is required
    u8      rightOffsetNeed;           // 1 if an offset to the right child node is required
    u16     leftNodeNo;                // The left child node number
    u16     rightNodeNo;               // Right child node number
}
HuffTreeCtrlData;                      // Total of 6 bytes
static HuffTreeCtrlData *HuffTreeCtrl; // 1536 Byte  [ 256 ]
static const HuffTreeCtrlData HuffTreeCtrlInitData = { 1, 1, 0, 0 };

static u16 sHuffDataNum;               // The data type to encode; this is 16 for 4-bit encoding and 256 for 8-bit encoding

// Structure of the Huffman work buffer
typedef struct
{
    HuffData HuffTable[512];           //    12288B
    u8      HuffTree[256 * 2];         //      512B
    HuffTreeCtrlData HuffTreeCtrl[256]; //     1536B
}
HuffWork;                              // Total is 14336B

/*---------------------------------------------------------------------------*
  Name:         MI_CompressHuffman

  Description:  Performs Huffman compression.

  Arguments:    srcp:            Pointer to compression source data
                size:            Size of compression source data
                dstp            Pointer to compressed data
                                The buffer must be larger than the size of the compression source data.
                huffBitSize     The number of bits to encode
                work            Work buffer for Huffman compression

  Returns:      The data size after compression.
                If compressed data is larger than original data, compression is terminated and 0 is returned.
 *---------------------------------------------------------------------------*/
u32 MI_CompressHuffman(const u8 *srcp, u32 size, u8 *dstp, u8 huffBitSize, u8 *work)
{
    u32     HuffDstCount;              // Number of bytes of compressed data
    u8      tmp;
    u16     nodeNum;                   // Number of valid nodes
    u16     tableTop;                  // When creating table, the table top number
    s32     leftNo, rightNo;           // Node number for creating binary tree
    s32     i, ii, iii;
    u8      srcTmp;
    u32     bitStream = 0;
    u32     streamLength = 0;
    u16     rootNo;                    // Binary tree's root number


    SDK_NULL_ASSERT(srcp);
    SDK_NULL_ASSERT(dstp);
    SDK_ASSERT(huffBitSize == 4 || huffBitSize == 8);
    SDK_NULL_ASSERT(work);
    SDK_ALIGN4_ASSERT(work);

    HuffTable = ((HuffWork *) work)->HuffTable;
    HuffTree = ((HuffWork *) work)->HuffTree;
    HuffTreeCtrl = ((HuffWork *) work)->HuffTreeCtrl;

    sHuffDataNum = (u16)(1 << huffBitSize);     // 8->256, 4->16
    tableTop = sHuffDataNum;

    // Initialize table
    //  Subscript:   0  ~ 15(255)    : Information for the data to encode (8-bit encoding)
    //              16 ~ 31(511)    : Information for creating binary tree 
    for (i = 0; i < (u16)(sHuffDataNum * 2); i++)
    {
        HuffTable[i] = HuffTableInitData;
        HuffTable[i].No = (u16)i;
    }

    // Check frequency of occurrence
    if (huffBitSize == 8)
    {
        for (i = 0; i < size; i++)
        {
            HuffTable[srcp[i]].Freq++; // 8-bit encoding
        }
    }
    else
    {
        for (i = 0; i < size; i++)
        {                              // 4-bit encoding        
            tmp = (u8)((srcp[i] & 0xf0) >> 4);  // Store from upper 4 bits first
            HuffTable[tmp].Freq++;     // The problem is the encoding
            tmp = (u8)(srcp[i] & 0x0f);
            HuffTable[tmp].Freq++;
        }
    }

    // Create tree table
    leftNo = rightNo = -1;
    while (1)
    {
        // Search for two subtree vertices with low Freq value. At least one should be found.
        // Search child node (left)
        for (i = 0; i < tableTop; i++)
        {
            if ((HuffTable[i].Freq != 0) && (HuffTable[i].PaNo == 0))
            {
                leftNo = i;
                break;
            }
        }
        for (i = (u16)leftNo; i < tableTop; i++)
        {
            if ((HuffTable[i].Freq != 0) &&
                (HuffTable[i].PaNo == 0) && (HuffTable[i].Freq < HuffTable[leftNo].Freq))
            {
                leftNo = i;
            }
        }
        // Search child node (right)
        for (i = 0; i < tableTop; i++)
        {
            if ((HuffTable[i].Freq != 0) && (HuffTable[i].PaNo == 0) && (i != leftNo))
            {
                rightNo = i;
                break;
            }
        }
        for (i = (u16)rightNo; i < tableTop; i++)
        {
            if ((HuffTable[i].Freq != 0) &&
                (HuffTable[i].PaNo == 0) &&
                (HuffTable[i].Freq < HuffTable[rightNo].Freq) && (i != leftNo))
            {
                rightNo = i;
            }
        }
        // If only one, then end table creation
        if (rightNo < 0)
        {
            if (tableTop == sHuffDataNum)
                // When only one type of value exists or doesn't exist, then create one node that takes the same value for both 0 and 1
            {
                HuffTable[tableTop].Freq = HuffTable[leftNo].Freq;
                HuffTable[tableTop].ChNo[0] = (s16)leftNo;
                HuffTable[tableTop].ChNo[1] = (s16)leftNo;
                HuffTable[tableTop].LeafDepth = 1;
                HuffTable[leftNo].PaNo = (s16)tableTop;
                HuffTable[leftNo].Bit = 0;
                HuffTable[leftNo].PaDepth = 1;
            }
            else
            {
                tableTop--;
            }
            rootNo = tableTop;
            nodeNum = (u16)((rootNo - sHuffDataNum + 1) * 2 + 1);
            break;
        }

        // Create vertex that combines left subtree and right subtree
        HuffTable[tableTop].Freq = HuffTable[leftNo].Freq + HuffTable[rightNo].Freq;
        HuffTable[tableTop].ChNo[0] = (s16)leftNo;
        HuffTable[tableTop].ChNo[1] = (s16)rightNo;
        if (HuffTable[leftNo].LeafDepth > HuffTable[rightNo].LeafDepth)
        {
            HuffTable[tableTop].LeafDepth = (u16)(HuffTable[leftNo].LeafDepth + 1);
        }
        else
        {
            HuffTable[tableTop].LeafDepth = (u16)(HuffTable[rightNo].LeafDepth + 1);
        }

        HuffTable[leftNo].PaNo = HuffTable[rightNo].PaNo = (s16)tableTop;
        HuffTable[leftNo].Bit = 0;
        HuffTable[rightNo].Bit = 1;

        HuffUpdateParentDepth((u16)leftNo, (u16)rightNo);

        tableTop++;
        leftNo = rightNo = -1;
    }

    // Generate Huffman code (In HuffTable[i].HuffCode)
    HuffMakeCode(rootNo, 0x00);        // The Huffman code is the code with HuffCode's lower bit masked for PaDepth bits

    // For each intermediate node, calculate the amount of memory needed to store, in HuffTree, the subtree that has this node as its root
    (void)HuffCountHWord(rootNo);

    // Create HuffTree
    HuffMakeHuffTree(rootNo);
    HuffTree[0] = --HuffTreeTop;

    // Data header
    *(u32 *)dstp = size << 8 | MI_COMPRESSION_HUFFMAN | huffBitSize;
    HuffDstCount = 4;

    if (HuffDstCount + (HuffTreeTop + 1) * 2 >= size)   // Quit on error if size becomes larger than source
    {
        return 0;
    }

    for (i = 0; i < (u16)((HuffTreeTop + 1) * 2); i++)  // Tree table
    {
        dstp[HuffDstCount++] = ((u8 *)HuffTree)[i];
    }

    // 4-byte boundary alignment
    //   Data0 used for alignment is included in data size (as per the decoder algorithm)
    while (HuffDstCount & 0x3)
    {
        if (HuffDstCount & 0x1)
        {
            HuffTreeTop++;
            dstp[4]++;
        }
        dstp[HuffDstCount++] = 0;
    }

    // Huffman encoding
    for (i = 0; i < size; i++)         // Data compression
    {
        u8 val = srcp[i];
        if (huffBitSize == 8)          // 8-bit Huffman
        {
            bitStream = (bitStream << HuffTable[ val ].PaDepth) | HuffTable[ val ].HuffCode;
            streamLength += HuffTable[ val ].PaDepth;

            if (HuffDstCount + (streamLength / 8) >= size)
            {
                // Quit on error if size becomes larger than source
                return 0;
            }
            for (ii = 0; ii < streamLength / 8; ii++)
            {
                dstp[HuffDstCount++] = (u8)(bitStream >> (streamLength - (ii + 1) * 8));
            }
            streamLength %= 8;
        }
        else                           // 4-bit Huffman
        {
            for (ii = 0; ii < 2; ii++)
            {
                if (ii)
                {
                    srcTmp = (u8)( val >> 4 );        // Upper 4 bits
                }
                else
                {
                    srcTmp = (u8)( val & 0x0F );      // Lower 4 bits
                }
                bitStream = (bitStream << HuffTable[srcTmp].PaDepth) | HuffTable[srcTmp].HuffCode;
                streamLength += HuffTable[srcTmp].PaDepth;
                if (HuffDstCount + (streamLength / 8) >= size)
                {
                    // Quit on error if size becomes larger than source
                    return 0;
                }
                for (iii = 0; iii < streamLength / 8; iii++)
                {
                    dstp[HuffDstCount++] = (u8)(bitStream >> (streamLength - (iii + 1) * 8));
                }
                streamLength %= 8;
            }
        }
    }

    if (streamLength != 0)
    {
        if (HuffDstCount + 1 >= size)
        {
            // Quit on error if size becomes larger than source
            return 0;
        }
        dstp[HuffDstCount++] = (u8)(bitStream << (8 - streamLength));
    }

    // 4-byte boundary alignment
    //   Data0 for alignment is included in data size 
    //   This is special to Huffman encoding!   Data is stored after the alignment-boundary data because little endian conversion is used.
    while (HuffDstCount & 0x3)
    {
        dstp[HuffDstCount++] = 0;
    }

    for (i = 1 + (HuffTreeTop + 1) * 2 / 4; i < (HuffDstCount / 4) + 1; i++)    // Little endian conversion
    {
        tmp = dstp[i * 4 + 0];
        dstp[i * 4 + 0] = dstp[i * 4 + 3];
        dstp[i * 4 + 3] = tmp;         // Swap
        tmp = dstp[i * 4 + 1];
        dstp[i * 4 + 1] = dstp[i * 4 + 2];
        dstp[i * 4 + 2] = tmp;         // Swap
    }

    return HuffDstCount;
}



//-----------------------------------------------------------------------
// Create Huffman code table
//-----------------------------------------------------------------------
static void HuffMakeHuffTree(u16 rootNo)
{
    s16     i;
    u16     tmp;
    s16     costHWord, tmpCostHWord;   // Make subtree code table for most-costly node when subtree code table has not been created.
    s16     costOffsetNeed, tmpCostOffsetNeed;
    s16     costMaxKey, costMaxRightFlag;       // Information for singling out the least costly node from HuffTree
    u8      offsetNeedNum;
    u8      tmpKey, tmpRightFlag;

    // Initialize HuffTreeCtrl
    for (i = 0; i < 256; i++)
    {
        HuffTree[i * 2] = HuffTree[i * 2 + 1] = 0;
    }
    for (i = 0; i < 256; i++)
    {
        HuffTreeCtrl[i] = HuffTreeCtrlInitData;
    }
    HuffTreeTop = 1;
    costOffsetNeed = 0;

    HuffTreeCtrl[0].leftOffsetNeed = 0; // Do not use (used as table size)
    HuffTreeCtrl[0].rightNodeNo = rootNo;

    while (1)                          // Until return 
    {
        // Calculate the number of nodes required to set offset
        offsetNeedNum = 0;
        for (i = 0; i < HuffTreeTop; i++)
        {
            if (HuffTreeCtrl[i].leftOffsetNeed)
            {
                offsetNeedNum++;
            }
            if (HuffTreeCtrl[i].rightOffsetNeed)
            {
                offsetNeedNum++;
            }
        }

        // Search for node with greatest cost
        costHWord = -1;
        costMaxKey = -1;
        tmpKey = 0;
        tmpRightFlag = 0;

        for (i = 0; i < HuffTreeTop; i++)
        {

            tmpCostOffsetNeed = (u8)(HuffTreeTop - i);

            // Evaluate cost of left child node
            if (HuffTreeCtrl[i].leftOffsetNeed)
            {
                tmpCostHWord = (s16)HuffTable[HuffTreeCtrl[i].leftNodeNo].HWord;

                if ((tmpCostHWord + offsetNeedNum) > 64)
                {
                    goto leftCostEvaluationEnd;
                }
                if (!HuffRemainingNodeCanSetOffset((u8)tmpCostHWord))
                {
                    goto leftCostEvaluationEnd;
                }
                if (tmpCostHWord > costHWord)
                {
                    costMaxKey = i;
                    costMaxRightFlag = 0;
                }
                else if ((tmpCostHWord == costHWord) && (tmpCostOffsetNeed > costOffsetNeed))
                {
                    costMaxKey = i;
                    costMaxRightFlag = 0;
                }
            }
          leftCostEvaluationEnd:{
            }

            // Evaluate cost of right child node
            if (HuffTreeCtrl[i].rightOffsetNeed)
            {
                tmpCostHWord = (s16)HuffTable[HuffTreeCtrl[i].rightNodeNo].HWord;

                if ((tmpCostHWord + offsetNeedNum) > 64)
                {
                    goto rightCostEvaluationEnd;
                }
                if (!(HuffRemainingNodeCanSetOffset((u8)tmpCostHWord)))
                {
                    goto rightCostEvaluationEnd;
                }
                if (tmpCostHWord > costHWord)
                {
                    costMaxKey = i;
                    costMaxRightFlag = 1;
                }
                else if ((tmpCostHWord == costHWord) && (tmpCostOffsetNeed > costOffsetNeed))
                {
                    costMaxKey = i;
                    costMaxRightFlag = 1;
                }
            }
          rightCostEvaluationEnd:{
            }
        }

        // Store entire subtree in HuffTree
        if (costMaxKey >= 0)
        {
            HuffMakeSubsetHuffTree((u8)costMaxKey, (u8)costMaxRightFlag);
            goto nextTreeMaking;
        }
        else
        {
            // Search for node with largest required offset
            for (i = 0; i < HuffTreeTop; i++)
            {
                tmp = 0;
                tmpRightFlag = 0;
                if (HuffTreeCtrl[i].leftOffsetNeed)
                {
                    tmp = HuffTable[HuffTreeCtrl[i].leftNodeNo].HWord;
                }
                if (HuffTreeCtrl[i].rightOffsetNeed)
                {
                    if (HuffTable[HuffTreeCtrl[i].rightNodeNo].HWord > tmp)
                    {
                        tmpRightFlag = 1;
                    }
                }
                if ((tmp != 0) || (tmpRightFlag))
                {
                    HuffSetOneNodeOffset((u8)i, tmpRightFlag);
                    goto nextTreeMaking;
                }
            }
        }
        return;
      nextTreeMaking:{
        }
    }
}

//-----------------------------------------------------------------------
// Store entire subtree in HuffTree
//-----------------------------------------------------------------------
static void HuffMakeSubsetHuffTree(u16 huffTreeNo, u8 rightNodeFlag)
{
    u8      i;

    i = HuffTreeTop;
    HuffSetOneNodeOffset(huffTreeNo, rightNodeFlag);

    if (rightNodeFlag)
    {
        HuffTreeCtrl[huffTreeNo].rightOffsetNeed = 0;
    }
    else
    {
        HuffTreeCtrl[huffTreeNo].leftOffsetNeed = 0;
    }

    while (i < HuffTreeTop)
    {
        if (HuffTreeCtrl[i].leftOffsetNeed)
        {
            HuffSetOneNodeOffset(i, 0);
            HuffTreeCtrl[i].leftOffsetNeed = 0;
        }
        if (HuffTreeCtrl[i].rightOffsetNeed)
        {
            HuffSetOneNodeOffset(i, 1);
            HuffTreeCtrl[i].rightOffsetNeed = 0;
        }
        i++;
    }
}

//-----------------------------------------------------------------------
// Check if there are any problems with HuffTree construction even after subtree of the given data size is decompressed.
//-----------------------------------------------------------------------
static u8 HuffRemainingNodeCanSetOffset(u8 costHWord)
{
    u8      i;
    s16     capacity;

    capacity = (s16)(64 - costHWord);

    // The offset number is larger for smaller values of i, so you should calculate without sorting, with i=0 -> HuffTreeTop
    for (i = 0; i < HuffTreeTop; i++)
    {
        if (HuffTreeCtrl[i].leftOffsetNeed)
        {
            if ((HuffTreeTop - i) <= capacity)
            {
                capacity--;
            }
            else
            {
                return 0;
            }
        }
        if (HuffTreeCtrl[i].rightOffsetNeed)
        {
            if ((HuffTreeTop - i) <= capacity)
            {
                capacity--;
            }
            else
            {
                return 0;
            }
        }
    }

    return 1;
}

//-----------------------------------------------------------------------
// Create Huffman code table for one node
//-----------------------------------------------------------------------
static void HuffSetOneNodeOffset(u16 huffTreeNo, u8 rightNodeFlag)
{
    u16     nodeNo;
    u8      offsetData = 0;

    if (rightNodeFlag)
    {
        nodeNo = HuffTreeCtrl[huffTreeNo].rightNodeNo;
        HuffTreeCtrl[huffTreeNo].rightOffsetNeed = 0;
    }
    else
    {
        nodeNo = HuffTreeCtrl[huffTreeNo].leftNodeNo;
        HuffTreeCtrl[huffTreeNo].leftOffsetNeed = 0;
    }

    // Left child node
    if (HuffTable[HuffTable[nodeNo].ChNo[0]].LeafDepth == 0)
    {
        offsetData |= 0x80;
        HuffTree[HuffTreeTop * 2 + 0] = (u8)HuffTable[nodeNo].ChNo[0];
        HuffTreeCtrl[HuffTreeTop].leftNodeNo = (u8)HuffTable[nodeNo].ChNo[0];
        HuffTreeCtrl[HuffTreeTop].leftOffsetNeed = 0;   // Offset no longer required
    }
    else
    {
        HuffTreeCtrl[HuffTreeTop].leftNodeNo = (u16)HuffTable[nodeNo].ChNo[0];  // Offset is required
    }

    // Right child node
    if (HuffTable[HuffTable[nodeNo].ChNo[1]].LeafDepth == 0)
    {
        offsetData |= 0x40;
        HuffTree[HuffTreeTop * 2 + 1] = (u8)HuffTable[nodeNo].ChNo[1];
        HuffTreeCtrl[HuffTreeTop].rightNodeNo = (u8)HuffTable[nodeNo].ChNo[1];
        HuffTreeCtrl[HuffTreeTop].rightOffsetNeed = 0;  // Offset no longer required
    }
    else
    {
        HuffTreeCtrl[HuffTreeTop].rightNodeNo = (u16)HuffTable[nodeNo].ChNo[1]; // Offset is required
    }

    offsetData |= (u8)(HuffTreeTop - huffTreeNo - 1);
    HuffTree[huffTreeNo * 2 + rightNodeFlag] = offsetData;

    HuffTreeTop++;
}


//-----------------------------------------------------------------------
// When creating binary tree and when combining subtrees, add 1 to the depth of every node in the subtree
//-----------------------------------------------------------------------
static void HuffUpdateParentDepth(u16 leftNo, u16 rightNo)
{
    HuffTable[leftNo].PaDepth++;
    HuffTable[rightNo].PaDepth++;

    if (HuffTable[leftNo].LeafDepth != 0)
    {
        HuffUpdateParentDepth((u16)HuffTable[leftNo].ChNo[0], (u16)HuffTable[leftNo].ChNo[1]);
    }
    if (HuffTable[rightNo].LeafDepth != 0)
    {
        HuffUpdateParentDepth((u16)HuffTable[rightNo].ChNo[0], (u16)HuffTable[rightNo].ChNo[1]);
    }
}

//-----------------------------------------------------------------------
// Create Huffman code
//-----------------------------------------------------------------------
static void HuffMakeCode(u16 nodeNo, u32 paHuffCode)
{
    HuffTable[nodeNo].HuffCode = (paHuffCode << 1) | HuffTable[nodeNo].Bit;

    if (HuffTable[nodeNo].LeafDepth != 0)
    {
        HuffMakeCode((u16)HuffTable[nodeNo].ChNo[0], HuffTable[nodeNo].HuffCode);
        HuffMakeCode((u16)HuffTable[nodeNo].ChNo[1], HuffTable[nodeNo].HuffCode);
    }
}

//-----------------------------------------------------------------------
// Data volume required by intermediate node to create HuffTree
//-----------------------------------------------------------------------
static u8 HuffCountHWord(u16 nodeNo)
{
    u8      leftHWord, rightHWord;

    switch (HuffTable[nodeNo].LeafDepth)
    {
    case 0:
        return 0;
    case 1:
        leftHWord = rightHWord = 0;
        break;
    default:
        leftHWord = HuffCountHWord((u16)HuffTable[nodeNo].ChNo[0]);
        rightHWord = HuffCountHWord((u16)HuffTable[nodeNo].ChNo[1]);
        break;
    }

    HuffTable[nodeNo].HWord = (u16)(leftHWord + rightHWord + 1);
    return (u8)(leftHWord + rightHWord + 1);
}
