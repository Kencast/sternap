#include "Peer.h"

int main() {
    Peer h("h", "../.vscode", 0, "http://localhost:8080");
    h.connect();
    string h1, h2;
    int in;
    while(1){
        cout << "Seleccione una opcion:\n1.Request\n2.Find\n";
        cin>>in;
        if(in == 1){
            cout << "Inserte el primer hash: ";
            cin >> h1;
            cout << "Inserte el segundo hash: ";
            cin >> h2;
        } else {
            cout << "Inserte el nombre del archivo que desea: ";
            cin >> h1;
            h.findFile(h1);
        }
    }
    return 0;
}