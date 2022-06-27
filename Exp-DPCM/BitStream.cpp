#include "BitStream.h"
#include <cmath>

BitIOStream::BitIOStream() {
    in = nullptr;
    out = nullptr;
    endpi = 0;
}

BitIOStream::~BitIOStream() {
    if(in){
        in->close();
        delete in;
    }
    if(out){
        out->close();
        delete out;
    }
}

int BitIOStream::readBits(const int& symlen) {
    if(in){
        int size;
        if(in->eof()){
            return -1;
        }else if(endpi - in->tellg() < symlen){
            size = endpi - in->tellg();
        }else{
            // 每批读codeWordLength个字节，可以保证得到8个符号
            size = symlen;
        }
        in->read(reinterpret_cast<char *>(buffer), sizeof(char) * size);
        parseBits(symlen, size);
        return size * 8 / symlen;
    }else{
        return -1;
    }
}

void BitIOStream::parseBits(const int& symlen, const int& size) {
    if(symlen == 8){
        memcpy(qbuffer, buffer, sizeof(char) * symlen);
    }else{
        // long long占用8字节，恰好可以完成拼接
        unsigned long long cat = 0;
        for(int i=0; i<size; i++){ // 拼接比特序列
            cat = (cat << 8) + (unsigned char)buffer[i];
        }
        cat >>= size*8-(size * 8 / symlen * symlen); // 去除多余比特
        for(int i= size * 8 / symlen; i >= 1; i--){
            qbuffer[i-1] = cat & ((1<<symlen) - 1); // 读codeWordLength比特
            cat >>= symlen;
        }
    }
}

list<char>* BitIOStream::readFromFile(const string &path, const int& symlen) {
    if(in){in->close();delete in;}
    in = new ifstream(path, ios::in|ios::binary);
    // 确定文件字节数
    in->seekg(0, ios::end);
    endpi = in->tellg();
    in->seekg(0, ios::beg);

    list<char>* lst = new list<char>();
    int size;
    while(in->tellg() < endpi){
        size = readBits(symlen);
        if(size > 0){for(int i=0; i<size; i++){lst->push_back(qbuffer[i]);}}
        else{break;}
    }

    in->close();
    delete in;
    in = nullptr;

    return lst;
}

void BitIOStream::writeToFile(const string &path, list<char>* lst, const int& symlen) {
    if(out){out->close();delete out;}
    out = new ofstream(path, ios::out|ios::binary);
    if(out){
        int size = lst->size();
        int batchs = size / 8;
        // 每批写8个字符，可保证每批写symlen个字节
        for(int i=0; i<batchs; i++){
            unsigned long long t = 0;
            // 读8个长为symlen的字符
            for(int j=0; j<8; j++){
                t = (t<<symlen) | (unsigned char)(lst->front());
                lst->pop_front();
            }
            // 写symlen个字节
            for(int j=symlen; j>=1; j--){
                wbuffer[j-1] = char(t & 0xff);
                t >>= 8;
            }
            out->write(reinterpret_cast<const char *>(wbuffer), sizeof(char) * symlen);
        }
        // 如有剩余，写剩余字符
        int num = size - batchs * 8;
        if(num){
            unsigned long long t = 0;
            // 读num个长为symlen的字符
            for(int j=0; j<num; j++){
                t = (t<<symlen) | (unsigned char)(lst->front());
                lst->pop_front();
            }
            int bytes = num * symlen / 8;
            if((num*symlen)%8){bytes++;}
            t <<= bytes * 8 - num * symlen; // 补0使比特数可被8整除
            // 写bytes个字节
            for(int j=bytes; j>=1; j--){
                wbuffer[j-1] = char(t & 0xff);
                t >>= 8;
            }
            out->write(reinterpret_cast<const char *>(wbuffer), sizeof(char) * bytes);
        }
    }

    out->close();
    delete out;
    out = nullptr;
}

