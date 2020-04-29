#pragma once

//assuming pixel lines have multiples of 4 bytes sizes
class BMP
{

  static void setChar(void *buffer, int pos, char ch)
  {
    *(char*)(buffer+pos) = ch;
  }

  static void setLong(void *buffer, int pos, long l)
  {
    *(long*)(buffer+pos) = l;
  }
  
  static void setShort(void *buffer, int pos, short s)
  {
    *(short*)(buffer+pos) = s;
  }
  
public:  
  static const int headerSize16 = 54 + 12;
  static const int headerSize24 = 54;
  
  //https://itnext.io/bits-to-bitmaps-a-simple-walkthrough-of-bmp-image-format-765dc6857393
  static void construct16BitHeader(void *buffer, long xres, long yres)
  {
	  long bytesPerLine = xres * 3;			//TODO padding
	  setChar(buffer,  0, 'B');
	  setChar(buffer,  1, 'M');
	  setLong(buffer,  2, bytesPerLine * yres  + headerSize16); //filesize
	  setLong(buffer,  6, 0);
	  setLong(buffer,  10, headerSize16); 	//offset for pixeldata


	  setLong(buffer,  14, 40); 		//header size
	  setLong(buffer,  18, xres);
	  setLong(buffer,  22, yres);
	  setShort(buffer, 26, 1); 		//planes
	  setShort(buffer, 28, 16); 		//bits

	  setLong(buffer,  30, 3); 		//compression 3 = bit fields
	  setLong(buffer,  38, 0); 		//x pix per meter
	  setLong(buffer,  42, 0); 		//y pix per meter

	  setLong(buffer,  46, 0); 		//biClrUsed
	  setLong(buffer,  50, 0); 		//biClrImportant

	  setLong(buffer,  54, 0xF800); //R mask
	  setLong(buffer,  58, 0x07E0); //G mask
	  setLong(buffer,  62, 0x001F); //B mask
  }

  static void construct24BitHeader(void *buffer, long xres, long yres)
  {
	  long bytesPerLine = xres * 2;
	  setChar(buffer,  0, 'B');
	  setChar(buffer,  1, 'M');
	  setLong(buffer,  2, bytesPerLine * yres  + headerSize24); //filesize
	  setLong(buffer,  6, 0);
	  setLong(buffer,  10, headerSize24); 	//offset for pixeldata


	  setLong(buffer,  14, 40); 		//header size
	  setLong(buffer,  18, xres);
	  setLong(buffer,  22, yres);
	  setShort(buffer, 26, 1); 		//planes
	  setShort(buffer, 28, 24); 		//bits

	  setLong(buffer,  30, 0); 		//compression none. 3 bytes per pixel
	  setLong(buffer,  38, 0); 		//x pix per meter
	  setLong(buffer,  42, 0); 		//y pix per meter

	  setLong(buffer,  46, 0); 		//biClrUsed
	  setLong(buffer,  50, 0); 		//biClrImportant

  }


};


