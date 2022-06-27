#ifndef EXP2_WINDOWS_DEFINE_H
#define EXP2_WINDOWS_DEFINE_H

typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned char BYTE;

typedef struct tagBITMAPFILEHEADER{
    WORD bfType; // 位图文件类型:BM
    DWORD bfSize; // 位图文件大小，单位字节
    WORD bfReserved1; // 保留字1，必须为0
    WORD bfReserved2; // 保留字2，必须为0
    DWORD bfOffBits; // 文件头到数据的偏移
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
    DWORD biSize; // 该结构所占字节数
    LONG biWidth; // 位图宽度
    LONG biHeight; // 位图高度
    WORD biPlanes; // 目标设备位面数，必须为1
    WORD biBitCount; // 颜色深度，即每个像素所占位数
    DWORD biCompression; // 位图压缩类型
    DWORD biSizeImage; // 位图大小
    LONG biXPelsPerMeter; // 位图水平分辨率
    LONG biYPelsPerMeter; // 位图垂直分辨率
    DWORD biClrUsed; // 位图实际使用的颜色表中颜色数
    DWORD biClrImportant; // 位图显示中比较重要的颜色数
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD{
    BYTE rgbBlue; // 该颜色的蓝色分量
    BYTE rgbGreen; // 该颜色的绿色分量
    BYTE rgbRed; // 该颜色的红色分量
    BYTE rgbReserved; // 保留值
} RGBQUAD;

#endif //EXP2_WINDOWS_DEFINE_H
