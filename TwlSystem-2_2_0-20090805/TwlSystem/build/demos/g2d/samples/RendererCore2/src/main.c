/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - RendererCore2
  File:     main.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

// ============================================================================
//  Explanation of the demo:
//      Sample of cell animation rendering using the RendererCore module
//      
//      This sample demo has almost the exact same contents as RendererCore1.
//
//      - This demos a case where the renderer core module API is used as needed while using the renderer module.
//       
//       (Since the renderer core module is used inside the renderer module, the above-mentioned operations are possible.)
//         
//  
//      Renderer module API
//       Renderer Core module API
//       Compares the performance of other renderer APIs.
//
//  Using the Demo
//      Start Button: Switches the cell output method between OBJ and Software Sprite.
//                      
//                      In the case of SoftwareSprite, drawing with MakeCellToOams is not done.
//      R Button: Switches the cell to be displayed. (Cell composed of a normal cell and a single image)
//      L Button: Changes the drawing method.
//      +Control Pad: Moves the cell. (Changes the flip status of the renderer at the same time.
//                                          This is not reflected with the MakeCellToOams function.)
//
// 
//  Because the renderer core uses specialized render functions to render cells that comprise single images for software sprite rendering, when a normal cell is rendered, it is possible to check for bugs that prevent it from being rendered correctly.
//  
//  
//  For this situation, use the R Button and try to change the display cell to a cell composed from a single image.
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"


#define SCREEN_WIDTH                256                     // Screen width
#define SCREEN_HEIGHT               192                     // Screen height

#define CELL_INIT_POS_X             (60 << FX32_SHIFT)     // Initial x position of the cell
#define CELL_INIT_POS_Y             (60 << FX32_SHIFT)     // Initial y position of the cell

#define INIT_OUTPUT_TYPE            NNS_G2D_SURFACETYPE_MAIN2D   // Initial value for output method

#define NUM_OF_OAM                  128                 // Number of OAMs allocated to OAM manager

#define CHARA_BASE                  0x0                 // Character image base address
#define PLTT_BASE                   0x0                 // Palette base address



//------------------------------------------------------------------------------
// Constants specific to rendered cells
// The number of rendered cells is equal to NUM_CELL_X * NUM_CELL_Y. When the total number of objects exceeds 128, however, and when MyDrawMethod == MYDRAWMETHOD_CELLTOOAM is not true, it must be noted that accurate performance comparisons, without processing interruptions, will no longer be possible.
// 
// 
// 
#define NUM_CELL_X 5        
#define NUM_CELL_Y 5
#define STEP_CELL_X 24
#define STEP_CELL_Y 24
  
//------------------------------------------------------------------------------
// Structure Definitions

typedef struct CellInfo
{
    NNSG2dFVec2                 scale;     // Scale
    NNSG2dFVec2                 pos;       // Position of cell rendering
    NNSG2dCellAnimation*        pCellAnim; // Cell animation
} CellInfo;

typedef enum 
{
    MYDRAWMETHOD_RENDERER,
    MYDRAWMETHOD_RENDERERCORE,
    MYDRAWMETHOD_CELLTOOAM,
    MYDRAWMETHOD_MAX

}MyDrawMethod;

const char*         drawTypeStrTbl[MYDRAWMETHOD_MAX]=
{
    "MYDRAWMETHOD_RENDERER    ",
    "MYDRAWMETHOD_RENDERERCORE",
    "MYDRAWMETHOD_CELLTOOAM   ",

};

//------------------------------------------------------------------------------
// Global Variables

static NNSG2dImageProxy             sImageProxy;    // Character/texture proxy for cell
static NNSG2dImagePaletteProxy      sPaletteProxy;  // Palette proxy for cell

static NNSG2dOamManagerInstance*    spCurrentOam;   // Manager of OAM subject to callback processing

static NNSG2dAnimBankData*          spAnimBank;     // Animation Bank
static NNSG2dCellAnimation*         spCellAnim;     // Cell animation


//------------------------------------------------------------------------------
// Prototype Declarations

void VBlankIntr(void);
static NNSG2dCellDataBank* LoadResources(void);
static void ProcessInput(CellInfo* pCellInfo, NNSG2dSurfaceType* pOutputType, u16 numCells);
static void MakeAffineMatrix(MtxFx22* pMtx, MtxFx22* pMtx2D, const CellInfo* pCellInfo);
static BOOL CallBackAddOam( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine );
static u16 CallBackAddAffine( const MtxFx22* mtx );
static void InitOamManager(NNSG2dOamManagerInstance*   pObjOamManager);
static void InitRenderer(
                NNSG2dRendererInstance* pRenderer,
                NNSG2dRenderSurface*    pSurface);


