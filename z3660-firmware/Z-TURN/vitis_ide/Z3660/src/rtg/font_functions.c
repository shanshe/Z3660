#include "fonts.h"
#include "../video.h"
#include "gfx.h"

extern ZZ_VIDEO_STATE vs;
int hsize;
uint16_t argb888_to_rgb565(uint32_t argb);


void lcd_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code)
{
	if(RGB_Code==CL_TRANSPARENT)
		return;
	if(vs.colormode==MNTVA_COLOR_8BIT)
		*(volatile uint8_t*)(((uint32_t)vs.framebuffer+(uint32_t)vs.framebuffer_pan_offset) + (1*(Ypos*hsize + Xpos))) = (uint8_t)RGB_Code;
	else if(vs.colormode==MNTVA_COLOR_16BIT565 || vs.colormode==MNTVA_COLOR_15BIT)
		*(volatile uint16_t*)(((uint32_t)vs.framebuffer+(uint32_t)vs.framebuffer_pan_offset) + (2*(Ypos*hsize + Xpos))) = argb888_to_rgb565(RGB_Code);
	else if(vs.colormode==MNTVA_COLOR_32BIT)
		*(volatile uint32_t*)(((uint32_t)vs.framebuffer+(uint32_t)vs.framebuffer_pan_offset) + (4*(Ypos*hsize + Xpos))) = RGB_Code;
}

static void drawChar(sFONT *Font, uint16_t Xpos, uint16_t Ypos, const uint8_t *c)
{
  uint32_t i = 0, j = 0;
  uint16_t height, width;
  uint8_t  offset;
  uint8_t  *pchar;
  uint32_t line;

  height = Font->Height;
  width  = Font->Width;

  offset =  8 *((width + 7)/8) -  width ;

  for(i = 0; i < height; i++)
  {
    pchar = ((uint8_t *)c + (width + 7)/8 * i);

    switch(((width + 7)/8))
    {

    case 1:
      line =  pchar[0];
      break;

    case 2:
      line =  (pchar[0]<< 8) | pchar[1];
      break;

    case 3:
    default:
      line =  (pchar[0]<< 16) | (pchar[1]<< 8) | pchar[2];
      break;
    }

    for (j = 0; j < width; j++)
    {
      if(line & (1 << (width- j + offset- 1)))
      {
    	  if(vs.colormode==MNTVA_COLOR_8BIT)
    		  lcd_DrawPixel((Xpos + j), Ypos, 1);
    	  else
    		  lcd_DrawPixel((Xpos + j), Ypos, Font->TextColor);
      }
      else
      {
    	  if(vs.colormode==MNTVA_COLOR_8BIT)
    		  lcd_DrawPixel((Xpos + j), Ypos, 0);
    	  else
    		  lcd_DrawPixel((Xpos + j), Ypos, Font->BackColor);
      }
    }
    Ypos++;
  }
}

void lcd_DisplayChar(sFONT *Font, uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
	drawChar(Font, Xpos, Ypos, &Font->table[(Ascii-' ') * Font->Height * ((Font->Width + 7) / 8)]);
}
void displayStringAt(sFONT *Font,uint16_t Xpos, uint16_t Ypos, uint8_t *Text, Text_AlignModeTypdef Mode)
{
  uint16_t ref_column = 1, i = 0;
  uint32_t size = 0, xsize = 0;
  uint8_t  *ptr = Text;

  /* Get the text size */
  while (*ptr++) size ++ ;

  if(vs.scalemode)
	  hsize=vs.vmode_hsize>>1;
  else
	  hsize=vs.vmode_hsize;
  /* Characters number per line */
  xsize = (hsize/Font->Width);

  switch (Mode)
  {
  case CENTER_MODE:
    {
      ref_column = Xpos + ((xsize - size)* Font->Width) / 2;
      break;
    }
  case LEFT_MODE:
    {
      ref_column = Xpos;
      break;
    }
  case RIGHT_MODE:
    {
      ref_column = - Xpos + ((xsize - size)*Font->Width);
      break;
    }
  default:
    {
      ref_column = Xpos;
      break;
    }
  }

  /* Check that the Start column is located in the screen */
  if ((ref_column < 1) || (ref_column >= 0x8000))
  {
    ref_column = 1;
  }

  /* Send the string character by character on LCD */
  while ((*Text != 0) && (((hsize - ((i*Font->Width) & 0xFFFF)) >= Font->Width)))
  {
    /* Display one character on LCD */
    lcd_DisplayChar(Font,ref_column, Ypos, *Text);
    /* Decrement the column position by 16 */
    ref_column += Font->Width;
    /* Point on the next character */
    Text++;
    i++;
  }
}