int DPCMBitIOStream::readBits(const int& symlen) {
    if(in){
        int size;
        if(in->eof()){
            return -1;
        }else if(endpi - in->tellg() < symlen){
            size = endpi - in->tellg();
        }else{
            // 每批读codeWordLength个字节，可以保证得到8个符号
            size = symlen;
        }
        in->read(buffer, sizeof(char)*size);
        parseBits(symlen, size);
        return size * 8 / symlen;
    }else{
        return -1;
    }
}

void DPCMBitIOStream::parseBits(const int& symlen, const int& size) {
    if(symlen == 8){
        memcpy(qbuffer, buffer, sizeof(char) * symlen);
    }else{
        // long long占用8字节，恰好可以完成拼接
        unsigned long long cat = 0;
        for(int i=0; i<size; i++){ // 拼接比特序列
            cat = (cat << 8) + (unsigned char)buffer[i];
        }
        cat >>= size*8-(size * 8 / symlen * symlen); // 去除多余比特
        for(int i= size * 8 / symlen; i >= 1; i--){
            qbuffer[i-1] = cat & ((1<<symlen) - 1); // 读codeWordLength比特
            cat >>= symlen;
        }
    }
}

datalist* DPCMBitIOStream::readFromFile(const string &path, const int &symlen) {
    if(in){in->close();delete in;}
    in = new bitmapfstream(path, ios::in|ios::binary);
    // 读位图文件头
    in->read(&header);
    // 读位图信息头
    in->read(&infoheader);
    // 读颜色表
    if(infoheader.biClrUsed){
        in->read(rgbquad, infoheader.biClrUsed);
    }else{
        in->read(rgbquad, pow(2, infoheader.biBitCount));
    }
    datalist* dlst = new datalist;
    list<unsigned char>* lst = &dlst->img;
    // 第一个字符未量化
    in->read(buffer, sizeof(char));
    lst->push_back(buffer[0]);
    int currentpos = in->tellp();
    in->seekp(0, ios::end);
    int imgbytes = infoheader.biWidth*infoheader.biHeight;
    if(symlen == 8){
        endpi = currentpos + imgbytes - 1;
    }else{
        endpi = currentpos + ceil(imgbytes*symlen/8);
    }
    streamoff remainendp = in->tellp();
    in->seekp(currentpos, ios::beg);
    int size = 0, sizecount = 0;
    while(in->tellp() < endpi){
        size = readBits(symlen);
        if(size > 0){
            if(sizecount + size > imgbytes - 1){
                size -= sizecount + size - imgbytes + 1;
            }
            for(int i=0; i<size; i++){lst->push_back(qbuffer[i]);}
            sizecount += size;
        }
        else{break;}
    }
    while(in->tellp() < remainendp){
        in->read(buffer, sizeof(char));
        dlst->remains.push_back(buffer[0]);
    }

    in->close();
    delete in;
    in = nullptr;

    return dlst;
}

void DPCMBitIOStream::writeToFile(const string &path, datalist *dlst, const int &symlen) {
    if(out){out->close();delete out;}
    out = new bitmapfstream(path, ios::out|ios::binary);
    if(out){
        // 写位图文件头
        out->write(&header);
        // 写位图信息头
        out->write(&infoheader);
        // 写颜色表
        if(infoheader.biClrUsed){
            out->write(rgbquad, infoheader.biClrUsed);
        }else{
            out->write(rgbquad, pow(2, infoheader.biBitCount));
        }
        list<unsigned char>* lst = &dlst->img;
        // 写第一个元素
        wbuffer[0] = lst->front();
        lst->pop_front();
        out->write(wbuffer, sizeof(char));

        int size = lst->size();
        int batchs = size / 8;
        // 每批写8个字符，可保证每批写symlen个字节
        for(int i=0; i<batchs; i++){
            unsigned long long t = 0;
            // 读8个长为symlen的字符
            for(int j=0; j<8; j++){
                t = (t<<symlen) | (unsigned char)(lst->front());
                lst->pop_front();
            }
            // 写symlen个字节
            for(int j=symlen; j>=1; j--){
                wbuffer[j-1] = char(t & 0xff);
                t >>= 8;
            }
            out->write(wbuffer, sizeof(char) * symlen);
        }
        // 如有剩余，写剩余字符
        int num = size - batchs * 8;
        if(num){
            unsigned long long t = 0;
            // 读num个长为symlen的字符
            for(int j=0; j<num; j++){
                t = (t<<symlen) | (unsigned char)(lst->front());
                lst->pop_front();
            }
            int bytes = num * symlen / 8;
            if((num*symlen)%8){bytes++;}
            t <<= bytes * 8 - num * symlen; // 补0使比特数可被8整除
            // 写bytes个字节
            for(int j=bytes; j>=1; j--){
                wbuffer[j-1] = char(t & 0xff);
                t >>= 8;
            }
            out->write(wbuffer, sizeof(char) * bytes);
        }
    }
    if(!dlst->remains.empty()){
        for(unsigned char& c:dlst->remains){
            wbuffer[0] = c;
            out->write(wbuffer, sizeof(char));
        }
    }

    out->close();
    delete out;
    out = nullptr;
}

