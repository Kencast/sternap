#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <bits/stdc++.h>
#include "../httplib.h"
#include "Util.h"

using namespace std;
using namespace httplib;

typedef long long ll;
typedef unsigned long long ull;

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
    int port;
};

class MainServer {
    private:
        multimap<melvin, hugo> fileTable;
        string IP;
        int PORT; 
        char separator='\n';

    bool registerFiles(string &data){
        //Primer string es el ip, y segundo el puerto
        //En grupos de 4: hash1, hash2, tamaño, nombre
        vector<string> tokens;
        istringstream stream(data);
        string token;
        try{
            while(getline(stream, token, this->separator)){
                tokens.push_back(token);
            }
            if(tokens.size()<2 || (tokens.size()-2)%4) return false;
            //cout<<token[0]<<' '<<token[1]<<'\n';
            for(int i=2; i<tokens.size(); i+=4){
                melvin file_data = {stoull(tokens[i]), stoull(tokens[i+1]), stoull(tokens[i+2])};
                hugo connect_data = {tokens[i+3], tokens[0], stoi(tokens[1])};
                fileTable.insert({file_data, connect_data});
            }
            for(auto a: fileTable){
                cout<<a.first.hash1<<' '<<a.first.hash2<<' '<<a.first.size<<'\n'<<a.second.ip<<' '<<a.second.port<<' '<<a.second.name<<'\n';
            }
            return true;
        }
        catch(...){
            cerr<<"Error registering the files\n";
            return false;
        }
    }

    string findFiles(string fileName) {
        cout<<fileName<<'\n';
        set<melvin> found;
        for(auto &entry: fileTable){
            auto &edy=entry.second.name;
            int i;
            string name="";
            for(i = edy.size()-1; edy[i] != '/' && i >= 0; i--) 
                name.push_back(edy[i]);
            reverse(name.begin(), name.end());
            if(kmp(name, fileName)) found.insert(entry.first);
        }
        string res="";
        for(auto &chanto: found)
        res += ("" + to_string(chanto.hash1) + 
            this->separator+to_string(chanto.hash2) + this->separator + to_string(chanto.size) 
            + this->separator);
        if(res.size()) res.erase(res.end()-1);
        return res;
    }

    public: 

    MainServer(int serverSocket, string ip, int port){
        this->fileTable = {};
        this->PORT = port;
        this->IP = ip;
        
        Server server;

        server.Get("/find", [&](const Request& req, Response& res) {
            string name = req.get_param_value("file");
            cout<<"query: "<<name<<'\n';
            name = findFiles(name);
            cout<<name<<'\n';
            res.set_content(name, "text/plain");
        });
        
        server.Post("/", [&](const Request& req, Response& res) {
            string data = req.body;
            if(this->registerFiles(data)){
                res.status=200;
                res.set_content("Listo\n", "text/plain");
            }
            else{
                res.status=404;
                res.set_content("Error\n", "text/plain");
            }
        });

        cout << "Servidor escuchando en http://localhost:8080..." << '\n';
        server.listen(this->IP, this->PORT);
    }

};

#endif