#ifndef UTIL_H
#define UTIL_H
#include <bits/stdc++.h>
using namespace std;

bool kmp(string s, string pattern){
    vector<int> b(pattern.size()+1, 0);
    //precompute
    int i=0, j=-1;
    b[0]=-1;
    while(i<pattern.size()){
        while(j>=0 && pattern[i]!=pattern[j]) j=b[j];
        i++, j++, b[i]=j;
    }
    // find
    i=0, j=0;
    while(i<s.size()){
        while(j>=0 && s[i]!=pattern[j]) j=b[j];
        i++, j++;
        if(j==pattern.size()) return true;
    }
    return false;
}

#endif