DPCMBitIOStream::DPCMBitIOStream() {
    memset(&header, 0, sizeof(BITMAPFILEHEADER));
    memset(&infoheader, 0, sizeof(BITMAPINFOHEADER));
    memset(rgbquad, 0, 256*sizeof(RGBQUAD));
    in = nullptr; out = nullptr; endpi = 0;
}

DPCMBitIOStream::~DPCMBitIOStream() {
    if(in){in->close(); delete in;}
    if(out){out->close(); delete out;}
}

LONG DPCMBitIOStream::getHeight() {
    return infoheader.biHeight;
}

LONG DPCMBitIOStream::getWidth() {
    return infoheader.biWidth;
}

ifstream& operator>>(ifstream& in, BITMAPFILEHEADER& header){
    in>>header.bfType>>header.bfSize>>header.bfReserved1>>header.bfReserved2>>header.bfOffBits;
    return in;
}

ifstream& operator>>(ifstream& in, BITMAPINFOHEADER& infoheader){
    in>>infoheader.biSize>>infoheader.biWidth>>infoheader.biHeight>>infoheader.biPlanes
        >>infoheader.biBitCount>>infoheader.biCompression>>infoheader.biSizeImage
        >>infoheader.biXPelsPerMeter>>infoheader.biYPelsPerMeter>>infoheader.biClrUsed
        >>infoheader.biClrImportant;
    return in;
}

ofstream& operator<<(ofstream& out, BITMAPFILEHEADER& header){
    out<<header.bfType<<header.bfSize<<header.bfReserved1<<header.bfReserved2<<header.bfOffBits;
    return out;
}

ofstream& operator<<(ofstream& out, BITMAPINFOHEADER& infoheader){
    out<<infoheader.biSize<<infoheader.biWidth<<infoheader.biHeight<<infoheader.biPlanes
        <<infoheader.biBitCount<<infoheader.biCompression<<infoheader.biSizeImage
        <<infoheader.biXPelsPerMeter<<infoheader.biYPelsPerMeter<<infoheader.biClrUsed
        <<infoheader.biClrImportant;
    return out;
}

bitmapfstream &bitmapfstream::read(BITMAPFILEHEADER *header) {
    fstream::read((char*)&(header->bfType), sizeof(WORD));
    fstream::read((char*)&(header->bfSize), sizeof(DWORD));
    fstream::read((char*)&(header->bfReserved1), sizeof(WORD));
    fstream::read((char*)&(header->bfReserved2), sizeof(WORD));
    fstream::read((char*)&(header->bfOffBits), sizeof(DWORD));
    return *this;
}

void bitmapfstream::close() {
    basic_fstream::close();
}

bitmapfstream &bitmapfstream::read(BYTE *p, const size_t &size) {
    fstream::read(reinterpret_cast<char *>(p), size);
    return *this;
}

bitmapfstream &bitmapfstream::write(BYTE *p, const size_t &size) {
    fstream::write(reinterpret_cast<const char *>(p), size);
    return *this;
}

