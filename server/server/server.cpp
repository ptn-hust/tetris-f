// Server.cpp
#include <string>
#include <utility>
#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <filesystem>
#include <sys/stat.h>  

#include "network.h"
// #include <direct.h>

const unsigned short SERVER_PORT = 55004;
const unsigned int MAX_CLIENTS = 5;

struct room
{
    std::string name;
    std::string passwd;
    bool status;
    int number;
    std::vector<int> sockets;
    int score1 = -1;
    int score2 = -1;
    bool guestReady = false;
    std::vector<std::string> players;
};

std::vector<room> gameRooms;

struct demo
{
    // array declared inside structure
    std::string arr[100];
};
struct account
{
    std::string email;
    std::string password;
    std::string isLoggedIn;
    std::string playTrain;
    std::string playSolo;
};

std::vector<account> accListRank;
int countAcc = 0;
struct demoAcc
{
    // array declared inside structure
    account arr[100];
};

demoAcc accInfoMain;
int info = 1;

bool comparePlayers(const account &a, const account &b)
{
    return std::stoi(a.playTrain) > std::stoi(b.playTrain);
}

void createFolderAndFile(const std::string &folderName1)
{
    // Tạo thư mục mới
    std::string folderName = "client/" + folderName1;
    if (mkdir(folderName.c_str(), 0777) == 0) {
        std::cout << "Thư mục \"" << folderName << "\" đã được tạo thành công.\n";
    }
    else
    {
        std::cerr << "Không thể tạo thư mục.\n";
        return;
    }

    // Tạo file mới trong thư mục vừa tạo
    std::string filePath = folderName + "/" + folderName1 + "_friend.txt";
    std::string filePath1 = folderName + "/" + folderName1 + "_trainHis.txt";
    std::string filePath2 = folderName + "/" + folderName1 + "_request.txt";

    std::ofstream outputFile(filePath);

    if (outputFile.is_open())
    {
        std::cout << "File \"" << folderName1 << "_friend.txt\" đã được tạo thành công trong thư mục \"" << folderName << "\".\n";
        // Viết dữ liệu vào file nếu cần
        // outputFile << "Nội dung của file\n";
        outputFile.close();
    }
    else
    {
        std::cerr << "Không thể tạo file.\n";
        return;
    }

    std::ofstream outputFile1(filePath1);

    if (outputFile1.is_open())
    {
        std::cout << "File \"" << folderName1 << "_trainHis.txt\" đã được tạo thành công trong thư mục \"" << folderName << "\".\n";
        // Viết dữ liệu vào file nếu cần
        // outputFile << "Nội dung của file\n";
        outputFile1.close();
    }
    else
    {
        std::cerr << "Không thể tạo file.\n";
        return;
    }
    std::ofstream outputFile2(filePath2);

    if (outputFile2.is_open())
    {
        std::cout << "File \"" << folderName1 << "_request.txt\" đã được tạo thành công trong thư mục \"" << folderName << "\".\n";
        // Viết dữ liệu vào file nếu cần
        // outputFile << "Nội dung của file\n";
        outputFile2.close();
    }
    else
    {
        std::cerr << "Không thể tạo file.\n";
        return;
    }
}

std::vector<std::string> getLine(const std::string &filename)
{
    std::ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return {};
    }

    // Đọc dữ liệu từ file vào vector
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inputFile, line))
    {
        lines.push_back(line);
    }

    inputFile.close();
    return lines;
}

std::string getLineString(const std::string &filename)
{
    std::ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return {};
    }
    int checkCount = 0;
    // Đọc dữ liệu từ file vào vector
    std::string lines1 = "";
    std::string line1;
    while (std::getline(inputFile, line1))
    {
        checkCount++;
        if (checkCount < 10)
        {
            lines1 += line1;
            lines1 += "|r|r|";
        }
        else if (checkCount == 10)
        {
            lines1 += line1;
        }
    }

    inputFile.close();
    return lines1;
}

