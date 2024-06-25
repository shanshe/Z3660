#include <Warp3D/Warp3D.h>        /* definitions of WORD ,UBYTE, etc... */

ASM(W3D_Context*)    LibW3D_CreateContext       (A0(ULONG* error), A1(struct TagItem* CCTags)) {return (W3D_Context*)W3D_CreateContext(error,CCTags);} 
ASM(void)            LibW3D_DestroyContext      (A0(W3D_Context* context)) {W3D_DestroyContext(context);} 
ASM(ULONG)           LibW3D_GetState            (A0(W3D_Context* context), D1(ULONG state)) {return W3D_GetState(context,state);} 
ASM(ULONG)           LibW3D_SetState            (A0(W3D_Context* context), D0(ULONG state), D1(ULONG action)) {return W3D_SetState(context,state,action);} 
ASM(ULONG)           LibW3D_CheckDriver         () {return W3D_CheckDriver();} 
ASM(ULONG)           LibW3D_LockHardware        (A0(W3D_Context* context)) {return W3D_LockHardware(context);} 
ASM(void)            LibW3D_UnLockHardware      (A0(W3D_Context* context)) {W3D_UnLockHardware(context);} 
ASM(void)            LibW3D_WaitIdle            (A0(W3D_Context* context)) {W3D_WaitIdle(context);} 
ASM(ULONG)           LibW3D_CheckIdle           (A0(W3D_Context* context)) {return W3D_CheckIdle(context);} 
ASM(ULONG)           LibW3D_Query               (A0(W3D_Context* context), D0(ULONG query), D1(ULONG destfmt)) {return W3D_Query(context,query,destfmt);} 
ASM(ULONG)           LibW3D_GetTexFmtInfo       (A0(W3D_Context* context), D0(ULONG texfmt), D1(ULONG destfmt)) {return W3D_GetTexFmtInfo(context,texfmt,destfmt);} 
ASM(W3D_Texture*)    LibW3D_AllocTexObj         (A0(W3D_Context* context), A1(ULONG* error), A2(struct TagItem* ATOTags)) {return (W3D_Texture*)W3D_AllocTexObj(context,error,ATOTags);} 
ASM(void)            LibW3D_FreeTexObj          (A0(W3D_Context* context), A1(W3D_Texture* texture)) {W3D_FreeTexObj(context,texture);} 
ASM(void)            LibW3D_ReleaseTexture      (A0(W3D_Context* context), A1(W3D_Texture* texture)) {W3D_ReleaseTexture(context,texture);} 
ASM(void)            LibW3D_FlushTextures       (A0(W3D_Context* context)) {W3D_FlushTextures(context);} 
ASM(ULONG)           LibW3D_SetFilter           (A0(W3D_Context* context), A1(W3D_Texture* texture), D0(ULONG MinFilter), D1(ULONG MagFilter)) {return W3D_SetFilter(context,texture,MinFilter,MagFilter);} 
ASM(ULONG)           LibW3D_SetTexEnv           (A0(W3D_Context* context), A1(W3D_Texture* texture), D1(ULONG envparam), A2(W3D_Color* envcolor)) {return W3D_SetTexEnv(context,texture,envparam,envcolor);} 
ASM(ULONG)           LibW3D_SetWrapMode         (A0(W3D_Context* context), A1(W3D_Texture* texture), D0(ULONG s_mode), D1(ULONG t_mode), A2(W3D_Color* bordercolor)) {return W3D_SetWrapMode(context,texture,s_mode,t_mode,bordercolor);} 
ASM(ULONG)           LibW3D_UpdateTexImage      (A0(W3D_Context* context), A1(W3D_Texture* texture), A2(void* teximage), D1(int level), A3(ULONG* palette)) {return W3D_UpdateTexImage(context,texture,teximage,level,palette);} 
ASM(ULONG)           LibW3D_UploadTexture       (A0(W3D_Context* context), A1(W3D_Texture* texture)) {return W3D_UploadTexture(context,texture);} 
ASM(ULONG)           LibW3D_DrawLine            (A0(W3D_Context* context), A1(W3D_Line* line)) {return W3D_DrawLine(context,line);} 
ASM(ULONG)           LibW3D_DrawPoint           (A0(W3D_Context* context), A1(W3D_Point* point)) {return W3D_DrawPoint(context,point);} 
ASM(ULONG)           LibW3D_DrawTriangle        (A0(W3D_Context* context), A1(W3D_Triangle* triangle)) {return W3D_DrawTriangle(context,triangle);} 
ASM(ULONG)           LibW3D_DrawTriFan          (A0(W3D_Context* context), A1(W3D_Triangles* triangles)) {return W3D_DrawTriFan(context,triangles);} 
ASM(ULONG)           LibW3D_DrawTriStrip        (A0(W3D_Context* context), A1(W3D_Triangles* triangles)) {return W3D_DrawTriStrip(context,triangles);} 
ASM(ULONG)           LibW3D_SetAlphaMode        (A0(W3D_Context* context), D1(ULONG mode), A1(W3D_Float* refval)) {return W3D_SetAlphaMode(context,mode,refval);} 
ASM(ULONG)           LibW3D_SetBlendMode        (A0(W3D_Context* context), D0(ULONG srcfunc), D1(ULONG dstfunc)) {return W3D_SetBlendMode(context,srcfunc,dstfunc);} 
ASM(ULONG)           LibW3D_SetDrawRegion       (A0(W3D_Context* context), A1(struct BitMap* bm), D1(int yoffset), A2(W3D_Scissor* scissor)) {return W3D_SetDrawRegion(context,bm,yoffset,scissor);} 
ASM(ULONG)           LibW3D_SetFogParams        (A0(W3D_Context* context), A1(W3D_Fog* fogparams), D1(ULONG fogmode)) {return W3D_SetFogParams(context,fogparams,fogmode);} 
ASM(ULONG)           LibW3D_SetColorMask        (A0(W3D_Context* context), D0(W3D_Bool red), D1(W3D_Bool green), D2(W3D_Bool blue), D3(W3D_Bool alpha)) {return W3D_SetColorMask(context,red,green,blue,alpha);} 
ASM(ULONG)           LibW3D_SetStencilFunc      (A0(W3D_Context* context), D0(ULONG func), D1(ULONG refvalue), D2(ULONG mask)) {return W3D_SetStencilFunc(context,func,refvalue,mask);} 
ASM(ULONG)           LibW3D_AllocZBuffer        (A0(W3D_Context* context)) {return W3D_AllocZBuffer(context);} 
ASM(ULONG)           LibW3D_FreeZBuffer         (A0(W3D_Context* context)) {return W3D_FreeZBuffer(context);} 
ASM(ULONG)           LibW3D_ClearZBuffer        (A0(W3D_Context* context), A1(W3D_Double* clearvalue)) {return W3D_ClearZBuffer(context,clearvalue);} 
ASM(ULONG)           LibW3D_ReadZPixel          (A0(W3D_Context* context), D0(ULONG x), D1(ULONG y), A1(W3D_Double* z)) {return W3D_ReadZPixel(context,x,y,z);} 
ASM(ULONG)           LibW3D_ReadZSpan           (A0(W3D_Context* context), D0(ULONG x), D1(ULONG y), D2(ULONG n), A1(W3D_Double* z)) {return W3D_ReadZSpan(context,x,y,n,z);} 
ASM(ULONG)           LibW3D_SetZCompareMode     (A0(W3D_Context* context), D1(ULONG mode)) {return W3D_SetZCompareMode(context,mode);} 
ASM(ULONG)           LibW3D_AllocStencilBuffer  (A0(W3D_Context* context)) {return W3D_AllocStencilBuffer(context);} 
ASM(ULONG)           LibW3D_ClearStencilBuffer  (A0(W3D_Context* context), A1(ULONG* clearvalue)) {return W3D_ClearStencilBuffer(context,clearvalue);} 
ASM(ULONG)           LibW3D_FillStencilBuffer   (A0(W3D_Context* context), D0(ULONG x), D1(ULONG y), D2(ULONG width), D3(ULONG height), D4(ULONG depth), A1(void* data)) {return W3D_FillStencilBuffer(context,x,y,width,height,depth,data);} 
ASM(ULONG)           LibW3D_FreeStencilBuffer   (A0(W3D_Context* context)) {return W3D_FreeStencilBuffer(context);} 
ASM(ULONG)           LibW3D_ReadStencilPixel    (A0(W3D_Context* context), D0(ULONG x), D1(ULONG y), A1(ULONG* st)) {return W3D_ReadStencilPixel(context,x,y,st);} 
ASM(ULONG)           LibW3D_ReadStencilSpan     (A0(W3D_Context* context), D0(ULONG x), D1(ULONG y), D2(ULONG n), A1(ULONG* st)) {return W3D_ReadStencilSpan(context,x,y,n,st);} 
ASM(ULONG)           LibW3D_SetLogicOp          (A0(W3D_Context* context), D1(ULONG operation)) {return W3D_SetLogicOp(context,operation);} 
ASM(ULONG)           LibW3D_Hint                (A0(W3D_Context* context), D0(ULONG mode), D1(ULONG quality)) {return W3D_Hint(context,mode,quality);} 
ASM(ULONG)           LibW3D_SetDrawRegionWBM    (A0(W3D_Context* context), A1(W3D_Bitmap* bm), A2(W3D_Scissor* scissor)) {return W3D_SetDrawRegionWBM(context,bm,scissor);} 
ASM(ULONG)           LibW3D_GetDriverState      (A0(W3D_Context* context)) {return W3D_GetDriverState(context);} 
ASM(ULONG)           LibW3D_Flush               (A0(W3D_Context* context)) {return W3D_Flush(context);} 
ASM(ULONG)           LibW3D_SetPenMask          (A0(W3D_Context* context), D1(ULONG pen)) {return W3D_SetPenMask(context,pen);} 
ASM(ULONG)           LibW3D_SetStencilOp        (A0(W3D_Context* context), D0(ULONG sfail), D1(ULONG dpfail), D2(ULONG dppass)) {return W3D_SetStencilOp(context,sfail,dpfail,dppass);} 
ASM(ULONG)           LibW3D_SetWriteMask        (A0(W3D_Context* context), D1(ULONG mask)) {return W3D_SetWriteMask(context,mask);} 
ASM(ULONG)           LibW3D_WriteStencilPixel   (A0(W3D_Context* context), D0(ULONG x), D1(ULONG y), D2(ULONG st)) {return W3D_WriteStencilPixel(context,x,y,st);} 
ASM(ULONG)           LibW3D_WriteStencilSpan    (A0(W3D_Context* context), D0(ULONG x), D1(ULONG y), D2(ULONG n), A1(ULONG* st), A2(UBYTE* mask)) {return W3D_WriteStencilSpan(context,x,y,n,st,mask);} 
ASM(void)            LibW3D_WriteZPixel         (A0(W3D_Context* context), D0(ULONG x), D1(ULONG y), A1(W3D_Double* z)) {W3D_WriteZPixel(context,x,y,z);} 
ASM(void)            LibW3D_WriteZSpan          (A0(W3D_Context* context), D0(ULONG x), D1(ULONG y), D2(ULONG n), A1(W3D_Double* z), A2(UBYTE* mask)) {W3D_WriteZSpan(context,x,y,n,z,mask);} 
ASM(ULONG)           LibW3D_SetCurrentColor     (A0(W3D_Context* context), A1(W3D_Color* color)) {return W3D_SetCurrentColor(context,color);} 
ASM(ULONG)           LibW3D_SetCurrentPen       (A0(W3D_Context* context), D1(ULONG pen)) {return W3D_SetCurrentPen(context,pen);} 
ASM(ULONG)           LibW3D_UpdateTexSubImage   (A0(W3D_Context* context), A1(W3D_Texture* texture), A2(void* teximage), D1(ULONG level), A3(ULONG* palette), A4(W3D_Scissor* scissor), D0(ULONG srcbpr)) {return W3D_UpdateTexSubImage(context,texture,teximage,level,palette,scissor,srcbpr);} 
ASM(ULONG)           LibW3D_FreeAllTexObj       (A0(W3D_Context* context)) {return W3D_FreeAllTexObj(context);} 
ASM(ULONG)           LibW3D_GetDestFmt          () {return W3D_GetDestFmt();} 
ASM(ULONG)           LibW3D_DrawLineStrip       (A0(W3D_Context* context), A1(W3D_Lines* lines)) {return W3D_DrawLineStrip(context,lines);} 
ASM(ULONG)           LibW3D_DrawLineLoop        (A0(W3D_Context* context), A1(W3D_Lines* lines)) {return W3D_DrawLineLoop(context,lines);} 
ASM(W3D_Driver**)    LibW3D_GetDrivers          () {return (W3D_Driver**)W3D_GetDrivers();} 
ASM(ULONG)           LibW3D_QueryDriver         (A0(W3D_Driver* driver), D0(ULONG query), D1(ULONG destfmt)) {return W3D_QueryDriver(driver,query,destfmt);} 
ASM(ULONG)           LibW3D_GetDriverTexFmtInfo (A0(W3D_Driver* driver), D0(ULONG query), D1(ULONG destfmt)) {return W3D_GetDriverTexFmtInfo(driver,query,destfmt);} 
ASM(ULONG)           LibW3D_RequestMode         (A0(struct TagItem* taglist)) {return W3D_RequestMode(taglist);} 
ASM(void)            LibW3D_SetScissor          (A0(W3D_Context* context), A1(W3D_Scissor* scissor)) {W3D_SetScissor(context,scissor);} 
ASM(void)            LibW3D_FlushFrame          (A0(W3D_Context* context)) {W3D_FlushFrame(context);} 
ASM(W3D_Driver*)     LibW3D_TestMode            (D0(ULONG ModeID)) {return (W3D_Driver*) W3D_TestMode(ModeID);} 
ASM(ULONG)           LibW3D_SetChromaTestBounds (A0(W3D_Context* context), A1(W3D_Texture* texture), D0(ULONG rgba_lower), D1(ULONG rgba_upper), D2(ULONG mode)) {return W3D_SetChromaTestBounds(context,texture,rgba_lower,rgba_upper,mode);} 
ASM(ULONG)           LibW3D_ClearDrawRegion     (A0(W3D_Context* context), D0(ULONG color)) {return W3D_ClearDrawRegion(context,color);} 
ASM(ULONG)           LibW3D_DrawTriangleV       (A0(W3D_Context* context), A1(W3D_TriangleV* triangle)) {return W3D_DrawTriangleV(context,triangle);} 
ASM(ULONG)           LibW3D_DrawTriFanV         (A0(W3D_Context* context), A1(W3D_TrianglesV* triangles)) {return W3D_DrawTriFanV(context,triangles);} 
ASM(ULONG)           LibW3D_DrawTriStripV       (A0(W3D_Context* context), A1(W3D_TrianglesV* triangles)) {return W3D_DrawTriStripV(context,triangles);} 
ASM(W3D_ScreenMode*) LibW3D_GetScreenmodeList   () {return (W3D_ScreenMode*) W3D_GetScreenmodeList();} 
ASM(void)            LibW3D_FreeScreenmodeList  (A0(W3D_ScreenMode* list)) {W3D_FreeScreenmodeList(list);} 
ASM(ULONG)           LibW3D_BestModeID          (A0(struct TagItem* tags)) {return W3D_BestModeID(tags);} 
ASM(ULONG)           LibW3D_VertexPointer       (A0(W3D_Context* context), A1(void* pointer), D0(int stride), D1(ULONG mode), D2(ULONG flags)) {return W3D_VertexPointer(context,pointer,stride,mode,flags);} 
ASM(ULONG)           LibW3D_TexCoordPointer     (A0(W3D_Context* context), A1(void* pointer), D0(int stride), D1(int unit), D2(int off_v), D3(int off_w), D4(ULONG flags)) {return W3D_TexCoordPointer(context,pointer,stride,unit,off_v,off_w,flags);} 
ASM(ULONG)           LibW3D_ColorPointer        (A0(W3D_Context* context), A1(void* pointer), D0(int stride), D1(ULONG format), D2(ULONG mode), D3(ULONG flags)) {return W3D_ColorPointer(context,pointer,stride,format,mode,flags);} 
ASM(ULONG)           LibW3D_BindTexture         (A0(W3D_Context* context), D0(ULONG tmu), A1(W3D_Texture* texture)) {return W3D_BindTexture(context,tmu,texture);} 
ASM(ULONG)           LibW3D_DrawArray           (A0(W3D_Context* context), D0(ULONG primitive), D1(ULONG base), D2(ULONG count)) {return W3D_DrawArray(context,primitive,base,count);} 
ASM(ULONG)           LibW3D_DrawElements        (A0(W3D_Context* context), D0(ULONG primitive), D1(ULONG type), D2(ULONG count), A1(void* indices)) {return W3D_DrawElements(context,primitive,type,count,indices);} 
ASM(void)            LibW3D_SetFrontFace        (A0(W3D_Context* context), D0(ULONG direction)) {W3D_SetFrontFace(context,direction);} 