bitmapfstream &bitmapfstream::write(BITMAPFILEHEADER *header) {
    fstream::write((char*)&(header->bfType), sizeof(WORD));
    fstream::write((char*)&(header->bfSize), sizeof(DWORD));
    fstream::write((char*)&(header->bfReserved1), sizeof(WORD));
    fstream::write((char*)&(header->bfReserved2), sizeof(WORD));
    fstream::write((char*)&(header->bfOffBits), sizeof(DWORD));
    return *this;
}

bitmapfstream &bitmapfstream::read(BITMAPINFOHEADER *infoheader) {
    fstream::read((char*)&(infoheader->biSize), sizeof(DWORD));
    fstream::read((char*)&(infoheader->biWidth), sizeof(LONG));
    fstream::read((char*)&(infoheader->biHeight), sizeof(LONG));
    fstream::read((char*)&(infoheader->biPlanes), sizeof(WORD));
    fstream::read((char*)&(infoheader->biBitCount), sizeof(WORD));
    fstream::read((char*)&(infoheader->biCompression), sizeof(DWORD));
    fstream::read((char*)&(infoheader->biSizeImage), sizeof(DWORD));
    fstream::read((char*)&(infoheader->biXPelsPerMeter), sizeof(LONG));
    fstream::read((char*)&(infoheader->biYPelsPerMeter), sizeof(LONG));
    fstream::read((char*)&(infoheader->biClrUsed), sizeof(DWORD));
    fstream::read((char*)&(infoheader->biClrImportant), sizeof(DWORD));
    return *this;
}

bitmapfstream &bitmapfstream::write(BITMAPINFOHEADER *infoheader) {
    fstream::write((char*)&(infoheader->biSize), sizeof(DWORD));
    fstream::write((char*)&(infoheader->biWidth), sizeof(LONG));
    fstream::write((char*)&(infoheader->biHeight), sizeof(LONG));
    fstream::write((char*)&(infoheader->biPlanes), sizeof(WORD));
    fstream::write((char*)&(infoheader->biBitCount), sizeof(WORD));
    fstream::write((char*)&(infoheader->biCompression), sizeof(DWORD));
    fstream::write((char*)&(infoheader->biSizeImage), sizeof(DWORD));
    fstream::write((char*)&(infoheader->biXPelsPerMeter), sizeof(LONG));
    fstream::write((char*)&(infoheader->biYPelsPerMeter), sizeof(LONG));
    fstream::write((char*)&(infoheader->biClrUsed), sizeof(DWORD));
    fstream::write((char*)&(infoheader->biClrImportant), sizeof(DWORD));
    return *this;
}

bitmapfstream &bitmapfstream::read(RGBQUAD *quads, const size_t &size) {
    for(int i=0; i<size; i++){
        fstream::read((char*)&quads[i].rgbBlue, sizeof(BYTE));
        fstream::read((char*)&quads[i].rgbGreen, sizeof(BYTE));
        fstream::read((char*)&quads[i].rgbRed, sizeof(BYTE));
        fstream::read((char*)&quads[i].rgbReserved, sizeof(BYTE));
    }
    return *this;
}

bitmapfstream &bitmapfstream::write(RGBQUAD *quads, const size_t &size) {
    for(int i=0; i<size; i++){
        fstream::write((char*)&quads[i].rgbBlue, sizeof(BYTE));
        fstream::write((char*)&quads[i].rgbGreen, sizeof(BYTE));
        fstream::write((char*)&quads[i].rgbRed, sizeof(BYTE));
        fstream::write((char*)&quads[i].rgbReserved, sizeof(BYTE));
    }
    return *this;
}

streamoff bitmapfstream::tellp() {
    return basic_ostream::tellp();
}

bitmapfstream& bitmapfstream::seekp(streamoff off, const ios_base::seekdir &dir) {
    fstream::seekp(off, dir);
    return *this;
}

streamoff bitmapfstream::tellg() {
    return basic_istream::tellg();
}

bool bitmapfstream::eof() {
    return fstream::eof();
}