bool removeLineFromFile(const std::string &filename, const std::string &targetString)
{
    std::ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }

    // Đọc dữ liệu từ file vào vector
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inputFile, line))
    {
        if (line != targetString)
        {
            lines.push_back(line);
        }
    }

    inputFile.close();

    // Ghi lại dữ liệu vào file
    std::ofstream outputFile(filename);
    if (!outputFile.is_open())
    {
        std::cerr << "Cannot open file for writing: " << filename << std::endl;
        return false;
    }

    for (const auto &line : lines)
    {
        outputFile << line << std::endl;
    }

    outputFile.close();

    return true;
}

// Hàm để tìm và xóa phòng dựa trên socket
void removeRoomBySocket(int socketToRemove)
{
    auto it = std::remove_if(gameRooms.begin(), gameRooms.end(),
                             [socketToRemove](const room &r)
                             {
                                 return std::find(r.sockets.begin(), r.sockets.end(), socketToRemove) != r.sockets.end();
                             });
    if (it != gameRooms.end())
    {
        if (gameRooms[std::distance(gameRooms.begin(), it)].sockets.size() == 1)
        {
            std::cout << "\n Delete room \n";
            gameRooms.erase(it);
        }
        else
        {
            auto it1 = std::find(gameRooms[std::distance(gameRooms.begin(), it)].sockets.begin(), gameRooms[std::distance(gameRooms.begin(), it)].sockets.end(), socketToRemove);
            gameRooms[std::distance(gameRooms.begin(), it)].sockets.erase(it1);
            std::cout << "\n Delete socket : " << socketToRemove << " khoi room :" << gameRooms[std::distance(gameRooms.begin(), it)].name << "\n";
        }
    }
    else
    {
        std::cout << "not found socket: " << socketToRemove << "\n";
    }
}

void updateFieldInFile(std::string &emailToSearch, int fieldIndex, std::string &newValue)
{
    account acc1[100];
    int info1 = 1;

    std::fstream file("clientList.txt", std::ios::in | std::ios::out);

    if (!file.is_open())
    {
        std::cerr << "Error opening file." << std::endl;
        return;
    }
    // Đưa con trỏ đọc/ghi về đầu file
    file.seekp(0, std::ios::beg);

    while (file >> acc1[info].email)
    {
        file >> acc1[info].password;
        if (acc1[info].email == emailToSearch)
        {
            std::cout << "Find location" << fieldIndex << "\n";

            switch (fieldIndex)
            {

            case 3:
                file.seekp(file.tellg());
                if (newValue == "false")
                {
                    file << "\n"
                         << newValue << "\n";
                }
                else
                {
                    file << "\n"
                         << newValue << " ";
                }
                break;
            case 4:
                file >> acc1[info].isLoggedIn;
                file.seekp(file.tellg());
                file << "\n"
                     << newValue << " ";
                std::cout << "diem tap luyen moi la: " << newValue << "\n";
                break;
            case 5:
                file >> acc1[info].isLoggedIn;
                file >> acc1[info].playTrain;
                file.seekp(file.tellg());
                file << "\n"
                     << newValue << " ";
                break;
            default:
                break;
            }
            break;
            // Nếu email trùng khớp, thực hiện sửa đổi và kết thúc hàm
        }
        else
        {
            file >> acc1[info].isLoggedIn;
            file >> acc1[info].playTrain;
            file >> acc1[info].playSolo;
        }
    }

    file.close();
}

void sendGameRooms(int clientSocket, const std::vector<room> &gameRooms)
{
    // Gửi số lượng phòng trước
    int numRooms = gameRooms.size();
    sendWrapper(clientSocket, reinterpret_cast<char *>(&numRooms), sizeof(int), 0);

    // Gửi thông tin từng phòng
    for (const auto &room : gameRooms)
    {
        // Gửi tên phòng
        int nameSize = room.name.size();
        sendWrapper(clientSocket, reinterpret_cast<char *>(&nameSize), sizeof(int), 0);
        sendWrapper(clientSocket, room.name.c_str(), nameSize, 0);

        // Gửi số lượng sockets trong phòng
        int numSockets = room.sockets.size();
        sendWrapper(clientSocket, reinterpret_cast<char *>(&numSockets), sizeof(int), 0);

        // Gửi thông tin từng socket
        for (int socket : room.sockets)
        {
            sendWrapper(clientSocket, reinterpret_cast<char *>(&socket), sizeof(int), 0);
        }
    }
}
void sendGameRank(int clientSocket, const std::vector<account> &accListRank)
{
    // Gửi số lượng phòng trước
    int numRooms1 = accListRank.size();
    sendWrapper(clientSocket, reinterpret_cast<char *>(&numRooms1), sizeof(int), 0);
    std::cout << numRooms1 << "\n";
    // Gửi thông tin từng acc
    for (const auto &room1 : accListRank)
    {
        // Gửi tên acc
        int nameSize1 = room1.email.size();
        sendWrapper(clientSocket, reinterpret_cast<char *>(&nameSize1), sizeof(int), 0);
        sendWrapper(clientSocket, room1.email.c_str(), nameSize1, 0);

        // Gửi tên acc
        int trainSize = room1.playTrain.size();
        sendWrapper(clientSocket, reinterpret_cast<char *>(&trainSize), sizeof(int), 0);
        sendWrapper(clientSocket, room1.playTrain.c_str(), trainSize, 0);
    }
}

