/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - samples - 1mat1shp
  File:     main.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
//
// This sample displays a very simple model at high speed.
//
// Uses the NNS_G3dDraw1Mat1Shp function to rapidly render using the combination of the specified material and shape IDs.
// 
//
// Sample Operation
// Pressing the A Button will allow you to switch between NNS_G3dDraw1MatShp and NNS_G3dDraw renderings.
// 
//
// HOWTO:
// 1: Load the model resource file (.nsbmd file) into main memory.
// 2: Perform the default setup operations for a resource loaded with the NNS_G3dResDefaultSetup function.
//    
// 3: Use the NNS_G3dGetMdISet and NNS_G3dGetMdIByldx functions to get the model to use from the model resource.
//    
// 4: Light direction and color, the projection matrix, the camera matrix, the model's base SRT, etc., are configured with the NNS_G3dGlbXXXX functions.
//    
// 5: Use NNS_G3dGlbFlush to send the values set in Step 5 to the geometry engine.
// 6: Use NNS_G3dDraw1Mat1Shp to draw a simple model at high speed.
//
//---------------------------------------------------------------------------

#include "g3d_demolib.h"

G3DDemoCamera gCamera;        // Camera structure.
G3DDemoGround gGround;        // Ground structure.

static void           InitG3DLib(void);
static NNSG3dResMdl*  LoadG3DModel(const char* path);
static void           SetCamera(G3DDemoCamera* camera);


/* -------------------------------------------------------------------------
  Name:         NitroMain

  Description:  Sample main.

  Arguments:    None.

  Returns:      None.
 ------------------------------------------------------------------------- */
