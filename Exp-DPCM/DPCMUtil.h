#ifndef EXP2_DPCMUTIL_H
#define EXP2_DPCMUTIL_H

#include <list>
using namespace std;

// 预测参数
struct ParaA{
    double a1, a2, a3;
};

// 仅针对256色灰度图
void DPCMEnc(list<unsigned char>* lst, const long& height, const long& width, const ParaA& paras, const int& qlen=8);
void DPCMDec(list<unsigned char>* lst, const long& height, const long& width, const ParaA& paras, const int& qlen=8);



#endif //EXP2_DPCMUTIL_H
