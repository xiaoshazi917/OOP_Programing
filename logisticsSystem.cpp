#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <utility>
#include <limits>
#include <sstream>

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

    // Lab02作业新增：状态输出更改：Lab01部分仅试图输出枚举的变量名，实际只会输出0
    // 返回当前状态字段。
    string statusOutput () {
        string res;
        switch (status) {
            case AWAITING:
                res = "AWAITING";
                break;
            
            case INBOUNDED:
                res = "INBOUNDED";
                break;

            case TRANSPORTING:
                res = "TRANSPORTING";
                break;

            case SIGNED:
                res = "SIGNED";
                break;

            default: // 目前不会产生的情况，可以考虑删除或者直接输出错误提示
                throw runtime_error("ERROR STATUS");
                break;

        }

        return res;
    }

    // 推进状态字段，在字段已经为“SIGNED”时返回false供管理类的函数进行进一步操作
    bool advanceStatus() {
        switch (status) {
            case AWAITING: status = INBOUNDED; return true;
            case INBOUNDED: status = TRANSPORTING; return true;
            case TRANSPORTING: status = SIGNED; return true;
            case SIGNED: return false;
        }
    }

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
        
        if (weight <= 0) {
            throw invalid_argument("The weight of the package must be greater than zero");
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

    // 添加一个新对象。
    void add (string NO, string Sender, string Receiver, double weight) {
        try {
            packages.emplace_back(NO, Sender, Receiver, weight, NoTable);
        } catch (const invalid_argument& err) {
            cout << err.what() << endl;
            return;
        }
        NoTable[NO] = packages.size() - 1;

        
        try {
            cout << "Package created" << endl;
            cout << "Package Number: " << NO << "\t"
            << "Package Sender: " << Sender << "\t"
            << "Package Receiver: " << Receiver << "\t"
            << "Package Weight: " << weight << "\t"
            << "Package Status: " << packages[NoTable[NO]].statusOutput();
        } catch (const runtime_error& err) { // 状态错误的输出，在目前的架构中无用，考虑删除
            cout << err.what() << endl; 
        }
    }

    // 根据包裹的号码查找相应对象，若未找到则输出相应提示。
    void find (string no) {
        if (NoTable.find(no) == NoTable.end() ) {
            cout << "The package was not found" << endl;
            return;
        }

        try {
            cout << "Package Number: " << packages[NoTable[no]].No << "\t"
                << "Package Sender: " << packages[NoTable[no]].sender << "\t"
                << "Package Receiver: " << packages[NoTable[no]].receiver << "\t"
                << "Package Weight: " << packages[NoTable[no]].weight << "\t"
                << "Package Status: " << packages[NoTable[no]].statusOutput() << endl;
        } catch (const runtime_error& err) { // 状态错误的输出，在目前的架构中无用，考虑删除
            cout << err.what() << endl;
        }
    }

    // 展示所有包裹。
    void showAll () {
        if (packages.empty()) {
            cout << "No packages" << endl;
        }
        for (auto pack : packages) {
            try {
                cout << "Package Number: " << pack.No << "\t"
                << "Package Sender: " << pack.sender << "\t"
                << "Package Receiver: " << pack.receiver << "\t"
                << "Package Weight: " << pack.weight << "\t"
                << "Package Status: " << pack.statusOutput() << endl;
            } catch (const runtime_error& err) {
                cout << err.what() << endl;
            }
        }
    }

    // 推进包裹的状态，不提供外部接口以防止非法状态变化（跳过中间状态和倒退状态），仅需要在再次更新SIGNED时返回错误提示。
    void advanceStatus (string No) {
        // 检查是否存在相应包裹
        if (NoTable.find(No) == NoTable.end() ) {
            cout << "The package was not found" << endl;
            return;
        }

        if (packages[NoTable[No]].advanceStatus()) {
            cout << "Status updated, current status: " << packages[NoTable[No]].statusOutput() << endl;
        } else {
            cout << "Invalid operation: The package has been signed." << endl;
        }
    }

    // 计算价格：如果未查询到则抛出错误。 由于读取时允许重量为小数，但运费通常为整数，因此在计算运费时，先计算出含小数点的运费，然后舍弃小数点。
    int priceCalc (string No) {
        // 检查是否存在相应包裹
        if (NoTable.find(No) == NoTable.end() ) {
            throw runtime_error("The package was not found");
            return -1;
        }

        int res;
        res = static_cast<int>(5 + 2 * packages[NoTable[No]].weight);
        return res;
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
        cout << "Press 4 to advance the package status" << endl;
        cout << "Press 5 to query the price" << endl;

        int s;
        cin >> s;
        
        if (s == 1) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            string NO, Sender, Receiver, weightInput;
            cout << "Package Number: ";
            if (!getline(cin, NO)) break;
            cout << "Package Sender: ";
            if (!getline(cin, Sender)) break;
            cout << "Package Receiver: ";
            if (!getline(cin, Receiver)) break;
            cout << "Package Weight: ";
            if (!getline(cin, weightInput)) break;

            istringstream weightStream(weightInput);
            double weight;
            string extra;
            if (!(weightStream >> weight) || (weightStream >> extra)) {
                cout << "Invalid weight: enter one numeric value." << endl;
                continue;
            }

            management.add(NO, Sender, Receiver, weight);
        } else if (s == 2) {
            cout << "Package Number: ";
            string NO;
            cin >> NO;
            management.find(NO);
        } else if (s == 3) {
            management.showAll();
        } else if (s == 4) {
            cout << "Package Number: ";
            string No;
            cin >> No;
            management.advanceStatus(No);
        } else if (s == 5) {
            cout << "Package Number: ";
            string No;
            cin >> No;
            try {
                cout << "Price: " << management.priceCalc(No) << endl;
            } catch (const runtime_error& err) {
                cout << err.what() << endl;
            }
        } else {
            cout << "Wrong instructions" << endl;
        }

    }
    return 0;
}