/*---------------------------------------------------------------------------*
  Name:         CallBackAddOam

  Description:  Function called internally by NNS_G2dDraw* functions to add an OAM.

  Arguments:    pOam:           Pointer to OAM to be added.
                affineIndex:    Affine index used by this OAM.
                                May have a value of 32 or greater.
                                If affine is not used,
                                specify NNS_G2D_OAM_AFFINE_IDX_NONE.
                bDoubleAffine:  Whether in double affine mode.

  Returns:      Returns TRUE if successful; returns FALSE otherwise.
 *---------------------------------------------------------------------------*/
static BOOL CallBackAddOam( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine )
{
#pragma unused( bDoubleAffine )

    SDK_NULL_ASSERT( spCurrentOam );
    SDK_NULL_ASSERT( pOam );

    return NNS_G2dEntryOamManagerOamWithAffineIdx( spCurrentOam, pOam, affineIndex );
}

/*---------------------------------------------------------------------------*
  Name:         CallBackAddAffine

  Description:  This function is called to add affine parameters in the
                NNS_G2dDraw* functions.

  Arguments:    mtx:        Pointer to affine transform matrix to be added.

  Returns:      Returns the AffineIndex for referencing added affine parameters
                
 *---------------------------------------------------------------------------*/
static u16 CallBackAddAffine( const MtxFx22* mtx )
{
    SDK_NULL_ASSERT( spCurrentOam );
    SDK_NULL_ASSERT( mtx );
    return NNS_G2dEntryOamManagerAffine( spCurrentOam, mtx );
}

/*---------------------------------------------------------------------------*
  Name:         CallBackAddAffine

  Description:  This is a function called to add an OAM inside the renderer core render function.
                NNS_G2dDraw* functions.
                Registers multiple OAMs together.
                It is called once with the timing of EndRendering.

  Arguments:    pOam:           Pointer to the OAM columns to be added.
                num:            Number of OAMs to be added.

  Returns:      Returns TRUE if successful; returns FALSE otherwise.
 *---------------------------------------------------------------------------*/
static BOOL CallBackAddOamBlock( const GXOamAttr* pOam, u16 num )
{
    return NNS_G2dEntryOamManagerOam( spCurrentOam, pOam, num );
}


/*---------------------------------------------------------------------------*
  Name:         InitOamManager

  Description:  Initializes the OAM manager system and initializes a single OAM manager instance.
                

  Arguments:    pOamManager:    Pointer to the OAM manager to be initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitOamManager(NNSG2dOamManagerInstance*   pOamManager)
{
    BOOL result;
    SDK_NULL_ASSERT( pOamManager );

    NNS_G2dInitOamManagerModule();

	result = NNS_G2dGetNewOamManagerInstanceAsFastTransferMode(
		        pOamManager, 0, NUM_OF_OAM, NNS_G2D_OAMTYPE_MAIN );
    SDK_ASSERT( result );
}

//------------------------------------------------------------------------------
static void ResetRendererSurface( NNSG2dRenderSurface*    pSurface )
{
    SDK_NULL_ASSERT( pSurface );
    {
        NNSG2dViewRect* pRect = &(pSurface->viewRect);
        
        pRect->posTopLeft.x = FX32_ONE * 0;
        pRect->posTopLeft.y = FX32_ONE * 0;
        pRect->sizeView.x = FX32_ONE * SCREEN_WIDTH;
        pRect->sizeView.y = FX32_ONE * SCREEN_HEIGHT;

        pSurface->pFuncOamRegister       = CallBackAddOam;
        pSurface->pFuncOamAffineRegister = CallBackAddAffine;
        
        pSurface->type                   = NNS_G2D_SURFACETYPE_MAIN2D;
    }
    
}

/*---------------------------------------------------------------------------*
  Name:         InitRenderer

  Description:  Initializes Renderer and Surface.

  Arguments:    pRenderer:  Pointer to Renderer to be initialized.
                pSurface:   Pointer to Surface to be initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitRenderer(
                NNSG2dRendererInstance* pRenderer,
                NNSG2dRenderSurface*    pSurface )
{
    NNSG2dViewRect* pRect = &(pSurface->viewRect);
    SDK_NULL_ASSERT( pRenderer );
    SDK_NULL_ASSERT( pSurface );

    NNS_G2dInitRenderer( pRenderer );
    
    NNS_G2dInitRenderSurface( pSurface );
    
    ResetRendererSurface( pSurface );
    
    NNS_G2dAddRendererTargetSurface( pRenderer, pSurface );
    NNS_G2dSetRendererImageProxy( pRenderer, &sImageProxy, &sPaletteProxy );
    
}


/*---------------------------------------------------------------------------*
  Name:         LoadResource

  Description:  Loads the animation data, cell bank, character data and palette data from a file, and then loads the character and palette data into VRAM.
                
                

  Arguments:    None.

  Returns:      Pointer to the loaded cell bank.
 *---------------------------------------------------------------------------*/
