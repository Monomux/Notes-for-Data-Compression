#include "DPCMUtil.h"

void DPCMEnc(list<unsigned char>* lst, const long& height, const long& width, const ParaA& paras, const int& qlen){
    unsigned char e = 0; // 预测误差
    unsigned char shat, s1, s2, s3; // 预测值
    // 延时队列
    list<unsigned char> delay, enclist;
    // 第一行预测
    delay.push_back(lst->front());
    enclist.push_back(lst->front()); // 第一个元素不参与预测
    lst->pop_front();
    for(int i=1; i<width; i++){ // a1=1, a2=a3=0
        shat = delay.back();
        e = lst->front() - shat;
        e >>= 8-qlen;
        enclist.push_back(e);
        delay.push_back(shat + (e<<(8-qlen)));
        lst->pop_front();
    }
    // 其余行预测
    for(int i=1; i<height; i++){
        if(i>1){delay.pop_front();}
        // 第一列预测 a1=a3=0 a2=1
        auto iter = delay.cbegin();
        shat = *iter;
        e = lst->front() - shat;
        e >>= 8-qlen;
        enclist.push_back(e);
        delay.push_back(shat + (e<<(8-qlen)));
        lst->pop_front();
        // 其余列
        for(int j=1; j<width; j++){
            s1 = delay.back();
            iter = delay.cbegin();
            s3 = *iter; iter++; s2 = *iter;
            shat = s1*paras.a1 + s2*paras.a2 + s3*paras.a3;
            e = lst->front() - shat;
            e >>= 8-qlen;
            enclist.push_back(e);
            delay.push_back(shat + (e << (8-qlen)));
            delay.pop_front();
            lst->pop_front();
        }
    }
    // 剩余信息不参与量化
    enclist.merge(*lst);
    lst->merge(enclist);
}

void DPCMDec(list<unsigned char>* lst, const long& height, const long& width, const ParaA& paras, const int& qlen){
    unsigned char s = 0; // 真值
    unsigned char shat, s1, s2, s3; // 预测值
    // 延时队列
    list<unsigned char> delay, declist;
    // 第一行预测
    delay.push_back(lst->front());
    declist.push_back(lst->front()); // 第一个元素不参与预测
    lst->pop_front();
    for(int i=1; i<width; i++){ // a1=1, a2=a3=0
        shat = delay.back();
        s = shat + (lst->front()<<(8-qlen));
        declist.push_back(s);
        delay.push_back(s);
        lst->pop_front();
    }
    // 其余行预测
    for(int i=1; i<height; i++){
        if(i>1){delay.pop_front();}
        // 第一列预测 a1=a3=0 a2=1
        auto iter = delay.cbegin();
        shat = *iter;
        s = shat + (lst->front()<<(8-qlen));
        declist.push_back(s);
        delay.push_back(s);
        lst->pop_front();
        // 其余列
        for(int j=1; j<width; j++){
            s1 = delay.back();
            iter = delay.cbegin();
            s3 = *iter; iter++; s2 = *iter;
            shat = s1*paras.a1 + s2*paras.a2 + s3*paras.a3;
            s = shat + (lst->front()<<(8-qlen));
            declist.push_back(s);
            delay.push_back(s);
            delay.pop_front();
            lst->pop_front();
        }
    }
    // 剩余信息不参与量化
    declist.merge(*lst);
    lst->merge(declist);
}
