#ifndef _TEXTFIELD_H_
#define _TEXTFIELD_H_

#define TEXTFIELD(x,y,w,h,text) do {\
	FILLRECT(   x+3, y+2, w-6, h-2, 0x00A8A8A8); \
	FILLRECT(     x,   y,   w,   1, 0x00FFFFFF); \
	FILLRECT(     x, y+1,   1, h-1, 0x00FFFFFF); \
	FILLRECT(   x+1, y+1,   1, h-2, 0x00FFFFFF); \
	FILLRECT(   x+2, y+1,   1, h-2, 0x00000000); \
	FILLRECT(   x+3, y+1,   1, h-2, 0x00000000); \
	FILLRECT(   x+3, y+1, w-4,   1, 0x00000000); \
	Font->BackColor=0x00A8A8A8; \
	Font->TextColor=0x00000000; \
	displayStringAt(Font,x+12,y+3,(uint8_t*)text,LEFT_MODE); \
	FILLRECT(   x+w,     y,   1,   h, 0x00000000); \
	FILLRECT( x+w-1,   y+1,   1, h-1, 0x00000000); \
	FILLRECT(   x+1,   y+h,   w,   1, 0x00000000); \
	FILLRECT( x+w-2,   y+3,   1, h-4, 0x00FFFFFF); \
	FILLRECT( x+w-3,   y+3,   1, h-4, 0x00FFFFFF); \
	FILLRECT(   x+3, y+h-1, w-4,   1, 0x00FFFFFF);}while(0)


#endif // _TEXTFIELD_H_
