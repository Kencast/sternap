#ifndef PEER_H
#define PEER_H

#include <bits/stdc++.h>
#include "../httplib.h"
#include <future>

using namespace std;

namespace fs  = filesystem;
namespace net = httplib;

using ll = long long;
using vc = vector<char>;
using vs = vector<string>;
using ull = unsigned long long;
using fi = future<int>;
using vfi = vector<fi>;
using vull = vector<ull>;
using si = set<int>;
using vll = vector<ll>;

class Peer {
    private:
        int PORT;
        string IP;
        string directory;
        string server;
        net::Client client;
        thread serverThread;
        
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
                filesInfo.push_back(this->IP+":"+to_string(this->PORT));
                for (const auto& entry : fs::recursive_directory_iterator(this->directory)) {
                    if(fs::is_regular_file(entry.status())){
                        ifstream file(entry.path().string(), ios::binary);   
                        if(!file){
                            cerr << "No se pudo abrir el archivo con dir :" 
                                + entry.path().string() + "\n";
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
                cerr << "Error al acceder al directorio: " 
                    << e.what() << '\n';
                filesInfo.clear();
                return filesInfo;
            }
        }

        void startServer(){
            net::Server server;
            server.Post("/download", [&](const net::Request& req, net::Response& res) {
                //ruta cantBytes posInicial
                string data=req.body;
                istringstream ss(data);
                vs viquez;
                string s;
                while(getline(ss, s, '\n')) viquez.push_back(s);
                if(viquez.size() < 3) {
                    res.status = 400;
                    res.set_content("Parámetros insuficientes", "text/plain");
                    return;
                }
                // cout << "Este es el archivo: " << viquez[0] << ' ' << viquez[1] << ' ' << viquez[2] << '\n';
                auto file_ptr = make_shared<std::ifstream>(viquez[0], std::ios::binary);
                if(!file_ptr->is_open()){
                    res.status=404;
                    res.set_content("Archivo no encontrado", "text/plain");
                    return;
                }
                ll sizePart=stoll(viquez[1]);
                ll startPos=stoll(viquez[2]);
                res.set_content_provider(
                    sizePart,
                    "application/octet-stream",
                    [file_ptr, startPos](size_t offset, size_t length, net::DataSink& sink) mutable {
                        file_ptr->seekg(startPos+offset);
                        vector<char> buffer(length);
                        file_ptr->read(buffer.data(), length);
                        size_t bytes_read = file_ptr->gcount();
                        sink.write(buffer.data(), bytes_read);
                        if (bytes_read < length) {
                            file_ptr->close();
                            sink.done();
                        }
                        return true;
                    }
                );
            });
            server.listen("0.0.0.0",this->PORT);
        }

        int requestPartFile(string ip, string path, ull cantBytes, ull posIni, ll id){
            net::Client peer(ip);
            string s = path+"\n"+to_string(cantBytes)+"\n"+to_string(posIni);
            auto res = peer.Post("/download",s,"application/octet-stream");
            if(res && res->status==200){
                ofstream outfile(this->directory+"/id="+to_string(id), ios::binary | ios::app);
                if (!outfile.is_open()) {
                    cerr << "Error al abrir el archivo de destino\n";
                    return 0;
                }
                outfile.write(res->body.c_str(), res->body.size());
                outfile.close();
                cout << "Archivo descargado exitosamente desde la posición " << posIni << " y guardado en: " << this->directory << "\n";
                return 1;
            }
            else if(res) {
                cerr<<"No se pudo: " << res->status <<'\n';
                return 0;
            }
            else {
                cerr << "Esta compa esta morido: "+ip+"\n"; 
                return 0;
            }
        }

    public:
        Peer(string ip, string directory, 
            int port, const string & serverUrl) : client(serverUrl) ,
                serverThread() {
            this->IP = ip;
            this->PORT = port;
            this->directory = directory;
            this->server = serverUrl;
        }
        
        void generateThread(){
            serverThread = thread(&Peer::startServer,this);
            serverThread.detach();
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
            } else cerr << "Error al conectarse al servidor central" << '\n';
        }
        
        void findFile(string name){
            // h1, h2, tam ,cant, [nombres]
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
            ll i = 0, c = 1;
            cout << "----COINCIDENCIAS----\n";
            cout << "Hash1 - Hash2 - Tamaño - [ Nombres ]\n";
            while(i < coincidencias.size()){
                cout << c << ") " << coincidencias[i] << " - " 
                << coincidencias[i+1] << " - " << coincidencias[i+2] 
                << " - [ "; 
                ll cN = stoll(coincidencias[i+3]), j;
                for(j = i+4; cN-- ; j++) 
                    cout << "'" << coincidencias[j] << "'" << " ";
                cout << "]\n";
                i = j;
                c++;
            }
        }

        void download(ull h1, ull h2, ull t){
            string body = to_string(h1)+'\n'+to_string(h2)+'\n'+to_string(t);
            // {(ip, name)}
            vs owners;
            auto res = client.Post("/owners", body, "text/plain");
            if(res){
                cout << "Owners: " << res->body << '\n';
            }
            else{
                cerr << "Error al intentar obtener los owners";
                return;
            }
            istringstream ss(res->body);
            while(getline(ss,body,'\n')) owners.push_back(body);
            vfi futuros;
            ll cantOwners = owners.size()/2;
            ll residual = t%cantOwners;
            ull cantBytes=0;
            vull otomanos(cantOwners);
            vull position(cantOwners);
            si avaliables;
            for(int i = 0; i < cantOwners; i++) {
                otomanos[i]=t/cantOwners;
                if(residual>0) otomanos[i]++, residual--;
                position[i]=cantBytes;
                futuros.push_back(async(launch::async,bind
                    (&Peer::requestPartFile, this, owners[i*2], owners[(i*2)+1],
                    otomanos[i], cantBytes, i)));
                cantBytes+=otomanos[i];
                avaliables.insert(i);
            }
            for(int i = 0; i < futuros.size(); i++){
                if(futuros[i].get() == 0){
                    avaliables.erase(i);
                    int f = 0;
                    while(avaliables.size()){
                        int j = *(avaliables.begin());
                        f = (async(launch::async,bind
                            (&Peer::requestPartFile, this, owners[2*j], owners[(2*j)+1],
                            otomanos[i], position[i], i))).get();
                        if(f) break;
                        avaliables.erase(j);
                    }
                    if(!avaliables.size()) {
                        cerr << "No hay peers disponibles\n";
                        return ;
                    }    
                }
            }
            cout << "Nombre para su archivo (con extension): ";
            string name; cin >> name;
            ofstream finalFile(this->directory+name,ios::out);
            if(!finalFile) {
                cerr << "No se pudo abrir el archivo: " + this->directory+name+ '\n';
                return;
            }
            for(int i = 0; i < cantOwners; i++) {
                ifstream file(this->directory+"/id="+to_string(i),ios::in);
                if(!file){
                    cerr << "No se pudo abrir el archivo: " + this->directory+"/id="+to_string(i)+'\n';
                    return;
                }
                finalFile << file.rdbuf();
                file.close();
                if(!fs::remove(this->directory+"/id="+to_string(i))) {
                    cerr << "NO se pudo borrar el archivo: " +this->directory+"/id="+to_string(i)+'\n';
                    return;
                }
            }
            finalFile.close();
            cout<<"Éxito en la descarga del archivo "+name+"\n";
        }

};

#endif