void sendListFriend(int clientSocket, const std::vector<account> &listFriend)
{
    // Gửi số lượng phòng trước
    int numRooms12 = listFriend.size();
    sendWrapper(clientSocket, reinterpret_cast<char *>(&numRooms12), sizeof(int), 0);

    // Gửi thông tin từng acc
    for (const auto &room2 : listFriend)
    {
        // Gửi tên acc
        int nameSize1 = room2.email.size();
        sendWrapper(clientSocket, reinterpret_cast<char *>(&nameSize1), sizeof(int), 0);
        sendWrapper(clientSocket, room2.email.c_str(), nameSize1, 0);
        std::cout << "send" + room2.email;
        // Gửi tên acc
        int trainSize1 = room2.isLoggedIn.size();
        sendWrapper(clientSocket, reinterpret_cast<char *>(&trainSize1), sizeof(int), 0);
        sendWrapper(clientSocket, room2.isLoggedIn.c_str(), trainSize1, 0);

        std::cout << "---" + room2.isLoggedIn + "\n";
    }
}

struct demoAcc input()
{
    account acc[100];
    info = 1;
    struct demoAcc accInfo;
    std::ifstream fin("clientList.txt", std::ios::in | std::ios::out);
    while (fin >> acc[info].email)
    {
        fin >> acc[info].password;
        fin >> acc[info].isLoggedIn;
        fin >> acc[info].playTrain;
        fin >> acc[info].playSolo;

        accInfo.arr[info] = acc[info];
        // std::cout << acc[info].email << "---" << acc[info].password;
        info++;
    }
    fin.close();
    return accInfo;
}

std::vector<account> inputCheck(std::vector<std::string> listname)
{
    account acc[100];
    info = 1;
    std::ifstream fin("clientList.txt", std::ios::in | std::ios::out);
    std::vector<account> friendList;
    while (fin >> acc[info].email)
    {
        fin >> acc[info].password;
        fin >> acc[info].isLoggedIn;
        fin >> acc[info].playTrain;
        fin >> acc[info].playSolo;

        acc[info].password = "";
        if (std::find(listname.begin(), listname.end(), acc[info].email) != listname.end())
        {
            friendList.push_back(acc[info]);
        }
        // std::cout << acc[info].email << "---" << acc[info].password;
        info++;
    }
    fin.close();
    return friendList;
}

struct demo tokenize(std::string s, std::string del = " ")
{
    struct demo result;
    int n = 0;
    int start, end = -1 * del.size();
    do
    {
        start = end + del.size();
        end = s.find(del, start);
        std::cout << s.substr(start, end - start) << std::endl;
        result.arr[n] = s.substr(start, end - start);
        n++;
    } while (end != -1);
    return result;
}

bool contains(std::vector<int> vec, int elem)
{
    bool result = false;
    if (find(vec.begin(), vec.end(), elem) != vec.end())
    {
        result = true;
    }
    return result;
}

