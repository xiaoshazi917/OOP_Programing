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
    friend class PackageStation;
    

    // 推进状态字段，在字段已经为“SIGNED”时返回false供管理类的函数进行进一步操作
    bool advanceStatus() {
        switch (status) {
            case AWAITING: status = INBOUNDED; return true;
            case INBOUNDED: status = TRANSPORTING; return true;
            case TRANSPORTING: status = SIGNED; return true;
            case SIGNED: return false;
            default: throw runtime_error("ERROR STATUS");
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

    // Lab02作业新增：状态输出更改：Lab01部分仅试图输出枚举的变量名，实际只会输出0
    // 返回当前状态字段。
    // Lab03作业改动：改为对外接口
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
};

class PackageManaging {
private:
    inline static unordered_map<string, size_t> NoTable; // 编号的哈希表，用于创建包裹时的编号查重以及查询包裹时返回包裹数据。
    friend class PackageStation;
public:
    inline static vector<Package> packages;

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

class PackageStation {
private:
    string No; // 站点编号
    string Name; //站点名称
    vector<Package> packs; //vector中会在随机位置插入和删除元素，因此无法继承PackageManaging中的哈希表索引
                            //目前的查找采用遍历方式，后期可以考虑改为隐式Treap
    PackageManaging management;
public:
    PackageStation (string no, string name, PackageManaging manage, vector<Package> Pack = {})
        : No(no), Name(name), packs(Pack), management(manage) { // 构造函数默认包裹列表为空, 这里可以考虑是否开放拷贝包裹列表的接口：如果开放，需要逐个判断包裹状态；不开放则添加包裹会较麻烦。 目前在主函数中不开放接口
        if (no.empty()) {
            throw invalid_argument("Station Number can not be empty");
        }
        if (name.empty()) {
            throw invalid_argument("Station Name can not be empty");
        }
    };

    void intoStation (string no) { //包裹入站
        if (PackageManaging::NoTable.find(no) == PackageManaging::NoTable.end() ) {
            throw runtime_error("Can not find this package");
            return;
        } else if (PackageManaging::packages[PackageManaging::NoTable[no]].statusOutput() != "AWAITING") {
            throw runtime_error("This Package is invalid to get into station"); // 当包裹状态不是待入站时，拒绝入站并抛出报错信息
        }
        // 入站的包裹必定是AWAITING的，因此不用考虑重复入站的问题
        management.advanceStatus(PackageManaging::packages[PackageManaging::NoTable[no]].No); // 推进包裹状态
        packs.push_back(PackageManaging::packages[PackageManaging::NoTable[no]]); // 加入站点的包裹vector
        cout << "Package inbounded successfully" << endl;
        cout << "Package info: \t";
        management.find(no);
    }

    void showInfo () { // 显示站点信息
        cout << "Station Number: " << No << "\t";
        cout << "Station Name: " << Name << endl;
        cout << "Num of packages in the station: " << packs.size() << endl;
    }

    string showNo () { // 对外的编号访问接口
        return No;
    }

    string showName () {
        return Name;
    }

    void outbound (string no) { // 包裹出站
        //寻找包裹
        bool founded = false;
        int index = 0;
        for ( ; index < packs.size(); index++) {
            if (packs[index].No == no) {
                founded = true;
                break;
            }
        }
        if (!founded) {
            throw runtime_error("Package is not in this station");
        }

        
        management.advanceStatus(PackageManaging::packages[PackageManaging::NoTable[no]].No); //推进状态
        packs.erase(packs.begin() + index); //出站
        cout << "Package outbound successfully" << endl;
    }


};

int main () {
    PackageManaging management;
    bool working = true;
    vector<PackageStation> stations;
    while (working) {
        cout << "Package Managing System" << endl;
        cout << "Press 1 to create a new package" << endl;
        cout << "Press 2 to search for a certain package" << endl;
        cout << "Press 3 to show all the packages" << endl;
        cout << "Press 4 to advance the package status" << endl;
        cout << "Press 5 to query the price" << endl;
        cout << "Press 6 to do with stations" << endl;

        int s = 0;
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
        } else if (s == 6) {
            while (1) {
                cout << "Press 1 to create a new station" << endl;
                cout << "Press 2 to inbound a package" << endl;
                cout << "Press 3 to outbound a package" << endl;
                cout << "Press 4 to check station info" << endl;
                cout << "Press keys excepts those upon to back to the main menu" << endl;

                int choice;
                cin >> choice;
                bool ifBreak = false;

                switch (choice) {
                    case 1: {
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        string NO, NAME;
                        cout << "Station Number: ";
                        if (!getline(cin, NO)) break;
                        cout << "Station Name: ";
                        if (!getline(cin, NAME)) break;

                        //判断是否存在编号或名称重复的问题
                        bool repeated = false;
                        for (auto sta : stations) {
                            if (sta.showName() == NAME) {
                                cout << "Station name can not be repeated" << endl;
                                repeated = true;
                                break;
                            }
                            if (sta.showNo() == NO) {
                                cout << "Station number can not be repeated" << endl;
                                repeated = true;
                                break;
                            }
                        }
                        if (repeated) break;

                        stations.emplace_back(NO, NAME, management);
                        cout << "Created successfully" << endl;
                        break;
                    }
                    case 2: {
                        string staNo;
                        cout << "Enter the station No to inbound" << endl;
                        cin >> staNo;
                        bool founded = false;
                        int index;
                        for (int i = 0; i < stations.size(); i++) {
                            if (stations[i].showNo() == staNo) {
                                founded = true;
                                index = i;
                                break;
                            }
                        }
                        if (!founded) {
                            cout << "Station not founded" << endl;
                            break;
                        }
                        while (1) {
                            cout << "Enter the package No to inbound" << endl;
                            string packNo;
                            cin >> packNo;
                            try {
                                stations[index].intoStation(packNo);
                            } catch (const exception& err) {
                                cout << err.what() << endl;
                            }
                            cout << "Inbound one more package? (y)" << endl;
                            string chance;
                            cin >> chance;
                            if (chance == "y") {
                                continue;
                            } else {
                                break;
                            }
                        }

                        break;
                    }
                    case 3: {
                        string staNo;
                        cout << "Enter the station No to outbound" << endl;
                        cin >> staNo;
                        bool founded = false;
                        int index;
                        for (int i = 0; i < stations.size(); i++) {
                            if (stations[i].showNo() == staNo) {
                                founded = true;
                                index = i;
                                break;
                            }
                        }
                        if (!founded) {
                            cout << "Station not founded" << endl;
                            break;
                        }
                        while (1) {
                            cout << "Enter the package No to outbound" << endl;
                            string packNo;
                            cin >> packNo;
                            try {
                                stations[index].outbound(packNo);
                            } catch (const runtime_error& err) {
                                cout << err.what() << endl;
                            }
                            cout << "Outbound one more package? (y)" << endl;
                            string chance;
                            cin >> chance;
                            if (chance == "y") {
                                continue;
                            } else {
                                break;
                            }
                        }

                        break;
                    }
                    case 4: {
                        cout << "Press 1 to select a certain station to show" << endl;
                        cout << "Press 2 to show all the stations" <<endl;

                        int Choice;
                        cin >> Choice;
                        if (Choice == 1) {
                            cout << "Enter the No of the station you want to check" << endl;
                            string staNo;
                            cin >> staNo;
                            bool founded = false;
                            int index;
                            for (int i = 0; i < stations.size(); i++) {
                                if (stations[i].showNo() == staNo) {
                                founded = true;
                                index = i;
                                break;
                                }
                            }
                            if (!founded) {
                                cout << "Station not founded" << endl;
                                break;
                            }

                            stations[index].showInfo();
                        } else if (Choice == 2) {
                            for (auto sta : stations)   sta.showInfo();
                        } else {
                            cout << "Wrong instructions" << endl;
                        }

                        break;
                    }
                    default: {
                        ifBreak = true;
                        break;
                    }
                }

                if (ifBreak) break;
            }
        } else {
            cout << "Wrong instructions" << endl;
        }

    }
    return 0;
}