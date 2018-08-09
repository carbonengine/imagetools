////////////////////////////////////////////////////////////
//
//    Creator:   Filipp Pavlov
//    Created:   March 2014
//    Copyright: CCP 2014
//

#define NOMINMAX

#ifdef _WIN32
#define WITH_COMPRESSONATOR 1
#else
#define WITH_COMPRESSONATOR 0
#endif

#include "BlueExposure/include/BlueExposure.h"
#include "nvtt/nvtt.h"

#if WITH_COMPRESSONATOR
#include "Compressonator.h"
#endif

#include "TrinityAL/include/Tr2PixelFormat.h"
#include "TrinityAL/include/Tr2TextureType.h"
#include "TrinityAL/include/Tr2BitmapDimensions.h"
#include "TrinityAL/include/Tr2CubemapFace.h"

#include "ImageIO/HostBitmap.h"
#include "ImageIO/Tr2ImageHandler.h"