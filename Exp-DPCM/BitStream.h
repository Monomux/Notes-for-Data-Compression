#ifndef EXP2_BITIOSTREAM_H
#define EXP2_BITIOSTREAM_H

#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include "windows_define.h"

using namespace std;

class BitIOStream {
private:
    ifstream *in;
    ofstream *out;
    int readBits(const int& symlen=8);
    void parseBits(const int& symlen, const int& size);
protected:
    BYTE qbuffer[8]; // 实际读入字符值
    BYTE buffer[8]; // 读所用缓存
    BYTE wbuffer[8]; // 写所用缓存
    int endpi;
public:
    BitIOStream();
    ~BitIOStream();

    list<char>* readFromFile(const string &path, const int& symlen=8);
    void writeToFile(const string &path, list<char>* lst, const int& symlen=8);
};

class bitmapfstream:fstream {
public:
    bitmapfstream(const string& basicString, _Ios_Openmode openmode):fstream(basicString, openmode){}
    bitmapfstream& read(BITMAPFILEHEADER* header);
    bitmapfstream& read(BITMAPINFOHEADER* infoheader);
    bitmapfstream& read(RGBQUAD* quads, const size_t& size);
    bitmapfstream& read(BYTE* p, const size_t& size);
    bitmapfstream& write(BITMAPFILEHEADER* header);
    bitmapfstream& write(BITMAPINFOHEADER* infoheader);
    bitmapfstream& write(RGBQUAD* quads, const size_t& size);
    bitmapfstream& write(BYTE* p, const size_t& size);
    streamoff tellp();
    streamoff tellg();
    bool eof();
    bitmapfstream& seekp(streamoff off, const ios::seekdir& dir);
    void close();
};

struct datalist{
    list<unsigned char> img;
    list<unsigned char> remains;
};

// 针对256色位图实现的流转换
class DPCMBitIOStream:BitIOStream{
protected:
    BITMAPFILEHEADER header;
    BITMAPINFOHEADER infoheader;
    RGBQUAD rgbquad[256];
private:
    bitmapfstream* in;
    bitmapfstream* out;
    int readBits(const int& symlen=8);
    void parseBits(const int& symlen, const int& size);
public:
    DPCMBitIOStream();
    ~DPCMBitIOStream();
    LONG getHeight();
    LONG getWidth();
    datalist* readFromFile(const string &path, const int& symlen=8);
    void writeToFile(const string &path, datalist* lst, const int& symlen=8);
};

#endif //EXP2_BITIOSTREAM_H
