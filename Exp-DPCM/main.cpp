#include "cmdline.h"

#include <iostream>
#include <iomanip>
#include "BitStream.h"
#include "DPCMUtil.h"
using namespace std;

void compressFile(const string& filepath, const string& outpath, const ParaA& paras, const int& qlen=8);
void decompressFile(const string& filepath, const string& outpath, const ParaA& paras, const int& qlen=8);

int main(int argc, char *argv[])
{
    cmdline::parser a;
    a.add<string>("file", 'f', "source file name", true, "");
    a.add<int>("symlen", 'l', "symbol length", false, 8, cmdline::range(1, 8));
    a.add<double>("para1", 'a', "a1", false, 0);
    a.add<double>("para2", 'b', "a2", false, 0);
    a.add<double>("para3", 'c', "a3", false, 1);

    a.add("help", 'h', "print this message");
    a.footer("filename ...");
    a.set_program_name("Exp2");

    bool ok=a.parse(argc, argv);

    if (argc==1 || a.exist("help")){
        cerr<<a.usage();
        return 0;
    }

    if (!ok){
        cerr<<a.error()<<endl<<a.usage();
        return 0;
    }
    ParaA paras;
    int symlem = a.get<int>("symlen");
    string file = a.get<string>("file");
    paras.a1 = a.get<double>("para1");
    paras.a2 = a.get<double>("para2");
    paras.a3 = a.get<double>("para3");
    double sum = paras.a1 + paras.a2 + paras.a3;
    if(sum != 1){
        paras.a1 /= sum;
        paras.a2 /= sum;
        paras.a3 /= sum;
    }
    cout<<file<<"-"<<symlem<<' '<<paras.a1<<' '<<paras.a2<<' '<<paras.a3<<endl;
    string binname = file + to_string(symlem) + string("=")
            + to_string(paras.a1).substr(0, 4) + string("=")
            + to_string(paras.a2).substr(0, 4) + string("=")
            + to_string(paras.a3).substr(0, 4)+ ".bin";
    compressFile(file, binname, paras, symlem);
    string newname = string("rev-") + to_string(symlem) + string("=")
            + to_string(paras.a1).substr(0, 4) + string("=") + to_string(paras.a2).substr(0, 4) + string("=")
            + to_string(paras.a3).substr(0, 4) + string("=") + file;
    cout<<newname<<endl;
    decompressFile(binname, newname, paras, symlem);
//    for (size_t i=0; i<a.rest().size(); i++)
//        file = a.rest()[i];
//        cout<<file<<"--"<<symlem<<endl;
//        compressFile(file, file + ".bin", symlem);
//        decompressFile(file + ".bin", "rev" + file, symlem);

    return 0;
}

void compressFile(const string& filepath, const string& outpath, const ParaA& paras, const int& qlen){
    DPCMBitIOStream stream;
    auto dlst = stream.readFromFile(filepath, 8);
    LONG height, width;
    height = stream.getHeight(); width = stream.getWidth();
    DPCMEnc(&(dlst->img), height, width, paras, qlen);
    stream.writeToFile(outpath, dlst, qlen);
    delete dlst;
}

void decompressFile(const string& filepath, const string& outpath, const ParaA& paras, const int& qlen){
    DPCMBitIOStream stream;
    auto dlst = stream.readFromFile(filepath, qlen);
    LONG height, width;
    height = stream.getHeight(); width = stream.getWidth();
    DPCMDec(&(dlst->img), height, width, paras, qlen);
    stream.writeToFile(outpath, dlst, 8);
    delete dlst;
}
