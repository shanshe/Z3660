#define STRINGIZER_(exp) #exp
#define STRINGIZER(exp) STRINGIZER_(exp)

char *soft3d_op_string[]={
   STRINGIZER(OP_SETBITMAP         ),
   STRINGIZER(OP_SETCLIPPING       ),
   STRINGIZER(OP_SETDRAWSTATE      ),
   STRINGIZER(OP_DRAWPRIMITIVE     ),
   STRINGIZER(OP_DOUPDATE          ),
   STRINGIZER(OP_FLUSH             ),
   STRINGIZER(OP_END               ),
   STRINGIZER(OP_CREATETEXTURE     ),
   STRINGIZER(OP_FREETEXTURE       ),
   STRINGIZER(OP_UPDATETEXTURE     ),
   STRINGIZER(OP_START             ),
   STRINGIZER(OP_ALLOCZBUFFER      ),
   STRINGIZER(OP_ALLOCIMAGEBUFFER  ),
   STRINGIZER(OP_CLEARZBUFFER      ),
   STRINGIZER(OP_READZSPAN         ),
   STRINGIZER(OP_WRITEZSPAN        ),
   STRINGIZER(OP_SOFT3D_NUM        ),
};