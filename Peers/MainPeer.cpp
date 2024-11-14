#include "Peer.h"

string getPrivateIP() {
    struct ifaddrs *interfaces = nullptr;
    struct ifaddrs *temp_addr = nullptr;
    string ipAddress = "Not found";

    if (getifaddrs(&interfaces) == 0) {
        temp_addr = interfaces;
        while (temp_addr != nullptr) {
            if (temp_addr->ifa_addr && temp_addr->ifa_addr->sa_family == AF_INET) {
                string interfaceName(temp_addr->ifa_name);
                if (interfaceName != "lo") {
                    ipAddress = inet_ntoa(((struct sockaddr_in*)temp_addr->ifa_addr)->sin_addr);
                    break;
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    freeifaddrs(interfaces);
    return ipAddress;
}

int main() {
    string ip, path, serverIp;
    int port;
    cout<<"Ingrese la ip privada del servidor: ";
    cin>>serverIp;
    cout<<"Ingrese la ruta de la carpeta a subir: ";
    cin>>path;
    if(path[path.size()-1] != '/') path+="/";
    cout<<"Ingrese un puerto: ";
    cin>>port;
    Peer peer(getPrivateIP(), path, port, serverIp);
    peer.connect();
    peer.generateThread();
    ull h1, h2, t;
    string s;
    while(1) {
        cout << "Seleccione una opcion:\n1.Request\n2.Find\n";
        cin>>t;
        if(t == 1){
            cout << "Inserte el primer hash: ";
            cin >> h1;
            cout << "Inserte el segundo hash: ";
            cin >> h2;
            cout << "Inserte el tamaño del archivo: ";
            cin >> t;
            peer.download(h1,h2,t);
        } 
        else {
            cout << "Inserte el nombre del archivo que desea: ";
            cin >> s;
            peer.findFile(s);
        }
    }
    return 0;
}