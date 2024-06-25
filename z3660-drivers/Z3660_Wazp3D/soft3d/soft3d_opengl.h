/* Wazp3D Beta 56 : Alain THELLIER - Paris - FRANCE - (November 2006 to 2014) 	*/
/* Code clean-up and library enhancements from Gunther Nikl 				*/
/* Adaptation to AROS from Matthias Rustler							*/
/* Adaptation to Morphos from Szil�rd 'BSzili' Bir� 						*/
/* LICENSE: GNU General Public License (GNU GPL) for this file				*/


/* OpenGL-wrapper: functions definitions								*/

void HARD3D_AllocImageBuffer(void *hc,unsigned short large,unsigned short high);
void HARD3D_AllocZbuffer(void *hc,unsigned short large,unsigned short high);
void HARD3D_ClearImageBuffer(void *hc,unsigned short x,unsigned short y,unsigned short large,unsigned short high,void *rgba);
void HARD3D_ClearZBuffer(void *hc,float fz);
void HARD3D_CreateTexture(void *hc,void *ht,unsigned char *pt,unsigned short large,unsigned short high,unsigned short bits,unsigned char TexFlags);
void HARD3D_DoUpdate(void *hc);
void HARD3D_DrawPrimitive(void *hc,void *P,unsigned long Pnb,unsigned long primitive);
void HARD3D_End(void *hc);
void HARD3D_Flush(void *hc);
void HARD3D_FreeTexture(void *hc,void *ht);
void HARD3D_ReadZSpan(void *hc, unsigned short x, unsigned short y,unsigned long n, double *dz);
void HARD3D_SetBitmap(void *hc,void *bm,unsigned char *Image8,unsigned long format,unsigned short x,unsigned short y,unsigned short large,unsigned short high);
void HARD3D_SetClipping(void *hc,unsigned short xmin,unsigned short xmax,unsigned short ymin,unsigned short ymax);
void HARD3D_Start(void *hc);
void HARD3D_UpdateTexture(void *hc,void *ht,unsigned char *pt,unsigned short large,unsigned short high,unsigned char bits);
void HARD3D_WriteZSpan(void *hc, unsigned short x, unsigned short y,unsigned long n, double *dz, unsigned char *mask);
/*=============================================================*/
void HARD3D_Current(void *hc);
void HARD3D_SetDrawFunctions(void *hc);
/*=============================================================*/
#define MAXPRIM (3*1000)		/* Maximum points per primitive		*/
#define MAXSCREEN  2560			/* Maximum screen size 2048x2048		*/
/*=============================================================*/
struct HARD3D_context{
	void *hdc;				/* handle to Display Context  */
	void *hglrc;			/* handle to GL Rendering Context */
	void *instance;			/* handle to window-instance */
	void *glwnd;			/* handle to window for GL */
	void *hardbm;			/* off-screen Windows' bitmap / off-screen Morphos' bitmap*/
	void *RGBA32;			/* off-screen Windows' bitmap data */
	void *Image8;
	void *awin;				/* Amiga's current window at startup */
	void *overwin;			/* Amiga's window created for Aros-Mesa overlay */
	void *hackrastport;		/* Copy of current window->rport */
	unsigned short large,high;			/* to convert origin to lower-left corner */
	unsigned char glstarted;			/* OpenGL ready ? */
	unsigned char registered;			/* window-instance registered ?*/
	unsigned char UseAntiAlias;			/* enable POLYGON/POINT/LINE_SMOOTH  */
	unsigned char UseDoubleBuffer;		/* use OS_SwapBuffers else not    */
	unsigned char UseOverlay;			/* create a sub-window("overlay") else use WinUAE's window for GL context */
	unsigned char DebugHard;			/* for debugging */
	struct state3D *state;				/* direct pointer to SC->state */
	float fzspan[MAXSCREEN];			/* for read/write zspan */
	float      V[5*MAXPRIM];			/* GL: for perspectived texture-coordinates. Compositing:  x y s t w  coordinates */
	};
/*=============================================================*/
struct HARD3D_texture{
	unsigned long gltex;	/* GL texture */
	unsigned char *ptmm;
	unsigned char TexFlags,padding1,padding2,padding3;
	};
/*=============================================================*/