void handleClient(int clientSocket)
{
    char buffer[1024];
    int bytesRead;
    std::string messFromClient = "";

    while (true)
    {
        bytesRead = receiveWrapper(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0)
        {
            // Error or connection closed
            break;
        }

        // Process received data
        messFromClient = std::string(buffer, bytesRead);
        struct demo clientInfo = tokenize(messFromClient, "||");

        if (clientInfo.arr[0] == "LOGIN")
        {
            std::string e, p;
            e = clientInfo.arr[1];
            p = clientInfo.arr[2];
            int checkSuccess = 0;
            struct demoAcc checkLogin = input();
            std::cout << "username:: " << e << std::endl;
            std::cout << "password:: " << p << std::endl;

            for (int j = 1; j <= info; j++)
            {
                std::cout << "check username:: " << checkLogin.arr[j].email << std::endl;
                std::cout << "check password:: " << checkLogin.arr[j].password << std::endl;
                if (e == checkLogin.arr[j].email && p == checkLogin.arr[j].password)
                {
                    std::cout << "found" << std::endl;
                    std::cout << checkLogin.arr[j].isLoggedIn << "\n";
                    std::cout << checkLogin.arr[j].playTrain << "\n";
                    std::string str1 = checkLogin.arr[j].isLoggedIn;
                    str1.erase(std::remove_if(str1.begin(), str1.end(), [](unsigned char x)
                                              { return std::isspace(x); }),
                               str1.end());
                    checkLogin.arr[j].isLoggedIn = "true";
                    std::string newValue = "true";
                    updateFieldInFile(e, 3, newValue);
                    sendWrapper(clientSocket, "+OK||4", 7, 0);
                    checkSuccess++;
                    break;
                }
            }
            if (checkSuccess == 0)
            {
                sendWrapper(clientSocket, "-NO||5", 7, 0);
            }
        }
        if (clientInfo.arr[0] == "LOGOUT")
        {
            std::string e;
            e = clientInfo.arr[1];
            std::string newValue = "false";
            updateFieldInFile(e, 3, newValue);
            sendWrapper(clientSocket, "+OK||4", 7, 0);

        }
        else if (clientInfo.arr[0] == "REGISTER")
        {
            std::string e, p;
            e = clientInfo.arr[1];
            p = clientInfo.arr[2];
            int checkSuccess = 0;
            struct demoAcc checkLogin = input();

            for (int j = 1; j <= info; j++)
            {
                // std::cout << checkLogin.arr[j].email << "---" << checkLogin.arr[j].password << std::endl;
                if (e == checkLogin.arr[j].email)
                {
                    sendWrapper(clientSocket, "-NO||5", 7, 0);
                    checkSuccess++;
                    break;
                }
            }
            if (checkSuccess == 0)
            {
                sendWrapper(clientSocket, "+OK||5", 7, 0);
                std::ofstream fout("clientList.txt", std::ios::app);
                fout << e << "\n";
                fout << p << "\n";
                fout << "true "
                     << "\n";
                fout << 0 << "\n";
                fout << 0 << "\n";

                fout.close();
                createFolderAndFile(e);
            }
        }
        // choi che do luyen tap
        else if (clientInfo.arr[0] == "TRAIN")
        {
            std::string e, p;
            e = clientInfo.arr[1];
            p = clientInfo.arr[2];

            if (e != "" && p != "")
            {

                if (stoi(e) > 0)
                {
                    struct demoAcc checkLogin = input();

                    for (int j = 1; j <= info; j++)
                    {

                        if (p == checkLogin.arr[j].email)
                        {
                            std::cout << checkLogin.arr[j].playTrain << "\n";
                            std::string str1 = checkLogin.arr[j].playTrain;
                            str1.erase(std::remove_if(str1.begin(), str1.end(), [](unsigned char x)
                                                      { return std::isspace(x); }),
                                       str1.end());
                            std::string newValue = std::to_string(std::stoi(str1) + 1);
                            updateFieldInFile(p, 4, newValue);

                            std::ofstream fout1his("client/" + p + "/" + p + "_trainHis.txt", std::ios::app);
                            fout1his << e << "\n";
                            fout1his.close();

                            break;
                        }
                    }
                }
            }
        }
        // tao phong choi moi
        else if (clientInfo.arr[0] == "ADD_ROOM")
        {
            std::string e, p;
            e = clientInfo.arr[1];
            p = clientInfo.arr[2];
            auto playerName = clientInfo.arr[3];
            int checkSuccess = 0;
            // Gửi thông tin từng phòng
            for (const auto &room : gameRooms)
            {
                if (e == room.name)
                {
                    sendWrapper(clientSocket, "-NO||6", 7, 0);
                    checkSuccess++;
                    break;
                }
            }
            if (checkSuccess == 0)
            {
                sendWrapper(clientSocket, "+OK||7", 7, 0);
                room room1;
                room1.name = e;
                room1.passwd = p;
                room1.number = 1;
                room1.status = false;
                room1.sockets.push_back(clientSocket);
                room1.players.push_back(playerName);
                gameRooms.push_back(room1);
            }
        }

        else if (clientInfo.arr[0] == "CHECK_ROOM")
        {
            auto roomName = clientInfo.arr[1];
            int checkSuccess = 0;
            for (auto &room : gameRooms)
            {
                if (roomName == room.name)
                {
                    if (room.number == 2)
                    {
                        auto message = "+OK||READY||" + room.players[1] + "||" + (room.guestReady ? "true" : "false");
                        sendWrapper(clientSocket, message.c_str(), message.size(), 0);
                    }
                    else
                    {
                        sendWrapper(clientSocket, "+OK||WAIT", 9, 0);
                    }
                    checkSuccess++;
                    break;
                }
            }
            if (checkSuccess == 0)
            {
                sendWrapper(clientSocket, "-NO||6", 7, 0);
            }
        }

        else if (clientInfo.arr[0] == "READY") {
            auto roomName = clientInfo.arr[1];
            for (auto &room : gameRooms) {
                if (room.name == roomName) {
                    room.guestReady = true;
                }
            }
        }
        // tham gia phong
        else if (clientInfo.arr[0] == "JOIN_ROOM")
        {
            std::string e, p;
            e = clientInfo.arr[1];
            p = clientInfo.arr[2];
            auto username = clientInfo.arr[3];
            int checkSuccess = 0;
            // Kiểm tra thông tin từng phòng
            for (auto &room : gameRooms)
            {
                if (e == room.name)
                {
                    if (p == room.passwd)
                    {

                        if (!contains(room.sockets, clientSocket))
                        {
                            room.sockets.push_back(clientSocket);
                            room.number = 2;
                            room.status = true;
                            room.players.push_back(username);
                            auto msg = "+OK||8||" + room.players[0];
                            
                            sendWrapper(clientSocket, msg.c_str(), msg.size(), 0);
                        }
                        else
                        {
                            sendWrapper(clientSocket, "+OK||7", 7, 0);
                        }
                        checkSuccess++;
                        break;
                    }
                }
            }
            if (checkSuccess == 0)
            {
                sendWrapper(clientSocket, "-NO||6", 7, 0);
            }
        }
        // xem danh sach cac phong choi
        else if (clientInfo.arr[0] == "LIST")
        {
            // Gửi số lượng phòng trước
            // Thêm phòng game

            int numRooms = gameRooms.size(); // Số lượng phòng (điều này phải được tính toán động trong thực tế)
            if (numRooms > 0)
            {
                // std::cout << numRooms << "\n";
                sendWrapper(clientSocket, "+OK||6", 7, 0);
                sendGameRooms(clientSocket, gameRooms);
            }
            else
            {
                sendWrapper(clientSocket, "-NO||6", 7, 0);
            }
        }
        else if (clientInfo.arr[0] == "TETRIS")
        {
            sendWrapper(clientSocket, "-NO||6", 7, 0);
            std::cout << "tetris game send data\n";
        }
        else if (clientInfo.arr[0] == "TETRIS_TEST")
        {

            std::string e, p;
            e = clientInfo.arr[1];
            p = clientInfo.arr[2];
            int checkSuccess = 0;
            // Kiểm tra thông tin từng phòng
            for (auto &room : gameRooms)
            {
                if (e == room.name)
                {
                    for (auto &socket : room.sockets)
                    {
                        if (socket != clientSocket)
                        {
                            std::cout << socket << " ++ " << p << "\n";
                            sendWrapper(socket, "+OK||S", 7, 0);
                        }
                    }
                }
            }
        }
        // thong bao roi` game
        else if (clientInfo.arr[0] == "OUTGAME")
        {

            std::string e, p;
            e = clientInfo.arr[1];
            p = clientInfo.arr[2];
            int checkSuccess = 0;
            // Kiểm tra thông tin từng phòng
            for (auto &room : gameRooms)
            {
                if (e == room.name)
                {
                    for (auto &socket : room.sockets)
                    {
                        if (socket == (clientSocket))
                        {
                            removeRoomBySocket(clientSocket);
                        }
                    }
                }
            }
        }
        // gui loi moi ket ban
        else if (clientInfo.arr[0] == "SEARCH_F")
        {

            std::string e, p;
            p = clientInfo.arr[1];
            e = clientInfo.arr[2];
            int checkSuccess = 0;
            struct demoAcc checkLogin = input();

            for (int j = 1; j <= info; j++)
            {
                // std::cout << checkLogin.arr[j].email << "---" << checkLogin.arr[j].password << std::endl;
                if (e == checkLogin.arr[j].email)
                {
                    sendWrapper(clientSocket, "+OK||0", 7, 0);
                    checkSuccess++;

                    std::ofstream fout("client/" + e + "/" + e + "_request.txt", std::ios::app);
                    fout << p << "\n";
                    fout.close();

                    break;
                }
            }
            if (checkSuccess == 0)
            {
                sendWrapper(clientSocket, "-NO||0", 7, 0);
            }
        }

        // dong y ket ban
        else if (clientInfo.arr[0] == "ACCEPT_F")
        {

            std::string check = clientInfo.arr[3];
            std::string nameLogin = clientInfo.arr[1];
            std::string nameFriend = clientInfo.arr[2];
            if (check == "1")
            {
                std::ofstream fout("client/" + nameLogin + "/" + nameLogin + "_friend.txt", std::ios::app);
                fout << nameFriend << "\n";
                fout.close();

                std::ofstream fout1("client/" + nameFriend + "/" + nameFriend + "_friend.txt", std::ios::app);
                fout1 << nameLogin << "\n";
                fout1.close();
            }
            removeLineFromFile("client/" + nameLogin + "/" + nameLogin + "_request.txt", nameFriend);
            sendWrapper(clientSocket, "+OK||6", 7, 0);
        }

        // xem danh sach yeu cau ket ban
        else if (clientInfo.arr[0] == "LIST_R")
        {
            std::string nameClient;
            std::vector<std::string> friendList;
            std::vector<account> friendL;
            nameClient = clientInfo.arr[1];

            friendList = getLine("client/" + nameClient + "/" + nameClient + "_request.txt");
            friendL = inputCheck(friendList);
            if (friendL.size() > 0)
            {
                std::cout << "send friend list";
                sendWrapper(clientSocket, "+OK||6", 7, 0);
                sendListFriend(clientSocket, friendL);
            }
            else
            {
                sendWrapper(clientSocket, "-NO||-8", 7, 0);
            }
        }

        // xem danh sach ban be va trang thai
        else if (clientInfo.arr[0] == "LIST_F")
        {

            std::string nameClient;
            std::vector<std::string> friendList;
            std::vector<account> friendL;
            nameClient = clientInfo.arr[1];

            friendList = getLine("client/" + nameClient + "/" + nameClient + "_friend.txt");
            friendL = inputCheck(friendList);
            if (friendL.size() > 0)
            {
                std::cout << "send friend list";
                sendWrapper(clientSocket, "+OK||6", 7, 0);
                sendListFriend(clientSocket, friendL);
            }
            else
            {
                sendWrapper(clientSocket, "-NO||-8", 7, 0);
            }
        }
        else if (clientInfo.arr[0] == "HISTORY")
        {

            std::string nameClient;
            std::string listHis;
            std::string interHis = "+OK|r|r|";
            nameClient = clientInfo.arr[1];

            listHis = getLineString("client/" + nameClient + "/" + nameClient + "_trainHis.txt");
            listHis = interHis + listHis;
            std::cout << listHis << "\n";
            sendWrapper(clientSocket, listHis.c_str(), listHis.size(), 0);
        }

        // hien diem bang xep hang
        else if (clientInfo.arr[0] == "RANK")
        {

            struct demoAcc accRank = input();
            accListRank.clear();
            for (int k = 1; k < info; k++)
            {
                accListRank.push_back(accRank.arr[k]);
            }
            std::sort(accListRank.begin(), accListRank.end(), comparePlayers);
            std::cout << "send rank";
            sendGameRank(clientSocket, accListRank);
        }

        else if (clientInfo.arr[0] == "SCORE")
        {

            std::string score, roomname;
            score = clientInfo.arr[1];
            roomname = clientInfo.arr[2];
            std::string nameLoginClient = clientInfo.arr[3];
            std::string e, p;
            e = score;
            p = nameLoginClient;

            int checkSuccess = 0;
            // Kiểm tra thông tin từng phòng
            for (auto &room : gameRooms)
            {
                if (roomname == room.name)
                {
                    if (room.score1 == -1)
                    {
                        room.score1 = stoi(score);

                        // Xu ly doi thu disconnect
                        //
                        if (room.sockets.size() == 1)
                        {
                            sendWrapper(clientSocket, "+OK||C", 7, 0);
                        }
                    }
                    else
                    {
                        room.score2 = stoi(score);
                        if (room.sockets.size() == 2)
                        {
                            int temp1, temp2;
                            std::cout << "\ncheck score:  " << room.score1 << "++++" << room.score2 << "\n";
                            bool win = (room.score2 > room.score1);
                            bool draw = (room.score2 == room.score1);
                            std::cout << "Server tra ket qua\n"
                                      << win << "--" << draw << "\n";
                            for (auto &socket : room.sockets)
                            {
                                std::cout << socket << "--" << clientSocket << "\n";
                                if (socket == clientSocket)
                                {
                                    temp1 = clientSocket;
                                    if (draw)
                                        sendWrapper(clientSocket, "+OK||D", 7, 0);
                                    else if (win)
                                        sendWrapper(clientSocket, "+OK||W", 7, 0);
                                    else
                                        sendWrapper(clientSocket, "+OK||L", 7, 0);
                                }
                                else
                                {
                                    temp2 = socket;
                                    if (draw)
                                        sendWrapper(socket, "+OK||D", 7, 0);
                                    else if (win)
                                        sendWrapper(socket, "+OK||L", 7, 0);
                                    else
                                        sendWrapper(socket, "+OK||W", 7, 0);
                                }
                            }

                            removeRoomBySocket(temp1);
                            removeRoomBySocket(temp2);
                        }
                        else
                        {
                            sendWrapper(clientSocket, "+OK||C", 7, 0);
                        }
                    }
                }
            }
        }

        std::cout << "Received from client: " << clientInfo.arr[0] << "\n *********\n"
                  << clientSocket << std::endl;

        // Echo back to the client
        // sendWrapper(clientSocket, buffer, bytesRead, 0);
    }

    // Close the socket when done

    removeRoomBySocket(clientSocket);
    close(clientSocket);
}

