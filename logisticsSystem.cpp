#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <utility>

using namespace std;

class Package {
private:

    enum Status {AWAITING, INBOUNDED, TRANSPORTING, SIGNED}; // 四个状态分别对应 未入站、已入站、正在运输和已签收
    Status status;
    string No;
    string sender;
    string receiver;
    double weight;

    friend class PackageManaging;

public:

    Package (string NO, string Sender, string Receiver, double Weight
        , const unordered_map<string, size_t>& noTable, Status status = Status::AWAITING)
        : status(status), No(move(NO)), sender(move(Sender)), receiver(move(Receiver)), weight(move(Weight))
    {
        if (No.empty()) {
            throw invalid_argument("The package number can not be blank");
        } else if (noTable.count(No) > 0) {
            throw invalid_argument("The package number already exists");
        }
        
        if (weight < 0) {
            throw invalid_argument("The weight of the package can not be negative");
        }
        if (sender.empty()) {
            throw invalid_argument("Sender can not be empty");
        }
        if (receiver.empty()) {
            throw invalid_argument("Receiver can not be empty");
        }
    };    

};

class PackageManaging {
private:
    unordered_map<string, size_t> NoTable; // 编号的哈希表，用于创建包裹时的编号查重以及查询包裹时返回包裹数据。
public:
    vector<Package> packages;

    void add (string NO, string Sender, string Receiver, double weight) {
        try {
            packages.emplace_back(NO, Sender, Receiver, weight, NoTable);
        } catch (invalid_argument err) {
            cout << err.what() << endl;
            return;
        }
        NoTable[NO] = packages.size() - 1;

        cout << "Package created" << endl;
        cout << "Package Number: " << NO << "\t"
            << "Package Sender: " << Sender << "\t"
            << "Package Receiver: " << Receiver << "\t"
            << "Package Weight: " << weight << "\t"
            << "Package Status: AWAITING" << endl;
    }

    void find (string no) {
        if (NoTable.find(no) == NoTable.end() ) {
            cout << "The package was not found" << endl;
            return;
        }
        cout << "Package Number: " << packages[NoTable[no]].No << "\t"
            << "Package Sender: " << packages[NoTable[no]].sender << "\t"
            << "Package Receiver: " << packages[NoTable[no]].receiver << "\t"
            << "Package Weight: " << packages[NoTable[no]].weight << "\t"
            << "Package Status: " << packages[NoTable[no]].status << endl;
    }

    void showAll () {
        if (packages.empty()) {
            cout << "No packages" << endl;
        }
        for (auto pack : packages) {
            cout << "Package Number: " << pack.No << "\t"
            << "Package Sender: " << pack.sender << "\t"
            << "Package Receiver: " << pack.receiver << "\t"
            << "Package Weight: " << pack.weight << "\t"
            << "Package Status: " << pack.status << endl;
        }
    }
};

int main () {
    PackageManaging management;
    bool working = true;
    while (working) {
        cout << "Package Managing System" << endl;
        cout << "Press 1 to create a new package" << endl;
        cout << "Press 2 to search for a certain package" << endl;
        cout << "Press 3 to show all the packages" << endl;

        int s;
        cin >> s;
        
        if (s == 1) {
            string NO, Sender, Receiver;
            double weight;
            cin >> NO >> Sender >> Receiver >> weight;
            management.add(NO, Sender, Receiver, weight);
        } else if (s == 2) {
            string NO;
            cin >> NO;
            management.find(NO);
        } else if (s == 3) {
            management.showAll();
        } else {
            cout << "Wrong instructions" << endl;
        }

    }
    return 0;
}