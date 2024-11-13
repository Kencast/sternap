#ifndef MAINSERVER_H
#define MAINSERVER_H

#include "../httplib.h"
#include "Util.h"


class MainServer {
    private:
        multimap<melvin, hugo> fileTable;
        string IP;
        int PORT; 
        char separator='\n';
        mutex mtx;

    bool registerFiles(string &data){
        //Primer string es el ip
        //En grupos de 4: hash1, hash2, tamaño, nombre
        vector<string> tokens;
        istringstream stream(data);
        string token;
        try{
            while(getline(stream, token, this->separator)){
                tokens.push_back(token);
            }
            if(tokens.size()<1 || (tokens.size()-1)%4) return false;
            mtx.lock();
            for(int i=1; i<tokens.size(); i+=4){
                melvin file_data = {stoull(tokens[i]), stoull(tokens[i+1]), stoull(tokens[i+2])};
                hugo connect_data = {tokens[i+3], tokens[0]};
                fileTable.insert({file_data, connect_data});
            }
            mtx.unlock();
            for(auto a: fileTable){
                cout<<a.first.hash1<<' '<<a.first.hash2<<' '<<a.first.size<<"--"<<a.second.ip<<' '<<a.second.name<<'\n';
            }
            return true;
        }
        catch(...){
            cerr<<"Error registering the files\n";
            mtx.unlock();
            return false;
        }
    }

    string findFiles(string fileName) {
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

    string getOwners(string data){
        istringstream ss(data);
        string token;
        vector<ull> diego;
        try{
            while(getline(ss, token, this->separator)){
                diego.push_back(stoull(token));
            }
            if(diego.size()<3) return "";
            string res="";
            auto maria = fileTable.equal_range({diego[0], diego[1], diego[2]});
            for (auto it = maria.first; it != maria.second; ++it) {
                res+=(""+it->second.ip+this->separator+it->second.name+this->separator);
            }
            if(res.size()) res.erase(res.end()-1);
            return res;
        }
        catch(...){
            cout << "asd\n";
        }
        return "";
    }

    public: 

    MainServer(int serverSocket, string ip, int port){
        this->fileTable = {};
        this->PORT = port;
        this->IP = ip;
        
        Server server;

        server.Get("/find", [&](const Request& req, Response& res) {
            string name = req.get_param_value("file");
            name = findFiles(name);
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

        server.Post("/owners",[&](const Request& req, Response& res){
            string data = req.body;
            string owners = getOwners(data);
            if(owners != ""){
                res.status=200;
                res.set_content(owners,"text/plain");
            }else{
                res.status=404;
                res.set_content("Error\n", "text/plain");
            }
        });
        cout << "Servidor escuchando en http://localhost:8080..." << '\n';
        server.listen(this->IP, this->PORT);
    }

};

#endif