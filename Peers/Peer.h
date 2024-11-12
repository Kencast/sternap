#ifndef PEER_H
#define PEER_H

#include <bits/stdc++.h>
#include "../httplib.h"

using namespace std;

namespace fs  = filesystem;
namespace net = httplib;

using ll = long long;
using vc = vector<char>;
using vs = vector<string>;
using ull = unsigned long long;

class Peer {
    private:
        int PORT;
        string IP;
        string directory;
        string server;
        net::Client client;
        
        ull hash1(ull byte, ull h) {
            ull P1 = 2147483647;
            ull Q1 = 733; 
            return ((h << 1LL) ^ (Q1 - ~byte))%P1;
        }

        ull hash2(ull byte, ull h) {
            ull res = 0x811c9dc5;
            ull rareNum = 0x01000193;
            res = (res ^ h) * rareNum;
            return (res ^ byte) * rareNum;
        }

        vector<string> hashFiles(){
            //Primer string es el ip, y segundo el puerto
            //En grupos de 4: hash1, hash2, tamaño, nombre
            vs filesInfo;
            try{
                filesInfo.push_back(this->IP);
                filesInfo.push_back(to_string(this->PORT));
                for (const auto& entry : fs::recursive_directory_iterator(this->directory)) {
                    if(fs::is_regular_file(entry.status())){
                        ifstream file(entry.path().string(), ios::binary);   
                        if(!file){
                            cerr << "No se pudo abrir el archivo con dir :" + entry.path().string() + "\n";
                            continue;
                        }
                        const size_t buffer_size = 4096;
                        vc buffer(buffer_size);
                        ull h1 = 0, h2 = 0;
                        while (file.read(buffer.data(), buffer_size)) { 
                            for (size_t i = 0; i < buffer_size; ++i) {
                                char byte = buffer[i];
                                h1 = hash1(byte, h1);
                                h2 = hash2(byte, h2); 
                            }
                        }
                        if (file.gcount() > 0) {
                            for (size_t i = 0; i < file.gcount(); ++i) {
                                char byte = buffer[i];
                                h1 = hash1(byte, h1);
                                h2 = hash2(byte, h2);
                            }
                        }
                        filesInfo.push_back(to_string(h1));
                        filesInfo.push_back(to_string(h2));
                        filesInfo.push_back(to_string(entry.file_size()));
                        filesInfo.push_back(entry.path().string());
                    }
                }
                return filesInfo;
            } catch (const fs::filesystem_error &e) {
                cerr << "Error al acceder al directorio: " << e.what() << '\n';
                filesInfo.clear();
                return filesInfo;
            }
        }

    public:
        Peer(string ip, string directory, 
            int port, const string & serverUrl) : client(serverUrl) {
            this->IP = ip;
            this->PORT = port;
            this->directory = directory;
            this->server = serverUrl;
        }
        
        void connect() {
            vs hashedFiles = hashFiles();
            if(!hashedFiles.size()){
                cerr<<"Error al procesar el directorio\n";
                return;
            }
            string body = "";
            for(auto s : hashedFiles) body += s + '\n';   
            body.erase(body.end()-1);
            auto res = client.Post("/", body, "text/plain");
            if (res) {
                cout << "Código de estado: " << res->status << '\n';
                cout << "Respuesta: " << res->body << '\n';
            } else cerr << "Error al realizar la solicitud" << '\n';
        }
        
        void findFile(string name){
            // h1, h2, tam
            vs coincidencias;
            string s = "/find?file="+name;
            auto res = client.Get(s.c_str());
            if(res){
                s = res->body;
                istringstream ss(s);
                string token;
                while(getline(ss, token, '\n')){
                    coincidencias.push_back(token);
                }
            } else cerr << "Error al buscar un archivo\n";
            for (int i = 0; i < coincidencias.size(); i+=3) {
                cout << (i/3) << " - " << coincidencias[i] << ' ' << coincidencias[1+i] 
                << ' ' <<coincidencias[2+i] << '\n';
            }
        }
};

#endif