static NNSG2dCellDataBank* LoadResources(void)
{
    NNSG2dCellDataBank* pCellBank;
    void* pBuf;

    NNS_G2dInitImageProxy( &sImageProxy );
    NNS_G2dInitImagePaletteProxy( &sPaletteProxy );

    //------------------------------------------------------------------------------
    // Load cell data
    {
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/RendererCore.NCER" );
        SDK_NULL_ASSERT( pBuf );
        // Because the cell bank is used in main memory until the end,
        // this pBuf is not freed.
    }

    //------------------------------------------------------------------------------
    // Load animation data
    {
        // This pBuf is not deallocated because cell animation data is used in main memory until the end.
        // 
        pBuf = G2DDemo_LoadNANR( &spAnimBank, "data/RendererCore.NANR" );
        SDK_NULL_ASSERT( pBuf );

        //
        // Initialize the cell animation instance
        //
        {
            spCellAnim = G2DDemo_GetNewCellAnimation(1);
            SDK_NULL_ASSERT( spCellAnim );

            SDK_NULL_ASSERT( NNS_G2dGetAnimSequenceByIdx(spAnimBank, 0) );

            NNS_G2dInitCellAnimation(
                spCellAnim,
                NNS_G2dGetAnimSequenceByIdx(spAnimBank, 0),
                pCellBank );
        }
    }

    //------------------------------------------------------------------------------
    // Load character data for 2D
    {
        NNSG2dCharacterData* pCharData;

        pBuf = G2DDemo_LoadNCGR( &pCharData, "data/RendererCore.NCGR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading for 2D Graphics Engine
        NNS_G2dLoadImage1DMapping(
            pCharData,
            CHARA_BASE,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            &sImageProxy );

        // Because character data was copied into VRAM,
        // this pBuf is freed. Same below.
        G2DDemo_Free( pBuf );
    }

    // Load character data for 3D (software sprite)
    {
        NNSG2dCharacterData* pCharData;

        pBuf = G2DDemo_LoadNCGR( &pCharData, "data/RendererCore.NCBR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading for 3D Graphics Engine.
        NNS_G2dLoadImage1DMapping(
            pCharData,
            CHARA_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            &sImageProxy );

        G2DDemo_Free( pBuf );
    }

    //------------------------------------------------------------------------------
    // Load palette data
    {
        NNSG2dPaletteData* pPlttData;

        pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/RendererCore.NCLR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading for 2D Graphics Engine
        NNS_G2dLoadPalette(
            pPlttData,
            PLTT_BASE,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            &sPaletteProxy);

        // Loading for 3D Graphics Engine.
        NNS_G2dLoadPalette(
            pPlttData,
            PLTT_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            &sPaletteProxy);

        G2DDemo_Free( pBuf );
    }

    return pCellBank;
}

//------------------------------------------------------------------------------
static void MtxSetIdentity( MtxFx32* pM )
{
    pM->_00 = FX32_ONE;
    pM->_01 = 0;
    pM->_10 = 0;
    pM->_11 = FX32_ONE;
    pM->_20 = 0;
    pM->_21 = 0;
}

//------------------------------------------------------------------------------
// Carries out drawing using the renderer module.
static void DrawByRenderer
( 
    NNSG2dRendererInstance*     pRdr, 
    const CellInfo*             pCellInf 
)
{
    int                         i,j;
    //
    // Uses NNS_G2dBeginRenderingEx, which specifies the optimization option,
    // instead of NNS_G2dBeginRendering().
    //
    NNS_G2dBeginRenderingEx( pRdr, NNS_G2D_RDR_OPZHINT_NOT_SR | 
                                   NNS_G2D_RDR_OPZHINT_LOCK_PARAMS );
                                   
    for( i = 0; i < NUM_CELL_X; i++ )
    {
        for( j = 0; j < NUM_CELL_Y; j++ )
        {
            //
            // With this demo, Push Pop is not needed.
            // If this is the case, using code as the following allows more efficient processing.
            //
            NNS_G2dSetTrans( pCellInf->pos.x + FX32_ONE * STEP_CELL_X*i, 
                             pCellInf->pos.y + FX32_ONE * STEP_CELL_Y*j, 
                                  0 );
            NNS_G2dDrawCellAnimation( pCellInf->pCellAnim );
        }
    }
    NNS_G2dEndRendering();
}          
  


//------------------------------------------------------------------------------
// Carries out drawing using the renderer core module API.
// The renderer core module is a module that carries out the drawing processing for the renderer module.
// If the renderer core module is to be used directly, you need to carry out the matrix stack control, parameter rewrite, etc., which are normally carried out by the renderer module.
// 
static void DrawByRndCore
( 
    NNSG2dRendererInstance*     pRdr, 
    const CellInfo*             pCellInf 
)
{
    NNSG2dRndCoreInstance*    pRndCore = &pRdr->rendererCore;
    NNSG2dRenderSurface*      pSurface = pRdr->pTargetSurfaceList;
    MtxFx32                   mtx;
    int                       i,j;
    
    MtxSetIdentity( &mtx );
    
    
    //
    // Perform surface settings in render core
    //
    NNS_G2dSetRndCoreSurface( pRndCore, &pSurface->coreSurface );
    
    //
    // If rendering for a 2D surface...
    //
    if( pSurface->type != NNS_G2D_SURFACETYPE_MAIN3D )
    {
        //
        // Settings of registration function for 2D rendering
        //
        NNS_G2dSetRndCoreOamRegisterFunc  ( pRndCore, 
                                            CallBackAddOam,
                                            CallBackAddAffine );

        NNS_G2dRndCoreBeginRendering( pRndCore );
        for( i = 0; i < NUM_CELL_X; i++ )
        {
            for( j = 0; j < NUM_CELL_Y; j++ )
            {
                // Set the rendering position for the matrix set for the RendererCore.
                mtx._20 = pCellInf->pos.x + FX32_ONE * STEP_CELL_X*i;
                mtx._21 = pCellInf->pos.y + FX32_ONE * STEP_CELL_Y*j;
                //
                // Sets matrix cache in renderer.
                // (z value must be set in advance.)
                //
                NNS_G2dSetRndCoreCurrentMtx2D( &mtx, NULL );
                //
                // Perform rendering using the RenderCore function.
                //
                NNS_G2dRndCoreDrawCellFast2D( NNS_G2dGetCellAnimationCurrentCell( pCellInf->pCellAnim ) );
            }
        }
        NNS_G2dRndCoreEndRendering( );
    }else{
        //
        // Sets the z value for 3D.
        //
        NNS_G2dSetRndCore3DSoftSpriteZvalue( pRndCore, -1 );
        NNS_G2dRndCoreBeginRendering( pRndCore );
        NNS_G2dSetRndCoreCellCloneSource3D( NNS_G2dGetCellAnimationCurrentCell( pCellInf->pCellAnim ) );
        for( i = 0; i < NUM_CELL_X; i++ )
        {
            for( j = 0; j < NUM_CELL_Y; j++ )
            {
                // Set the rendering position for the matrix set for the RendererCore.
                mtx._20 = pCellInf->pos.x + FX32_ONE * STEP_CELL_X*i;
                mtx._21 = pCellInf->pos.y + FX32_ONE * STEP_CELL_Y*j;
                //
                // Sets matrix cache in renderer.
                // (z value must be set in advance.)
                //
                NNS_G2dSetRndCoreCurrentMtx3D( &mtx );
                //
                // Perform rendering using the RenderCore function.
                // Use the function to draw the identical sprites at high speeds.
                //
                NNS_G2dRndCoreDrawCellClone3D( NNS_G2dGetCellAnimationCurrentCell( pCellInf->pCellAnim ) );
            }
        }
        NNS_G2dRndCoreEndRendering( );
    }
}

//------------------------------------------------------------------------------
// Carries out drawing using NNS_G2dMakeCellToOams.
// This operates the fastest, but supplemental features are limited.
static void DrawByMakeOamsFunc
( 
    NNSG2dOamManagerInstance*   pOamMgr, 
    const CellInfo*             pCellInf 
)
{   
    static GXOamAttr          temp[NUM_OF_OAM];
    u16                       numOamBuffer = NUM_OF_OAM;                  // Number of remaining OAMs
    u16                       numDrawn;
    GXOamAttr*                pBuffer = temp;
    NNSG2dFVec2               trans;
    int                       i,j;
    
    
    
    
    for( i = 0; i < NUM_CELL_X; i++ )
    {
        for( j = 0; j < NUM_CELL_Y; j++ )
        {
            trans.x = pCellInf->pos.x + FX32_ONE * STEP_CELL_X*i;
            trans.y = pCellInf->pos.y + FX32_ONE * STEP_CELL_Y*j;
            
            if( numOamBuffer <= 0 )
            {
                return;
            }
            //
            // Writes an OBJ list equivalent to multicell to a temporary buffer.
            //
            numDrawn = NNS_G2dMakeCellToOams(
                                pBuffer,           // Output Oam buffer
                                numOamBuffer,   // Output buffer length
                                NNS_G2dGetCellAnimationCurrentCell( pCellInf->pCellAnim ), // Cell to output
                                NULL,           // Affine conversion
                                &trans,         // Offset position
                                NULL,           // Affine index
                                FALSE );        // Double affine?
            
            numOamBuffer    -= numDrawn;
            pBuffer         += numDrawn;
            
            SDK_ASSERT(numOamBuffer < NUM_OF_OAM);
        }
    }
    // Registers OBJ list to OAM Manager
    (void)NNS_G2dEntryOamManagerOam(
            pOamMgr,
            temp,
            (u16)( NUM_OF_OAM - numOamBuffer ) );
}


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    NNSG2dRendererInstance      render;         // Renderer for rendering
    NNSG2dRenderSurface         surface;        // Main screen surface
    NNSG2dCellDataBank*         pCellBank;      // Cell data
    NNSG2dOamManagerInstance    oamManager;     // OAM manager for 2D rendering
    BOOL                        bDrawByCore = FALSE;
    MyDrawMethod                drawMethod = MYDRAWMETHOD_RENDERERCORE;
    
    OSTick                      time;
    u16                         anmSeqIdx = 0;
    
    // Initializing app.
    {
        G2DDemo_CommonInit();
        G2DDemo_CameraSetup();
        G2DDemo_MaterialSetup();
        G2DDemo_PrintInit();
        InitOamManager( &oamManager );
        InitRenderer( &render, &surface );
        pCellBank = LoadResources();
        
        spCurrentOam = &oamManager;
    }

    // Start display
    {
        SVC_WaitVBlankIntr();
        GX_DispOn();
        GXS_DispOn();
    }

    // Main loop
    while( TRUE )
    {
        static CellInfo cellInfo = {
            {FX32_ONE, FX32_ONE},               // Scale
            {CELL_INIT_POS_X, CELL_INIT_POS_Y},  // Position of cell rendering
            NULL
        };

        cellInfo.pCellAnim = spCellAnim;
        //------------------------------------------------------------------------------
        // Update display information and output method for cell for which input is read
        {
            G2DDemo_ReadGamePad();
            
            // Move cell
            {
                if ( G2DDEMO_IS_PRESS(PAD_KEY_UP) )
                {
                    cellInfo.pos.y -= FX32_ONE;
                }
                if ( G2DDEMO_IS_PRESS(PAD_KEY_DOWN) )
                {
                    cellInfo.pos.y += FX32_ONE;
                }
                if ( G2DDEMO_IS_PRESS(PAD_KEY_LEFT) )
                {
                    cellInfo.pos.x -= FX32_ONE;
                    NNS_G2dSetRendererFlipMode( &render, FALSE, FALSE );
                }
                if ( G2DDEMO_IS_PRESS(PAD_KEY_RIGHT) )
                {
                    cellInfo.pos.x += FX32_ONE;
                    NNS_G2dSetRendererFlipMode( &render, TRUE, FALSE );
                }
            }

            // Reset
            if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_X) )
            {
                cellInfo.pos = (NNSG2dFVec2){CELL_INIT_POS_X, CELL_INIT_POS_Y};
            }

            // Toggle output type
            if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_START) )
            {
                surface.type = (surface.type == NNS_G2D_SURFACETYPE_MAIN2D) ?
                                    NNS_G2D_SURFACETYPE_MAIN3D:
                                    NNS_G2D_SURFACETYPE_MAIN2D;
            }
            
        
            // Drawing method change
            if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_L) )
            {
                drawMethod = (MyDrawMethod)(++drawMethod % MYDRAWMETHOD_MAX);
                //
                // Carries out initialization of the surface.
                // This is for resetting the callbacks for render core set up in the renderer module.
                //
                {
                    if( drawMethod == MYDRAWMETHOD_RENDERER )
                    {
                        NNS_G2dInitRenderSurface( &surface );
                        ResetRendererSurface( &surface );
                        
                    }else if( drawMethod == MYDRAWMETHOD_RENDERERCORE ){
                        
                        NNS_G2dInitRndCoreSurface( &surface.coreSurface );
                        ResetRendererSurface( &surface );
                    }
                }
            }
        
            //
            // Animation switch
            //
            if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_R) )
            {
                anmSeqIdx++;
                anmSeqIdx = (u16)(anmSeqIdx % NNS_G2dGetNumAnimSequence( spAnimBank ));
                
                NNS_G2dInitCellAnimation( spCellAnim, 
                                          NNS_G2dGetAnimSequenceByIdx(spAnimBank, anmSeqIdx ),
                                          pCellBank );
            }
        }
        
        
        //------------------------------------------------------------------------------
        // Render
        //
        // Render cells using various rendering functions.
        //
        time = OS_GetTick();   
        {
            switch( drawMethod )
            {
            case MYDRAWMETHOD_RENDERER:
                //
                // Rendering with renderer
                //                
                DrawByRenderer( &render, &cellInfo );break;            
            case MYDRAWMETHOD_RENDERERCORE:
                //
                // Drawing with renderer core
                //
                DrawByRndCore( &render, &cellInfo );break;            
            case MYDRAWMETHOD_CELLTOOAM:
                //
                // Rendering using NNS_G2dMakeCellToOams().
                //     Processing is not done with software sprite rendering.
                if( surface.type != NNS_G2D_SURFACETYPE_MAIN3D )
                {
                    DrawByMakeOamsFunc( &oamManager, &cellInfo );break;
                }
            }
        }
        time = OS_GetTick() - time;
        
        //------------------------------------------------------------------------------
        // Output display information
        {
            G2DDemo_PrintOutf(0, 0, "pos:   x=%3d     y=%3d",
                cellInfo.pos.x >> FX32_SHIFT, cellInfo.pos.y >> FX32_SHIFT);   
            G2DDemo_PrintOut(0, 1, drawTypeStrTbl[drawMethod] );
            G2DDemo_PrintOut(0, 2, (surface.type == NNS_G2D_SURFACETYPE_MAIN2D) ?
                   "OBJ           ": "SoftwareSprite"); 
            
            G2DDemo_PrintOutf( 0, 22, "OBJ_CNT:%04ld", 
                cellInfo.pCellAnim->pCurrentCell->numOAMAttrs * NUM_CELL_X * NUM_CELL_Y );
            G2DDemo_PrintOutf( 0, 23, "TIME   :%06ld usec\n", OS_TicksToMicroSeconds(time) );
            
            // Software sprite rendering is not carried out during NNS_G2dMakeSimpleMultiCellToOams() rendering.
            if( surface.type == NNS_G2D_SURFACETYPE_MAIN3D && drawMethod == MYDRAWMETHOD_CELLTOOAM )
            {
                G2DDemo_PrintOutf( 5, 10, "Not supported." );
            }else{
                G2DDemo_PrintOutf( 5, 10, "              " );
            }
        }

        // Wait for V-Blank
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
        SVC_WaitVBlankIntr();


        //
        // Write buffer contents to HW
        //
        {
            // Display information
            G2DDemo_PrintApplyToHW();

            // Cell
            NNS_G2dApplyOamManagerToHW( &oamManager );
            NNS_G2dResetOamManagerBuffer( &oamManager );
        }

        NNS_G2dTickCellAnimation( spCellAnim, FX32_ONE );
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Handles VBlank interrupts.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );                   // Checking VBlank interrupt
}
