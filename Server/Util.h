#ifndef UTIL_H
#define UTIL_H
#include <bits/stdc++.h>

using namespace std;
using namespace httplib;

using ll = long long;
using ull = unsigned ll;

struct melvin {
    ull hash1;
    ull hash2;
    ull size;

    bool operator<(const melvin& other) const {
        if(hash1==other.hash1 && hash2==other.hash2) return size<other.size;
        if(hash1==other.hash1) return hash2<other.hash2;
        return hash1<other.hash1;
    }
};

struct hugo {
    string name;
    string ip;
};

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