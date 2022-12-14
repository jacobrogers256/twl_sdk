/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FX - 
  File:     fx.c

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

#include <nitro/fx/fx.h>
#include <nitro/fx/fx_const.h>
#include <nitro/fx/fx_trig.h>

//
// Workaround for no macro warnings on CW:
//  - _MSL_USE_INLINE in MSL_C\MSL_ARM\Include\math.ARM.h
//
#ifdef      __MWERKS__
#  if       !defined(_MSL_USE_INLINE) && __option(dont_inline)
#    define _MSL_USE_INLINE		0
#  endif
#endif

#include <math.h>

#define SDK_FX_INIT_2PI      (4096 * 2)
#define SDK_FX_INIT_PI_2     (4096 * 2 / 4)
#define SDK_FX_INIT_PI       (4096 * 2 / 2)


/*---------------------------------------------------------------------------*
  Name:         FX_Init

  Description:  This is an empty function if SDK_FX_DYNAMIC_TABLE is
                not defined(default). Otherwise, initializes the sin/cos
                table.

  Arguments:    none

  Returns:      none
 *---------------------------------------------------------------------------*/
// Initialize a sin/cos table.
// To avoid increasing a margin of error, angle sum and difference relations are not used extensively.
void FX_Init()
{
    SDK_ASSERT(2 == sizeof(fx16));
    SDK_ASSERT(4 == sizeof(fx32));
    SDK_ASSERT(8 == sizeof(fx64));
    SDK_ASSERT(8 == sizeof(fx64c));
    SDK_ASSERT(6 == sizeof(VecFx16));
    SDK_ASSERT(12 == sizeof(VecFx32));

    SDK_ASSERT(64 == sizeof(MtxFx44));
    SDK_ASSERT(48 == sizeof(MtxFx43));
    SDK_ASSERT(36 == sizeof(MtxFx33));
    SDK_ASSERT(16 == sizeof(MtxFx22));

#ifdef SDK_FX_DYNAMIC_TABLE
#if 1
    // FX_SinFx64c version:
    // The table constructed is not the best because 'rad' cannot be precise enough.
    // Each value may be different from the static one by 1/4096.

    fx64c   idx, ss;
    fx32    rad;
    fx16    s;
    int     i;

    for (idx = 0, i = 0; i <= 4096 / 4; idx += (FX64C_TWOPI_65536 * (65536 / 4096)), ++i)
    {
        rad = (fx32)((idx + 0x80000LL) >> 20);
        ss = FX_SinFx64c(rad);
        s = (fx16)((ss + 0x80000LL) >> 20);

        FX_SinCosTable_[i * 2] = s;    // sin: 0 to pi/2
        if (i != 0)
            FX_SinCosTable_[SDK_FX_INIT_2PI - i * 2] = (fx16)-s;        // sin: 0 to -pi/2
        FX_SinCosTable_[SDK_FX_INIT_PI - i * 2] = s;    // sin: pi/2 to pi
        FX_SinCosTable_[SDK_FX_INIT_PI + i * 2] = (fx16)-s;     // sin: -pi/2 to -pi
    }

    FX_SinCosTable_[1] = FX16_ONE;     // cos(0)

    for (i = 1; i <= SDK_FX_INIT_PI_2; ++i)
    {
        FX_SinCosTable_[i * 2 + 1] = FX_SinCosTable_[SDK_FX_INIT_PI_2 + i * 2]; // cos: cos(x) = sin(pi/2 + x)
        FX_SinCosTable_[SDK_FX_INIT_2PI - i * 2 + 1] = FX_SinCosTable_[i * 2 + 1];      // cos: cos(x) = cos(-x)
    }
#else
    // Floating point version:
    // The output is same to the static one.
    // But, note that the code is slower and larger.

    int     i;
    f32     rad;
    fx16    s;
    for (i = 0; i <= 4096 / 4; ++i)
    {
        rad = (f32)((3.14159265358979323846 * 2 / 4096) * i);
        s = (fx16)FX_F32_TO_FX32(sinf(rad));
        FX_SinCosTable_[i * 2] = s;    // sin: 0 to pi/2
        if (i != 0)
            FX_SinCosTable_[SDK_FX_INIT_2PI - i * 2] = (fx16)-s;        // sin: 0 to -pi/2
        FX_SinCosTable_[SDK_FX_INIT_PI - i * 2] = s;    // sin: pi/2 to pi
        FX_SinCosTable_[SDK_FX_INIT_PI + i * 2] = (fx16)-s;     // sin: -pi/2 to -pi
    }

    FX_SinCosTable_[1] = FX16_ONE;     // cos(0)

    for (i = 1; i <= SDK_FX_INIT_PI_2; ++i)
    {
        FX_SinCosTable_[i * 2 + 1] = FX_SinCosTable_[SDK_FX_INIT_PI_2 + i * 2]; // cos: cos(x) = sin(pi/2 + x)
        FX_SinCosTable_[SDK_FX_INIT_2PI - i * 2 + 1] = FX_SinCosTable_[i * 2 + 1];      // cos: cos(x) = cos(-x)
    }
#endif
#endif
}

#include <nitro/code32.h>              // FX_MulFunc and FX_Mul32x64cFunc are always compiled into ARM binary.
/*---------------------------------------------------------------------------*
  Name:         FX_MulFunc

  Description:  Invoked from thumb code. This drops fractions.

  Arguments:    v1           fx32 value
                v2           fx32 value

  Returns:      result in fx32 format
 *---------------------------------------------------------------------------*/
fx32 FX_MulFunc(fx32 v1, fx32 v2)
{
    return FX32_CAST(((s64)(v1) * v2 + 0x800LL) >> FX32_SHIFT);
}


/*---------------------------------------------------------------------------*
  Name:         FX_Mul32x64cFunc

  Description:  Invoked from thumb code. This rounds up/down fractions.

  Arguments:    v32          fx32 value
                v64c         fx64c value

  Returns:      result in fx32 format
 *---------------------------------------------------------------------------*/
fx32 FX_Mul32x64cFunc(fx32 v32, fx64c v64c)
{
    fx64c   tmp = v64c * v32 + 0x80000000LL;    // for better precision
    return FX32_CAST(tmp >> FX64C_SHIFT);
}

#include <nitro/codereset.h>

/*---------------------------------------------------------------------------*
  Name:         FX_Modf

  Description:  Splits a fx32 number into integral part and fraction part.

  Arguments:    x            input in fx32 format
                iPtr         a pointer to integral part in fx32 format

  Returns:      fraction part of x in fx32 format
 *---------------------------------------------------------------------------*/
fx32 FX_Modf(fx32 x, fx32 *iPtr)
{
    if (x >= 0)
    {
        *iPtr = (x & FX32_INT_MASK);
        return (x & FX32_DEC_MASK);
    }
    else
    {
        *iPtr = -((-x) & FX32_INT_MASK);
        return -((-x) & FX32_DEC_MASK);
    }
}