static const APTR LibVectors[] = {
	(APTR) LibOpen,
	(APTR) LibClose,
	(APTR) LibExpunge,
	(APTR) LibReserved,

	(APTR) LibW3D_CreateContext,
	(APTR) LibW3D_DestroyContext,
	(APTR) LibW3D_GetState,
	(APTR) LibW3D_SetState,

	(APTR) LibW3D_CheckDriver,
	(APTR) LibW3D_LockHardware,
	(APTR) LibW3D_UnLockHardware,
	(APTR) LibW3D_WaitIdle,
	(APTR) LibW3D_CheckIdle,
	(APTR) LibW3D_Query,
	(APTR) LibW3D_GetTexFmtInfo,

	(APTR) LibW3D_AllocTexObj,
	(APTR) LibW3D_FreeTexObj,
	(APTR) LibW3D_ReleaseTexture,
	(APTR) LibW3D_FlushTextures,
	(APTR) LibW3D_SetFilter,
	(APTR) LibW3D_SetTexEnv,
	(APTR) LibW3D_SetWrapMode,
	(APTR) LibW3D_UpdateTexImage,
	(APTR) LibW3D_UploadTexture,

	(APTR) LibW3D_DrawLine,
	(APTR) LibW3D_DrawPoint,
	(APTR) LibW3D_DrawTriangle,
	(APTR) LibW3D_DrawTriFan,
	(APTR) LibW3D_DrawTriStrip,

	(APTR) LibW3D_SetAlphaMode,
	(APTR) LibW3D_SetBlendMode,
	(APTR) LibW3D_SetDrawRegion,
	(APTR) LibW3D_SetFogParams,
	(APTR) LibW3D_SetColorMask,
	(APTR) LibW3D_SetStencilFunc,

	(APTR) LibW3D_AllocZBuffer,
	(APTR) LibW3D_FreeZBuffer,
	(APTR) LibW3D_ClearZBuffer,
	(APTR) LibW3D_ReadZPixel,
	(APTR) LibW3D_ReadZSpan,
	(APTR) LibW3D_SetZCompareMode,

	(APTR) LibW3D_AllocStencilBuffer,
	(APTR) LibW3D_ClearStencilBuffer,
	(APTR) LibW3D_FillStencilBuffer,
	(APTR) LibW3D_FreeStencilBuffer,
	(APTR) LibW3D_ReadStencilPixel,
	(APTR) LibW3D_ReadStencilSpan,

	(APTR) LibW3D_SetLogicOp,
	(APTR) LibW3D_Hint,
	(APTR) LibW3D_SetDrawRegionWBM,
	(APTR) LibW3D_GetDriverState,
	(APTR) LibW3D_Flush,
	(APTR) LibW3D_SetPenMask,
	(APTR) LibW3D_SetStencilOp,
	(APTR) LibW3D_SetWriteMask,
	(APTR) LibW3D_WriteStencilPixel,
	(APTR) LibW3D_WriteStencilSpan,
	(APTR) LibW3D_WriteZPixel,
	(APTR) LibW3D_WriteZSpan,
	(APTR) LibW3D_SetCurrentColor,
	(APTR) LibW3D_SetCurrentPen,
	(APTR) LibW3D_UpdateTexSubImage,
	(APTR) LibW3D_FreeAllTexObj,
	(APTR) LibW3D_GetDestFmt,

	(APTR) LibW3D_DrawLineStrip,
	(APTR) LibW3D_DrawLineLoop,
	(APTR) LibW3D_GetDrivers,
	(APTR) LibW3D_QueryDriver,
	(APTR) LibW3D_GetDriverTexFmtInfo,
	(APTR) LibW3D_RequestMode,
	(APTR) LibW3D_SetScissor,
	(APTR) LibW3D_FlushFrame,
	(APTR) LibW3D_TestMode,
	(APTR) LibW3D_SetChromaTestBounds,
	(APTR) LibW3D_ClearDrawRegion,

	(APTR) LibW3D_DrawTriangleV,
	(APTR) LibW3D_DrawTriFanV,
	(APTR) LibW3D_DrawTriStripV,
	(APTR) LibW3D_GetScreenmodeList,
	(APTR) LibW3D_FreeScreenmodeList,
	(APTR) LibW3D_BestModeID,

	(APTR) LibW3D_VertexPointer,
	(APTR) LibW3D_TexCoordPointer,
	(APTR) LibW3D_ColorPointer,
	(APTR) LibW3D_BindTexture,
	(APTR) LibW3D_DrawArray,
	(APTR) LibW3D_DrawElements,
	(APTR) LibW3D_SetFrontFace,
	(APTR) -1
};
