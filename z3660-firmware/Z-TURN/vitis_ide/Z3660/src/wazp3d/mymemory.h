/* Wazp3D - Alain THELLIER - Paris - FRANCE 							*/
/* Adaptation to AROS from Matthias Rustler							*/
/* Code clean-up and library enhancements from Gunther Nikl					*/
/* LICENSE: GNU General Public License (GNU GPL) for this file				*/

/* This file  contain tracked memory functions 							*/

/*==================================================================================*/
void PrintME(struct memory3D *ME)
{
#ifdef WAZP3DDEBUG
UBYTE *wall;

	wall=ME->pt;wall=wall+ME->size;
	Libprintf("[ME %ld nextME %ld\t] pt=%ld \tsize=%ld \t<%s> \t[%c%c%c%c]\n ",ME,ME->nextME,ME->pt,ME->size,ME->name,wall[0],wall[1],wall[2],wall[3]);
#endif
;
}
/*==================================================================================*/
void MMDebug(ULONG level)
{
	Wazp3D->DebugMemUsage.ON=Wazp3D->DebugMemList.ON=FALSE;
	if(level==1)
		Wazp3D->DebugMemUsage.ON=TRUE;
	if(level==2)
		Wazp3D->DebugMemUsage.ON=Wazp3D->DebugMemList.ON=TRUE;
}
/*==================================================================================*/
LONG MMListMemUsage(void)
{
LONG MemoryUsage=0;
struct memory3D *ME=firstME;
LONG MEnb;

#ifdef WAZP3DDEBUG
	if(Wazp3D->DebugMemList.ON)
		Libprintf("TRACKED=MEMORY=USAGE=================)\n");
#endif
	MemoryUsage=0;
	MEnb=0;
	while(ME!=NULL)			/* for all packages in list */
	{
#ifdef WAZP3DDEBUG
	if(Wazp3D->DebugMemList.ON)
		{Libprintf("[%ld]",MEnb); PrintME(ME);}
#endif
	MemoryUsage=MemoryUsage+ME->size;
	MEnb++;
	ME=ME->nextME;
	}

#ifdef WAZP3DDEBUG
	if(Wazp3D->DebugMemUsage.ON)
		Libprintf("[%ld] MemoryUsage = %ld bytes = %ld MB\n",MEnb,MemoryUsage,MemoryUsage/(1024*1024));
#endif
	return(MemoryUsage);
}
/*==================================================================================*/
void *MMmalloc(ULONG size,char *name)
{
    struct memory3D *ME;
    UBYTE *pt;
    UBYTE *wall;


#ifdef WAZP3DDEBUG
	if(Wazp3D->DebugMemUsage.ON)
		Libprintf("Will call Libmalloc() for %ld bytes for <%s>\n",size,name);
#endif
	pt=Libmalloc(size+4+sizeof(struct memory3D));
	if (pt==NULL)
		{Libprintf("Libmalloc fail !\n");}

	ME=(struct memory3D *)pt;
	ME->pt=&pt[sizeof(struct memory3D)];
	ME->size=size;
	Libstrcpy(ME->name,name);
	ME->nextME=firstME;
	firstME=ME;

	wall=ME->pt;wall=wall+ME->size;
	wall[0]='W';		/* wall */
	wall[1]='A';
	wall[2]='L';
	wall[3]='L';	

	memset(ME->pt,0,ME->size);

#ifdef WAZP3DDEBUG
	MMListMemUsage();
	if(Wazp3D->DebugMemUsage.ON)
		Libprintf("MMmalloc() OK give pt: %ld (up to %ld) for <%s> \n",ME->pt,ME->pt+ME->size-1,ME->name);
#endif
	return(ME->pt);
}
/*==================================================================================*/
void MMfree(void *pt)
{
    struct memory3D *ME;
    struct memory3D fakeME;
    struct memory3D *thisME=&fakeME;
    UBYTE *Bpt=pt;
    UBYTE *wall;
    BOOL ok;

    if(pt==NULL) return;

#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugMemUsage.ON)
        Libprintf("Will free() memory at %ld\n",pt);
#endif
    ME =(struct memory3D *)(Bpt - sizeof(struct memory3D) );
    thisME->nextME=firstME;
    while(thisME!=NULL)
    {
        if(thisME->nextME==ME)
        {
            if(thisME->nextME==firstME)
		        firstME=ME->nextME;
            else
                thisME->nextME=ME->nextME;

            wall=ME->pt;wall=wall+ME->size;
            ok=FALSE;
            if(wall[0]=='W')
           	if(wall[1]=='A')
           	if(wall[2]=='L')
           	if(wall[3]=='L')
           	    ok=TRUE;

            if(ok)
            {
#ifdef WAZP3DDEBUG
                if(Wazp3D->DebugMemUsage.ON) Libprintf("MMfree() OK for pt: %ld was <%s>\n",ME->pt,ME->name);
#endif
                DEBUG_SOFT3D("MMfree() OK for pt: %08lx was <%s>\n",(uint32_t)ME->pt,ME->name);
            }
            else
            {
                Libprintf("MMfree() Mem. corruption for pt: %08lx was <%s>\n",(uint32_t)ME->pt,ME->name);
            }

            Libfree(ME);
            MMListMemUsage();
            return;
        }
        thisME=thisME->nextME;
    }

#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugMemUsage.ON) Libprintf("MMfree() ME %ld not found ==> not allocated pt: %ld  !!!!\n",ME,pt);
#endif
    Libprintf("MMfree() ME %08lx not found ==> not allocated pt: %08lx  !!!!\n",(uint32_t)ME,(uint32_t)pt);
}
/*==================================================================================*/
