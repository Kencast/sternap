#include "Peer.h"

int main() {
    Peer peer("192.168.100.7", "../", 8081, "localhost:8080");
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
        } else {
            cout << "Inserte el nombre del archivo que desea: ";
            cin >> s;
            peer.findFile(s);
        }
    }
    return 0;
}