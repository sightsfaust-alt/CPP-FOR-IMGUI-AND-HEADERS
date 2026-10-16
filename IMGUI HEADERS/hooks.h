#pragma once

#include <mod/amlmod.h>

#include "include/simpleGta.h"

inline void (*CSprite2d_DrawRect)(CRect const &posn, CRGBA const &color);
inline bool (*CSprite_CalcScreenCoors)(RwV3d const &posn, RwV3d *out, float *w,
                                       float *h, bool checkMaxVisible,
                                       bool checkMinVisible);
inline void (*FontSetOrientation)(unsigned char);
inline void (*FontSetColor)(CRGBA *);
inline void (*FontSetBackground)(unsigned char, unsigned char);
inline void (*FontSetWrapx)(float);
inline void (*FontSetStyle)(unsigned char);
inline void (*FontSetScale)(float w, float h);
inline void (*FontSetProportional)(unsigned char);
inline void (*FontSetDropShadowPosition)(char);
inline void (*FontSetEdge)(char);
inline void (*FontSetDropColor)(CRGBA *);
inline void (*FontPrintString)(float, float, unsigned short *);
inline void (*AsciiToGxtChar)(const char *txt, unsigned short *ret);
inline void (*RenderFontBuffer)(void);
inline int (*OS_ScreenGetWidth)(void);
inline int (*OS_ScreenGetHeight)(void);
inline RwRaster *(*RwRasterCreate)(RwInt32 width, RwInt32 height, RwInt32 depth,
                                   RwInt32 flags);
inline RwBool (*RwRasterDestroy)(RwRaster *raster);
inline RwRaster *(*RwRasterUnlock)(RwRaster *raster);
inline RwBool (*RwTextureSetMipmapping)(RwBool enable);
inline RwBool (*RwTextureSetAutoMipmapping)(RwBool enable);
inline RwTexture *(*RwTextureCreate)(RwRaster *raster);
inline RwUInt8 *(*RwRasterLock)(RwRaster *raster, RwUInt8 level,
                                RwInt32 lockMode);
inline RpMaterial *(*RpMaterialSetTexture)(RpMaterial *material,
                                           RwTexture *texture);
inline void (*RwImageFindRasterFormat)(RwImage *, int, int *, int *, int *,
                                       int *);
inline void (*RwImageDestroy)(RwImage *);
inline void (*RwRasterSetFromImage)(RwRaster *, RwImage *);
inline RwImage *(*RtPNGImageRead)(const char *filename);
inline RwImage *(*RtBMPImageRead)(const RwChar *imageName);
inline void (*CSprite2d_DrawSprite)(CSprite2d *, CRect const &, CRGBA const &);
inline int (*GetPedRef)(void *);
inline int (*GetVehicleRef)(void *);
inline void *(*GetPedFromRef)(int);
inline void *(*GetVehicleFromRef)(int);
inline void (*TransformRealWorldPointToRadarSpace)(CVector2D &out,
                                                   CVector2D const &in);
inline void (*TransformRadarPointToScreenSpace)(CVector2D &out,
                                                CVector2D const &in);
inline void (*LimitRadarPoint)(CVector2D &in);

inline bool *userPaused;
inline bool *codePaused;
inline CCamera *camera;
inline float flMenuMapScaling;
inline bool IsGamePaused() { return *userPaused || *codePaused; };
inline RsGlobalType *RsGlobal;
inline unsigned int *m_snTimeInMilliseconds;
inline bool *CTouchInterface_m_bTouchDown;
inline CVector2D *m_vecCachedPos;

inline uintptr_t *pPedPool;
inline uintptr_t *pVehiclePool;

void DoHooks();
void DoLateHooks();