void
NitroMain(void)
{
    NNSG3dRenderObj object;
    NNSG3dResMdl*   pModel;
    void*           pResFile;
    BOOL            is1Mat1Shp = TRUE;

    G3DDemo_InitSystem();
    G3DDemo_InitMemory();
    G3DDemo_InitVRAM();

    InitG3DLib();
    G3DDemo_InitDisplay();

    // Make the manager able to manage 4 slots'-worth of texture image slots, and set it as the default manager.
    // 
    NNS_GfdInitFrmTexVramManager(4, TRUE);

    // Make the manager able to manage 32 KB of palettes, and set it as the default manager.
    // 
    NNS_GfdInitFrmPlttVramManager(0x8000, TRUE);

    {
        //
        // Load model resource
        //
        pResFile = G3DDemo_LoadFile("data/cube_n_t.nsbmd");
        SDK_ASSERTMSG(pResFile, "load failed");

        // To use DMA transfers within functions like NNS_G3dResDefaultSetup and NNS_G3dDraw*, all model resources have to be stored in memory before calling those functions.
        // 
        // After a certain size, for large resources it is faster to just call DC_StoreAll.
        // For information on DC_StoreRange and DC_StoreAll, refer to the NITRO-SDK Function Reference Manual.
        DC_StoreRange(pResFile, ((NNSG3dResFileHeader*)pResFile)->fileSize);

        //
        // Calls the default initialization function and performs setup.
        //
        if (!NNS_G3dResDefaultSetup(pResFile))
        {
            NNS_G3D_ASSERTMSG(0, "NNS_G3dResDefaultSetup failed");
        }

        //
        // Gets the index #0 model from the model set inside the model resource.
        //
        pModel = NNS_G3dGetMdlByIdx(NNS_G3dGetMdlSet(pResFile), 0);
        SDK_ASSERTMSG(pModel, "model not found");
    }

    //
    // G3D: NNSG3dRenderObj initialization
    //
    // This is not necessary for NNS_G3dDraw1Mat1Shp, but it is used for comparison with the normal drawing method.
    //
    NNS_G3dRenderObjInit(&object, pModel);

    G3DDemo_InitCamera(&gCamera, 0*FX32_ONE, 120*FX32_ONE);
    G3DDemo_InitGround(&gGround, (fx32)(1.5*FX32_ONE));

    G3DDemo_InitConsole();
    G3DDemo_Print(0,0, G3DDEMO_COLOR_YELLOW, "1MAT1SHP");

    for(;;)
    {
        OSTick time;

        SVC_WaitVBlankIntr();
        G3DDemo_PrintApplyToHW();
        G3DDemo_ReadGamePad();

        G3DDemo_MoveCamera(&gCamera);
        G3DDemo_CalcCamera(&gCamera);

        G3X_Reset();
        G3X_ResetMtxStack();

        SetCamera(&gCamera);

        if (G3DDEMO_IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            is1Mat1Shp = is1Mat1Shp ? FALSE : TRUE;
        }
        if (is1Mat1Shp)
        {
            G3DDemo_Printf(0,1, G3DDEMO_COLOR_GREEN, "NNS_G3dDraw1Mat1Shp");
        }
        else
        {
            G3DDemo_Printf(0,1, G3DDEMO_COLOR_GREEN, "NNS_G3dDraw        ");
        }

        {
            //
            // G3D:
            // Collects and transfers the states set with G3dGlbXXXX.
            // Camera and projection matrix, and the like, are set.
            //
            NNS_G3dGlbFlush();

            time = OS_GetTick();
            {
                //
                // G3D: NNS_G3dDraw1Mat1Shp
                //
                // Renders a material ID and shape ID specified by the user.
                // Animations are not supported.
                // If the last argument is FALSE, sending of the material is omitted.
                // 
                //  (However, it is possible to derive a material animation effect by manipulating the model resource's material.
                //  See model.h and the "ModifyNsbmd" sample.)
                //
                int i, j;
                BOOL sendMaterial = TRUE;
                VecFx32 trans = {0, 0, 0};

                //
                // Arrange 200 textured boxes.
                //
                for (i = 0; i < 20; ++i)
                {
                    trans.z = (i - 10) * FX32_ONE * 7;
                    for (j = 0; j < 10; ++j)
                    {
                        trans.x = (j - 5) * FX32_ONE * 7;

                        // You only need to set the current matrix, without using the NNS_G3dGlbFlush function, because it's a simple object.
                        // 
                        NNS_G3dGeLoadMtx43(&NNS_G3dGlb.cameraMtx);
                        NNS_G3dGeTranslateVec(&trans);

                        if (is1Mat1Shp)
                        {
                            // For the 2nd and subsequent objects, material information will not be sent to the geometry engine.
                            NNS_G3dDraw1Mat1Shp(pModel, 0, 0, sendMaterial);
                            sendMaterial = FALSE;
                        }
                        else
                        {
                            NNS_G3dDraw(&object);
                        }
                    }
                }
            }
            time = OS_GetTick() - time;
        }

        {
            VecFx32 trans = {0, 0, 0};
            NNS_G3dGlbSetBaseTrans(&trans);
            //
            // G3D: Normally, when any of the G3_XXX functions are called, you have to call the NNS_G3dGeComFlushBuffer function before that and synchronize.
            //      
            //
            NNS_G3dGlbFlush();
            NNS_G3dGeFlushBuffer();
            G3DDemo_DrawGround(&gGround);
        }

        /* Swap memory relating to geometry and rendering engine */
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

        G3DDemo_Printf(16,0, G3DDEMO_COLOR_YELLOW, "TIME:%06d usec", OS_TicksToMicroSeconds(time));
    }
}


/* -------------------------------------------------------------------------
  Name:         InitG3DLib

  Description:  Initializes G3D library.

  Arguments:    None.

  Returns:      None.
   ------------------------------------------------------------------------- */
static void
InitG3DLib(void)
{
    //
    // G3D:
    // Performs default initialization.
    // ------------------------------------------------------------------------
    NNS_G3dInit();
}


/* -------------------------------------------------------------------------
  Name:         SetCamera

  Description:  Sets camera matrix and projection matrix in G3D global state.

  Arguments:    camera:     Pointer to the G3DDemoCamera structure.

  Returns:      None.
   ------------------------------------------------------------------------- */
static void
SetCamera(G3DDemoCamera* camera)
{
    G3DDemoLookAt* lookat = &camera->lookat;
    G3DDemoPersp*  persp  = &camera->persp;

    NNS_G3dGlbPerspective(persp->fovySin, persp->fovyCos, persp->aspect, persp->nearClip, persp->farClip);
    NNS_G3dGlbLookAt(&lookat->camPos, &lookat->camUp, &lookat->target);
}