int main()
{
    // Initialize Winsock
    // room room1;
    // room1.name = "Phong1";
    // room1.sockets.push_back(12345); // Thay thế bằng socket thực tế
    // room1.sockets.push_back(54321); // Thêm socket khác nếu cần
    // gameRooms.push_back(room1);

    // room room2;
    // room2.name = "Phong2";
    // room2.sockets.push_back(67890); // Thay thế bằng socket thực tế
    // gameRooms.push_back(room2);

    // Setup server socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cerr << "Error binding socket\n";
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1)
    {
        std::cerr << "Error listening on socket\n";
        close(serverSocket);
        return 1;
    }

    std::vector<std::thread> clientThreads;

    std::cout << "Server is listening on port " << SERVER_PORT << std::endl;

    while (true)
    {
        // Accept incoming connections
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressSize);

        if (clientSocket != -1)
        {
            // Handle the client in a separate thread
            clientThreads.emplace_back(handleClient, clientSocket);
        }

        // Remove finished threads
        clientThreads.erase(std::remove_if(clientThreads.begin(), clientThreads.end(),
                                           [](const std::thread &t)
                                           { return !t.joinable(); }),
                            clientThreads.end());
    }

    // Close all client sockets
    for (auto &thread : clientThreads)
    {
        thread.join();
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}