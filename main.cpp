#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
using namespace std;

// Структура для хранения информации о событии
struct Event {
    string time;
    int id;
    vector<string> body;
};

// Структура для хранения информации о столе
struct Table {
    int revenue = 0;
    int occupied_minutes = 0;
    string client = "";
};

// Парсинг времени из строки
int parse_time(const string &time_str) {
    int hours = stoi(time_str.substr(0, 2));
    int minutes = stoi(time_str.substr(3, 2));
    return hours * 60 + minutes;
}

// Преобразование времени из минут в строку
string format_time(int time) {
    int hours = time / 60;
    int minutes = time % 60;
    ostringstream oss;
    oss << setw(2) << setfill('0') << hours << ":"
        << setw(2) << setfill('0') << minutes;
    return oss.str();
}

// Печать ошибки
void print_error(const struct Event &event, const string &message) {
    cout << event.time << " " << event.id;
    for (const auto &item : event.body) {
        cout << " " << item;
    }
    cout << endl << event.time << " 13 " << message << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    ifstream infile(argv[1]);
    if (!infile.is_open()) {
        cerr << "Could not open the file." << endl;
        return 1;
    }

    int num_tables;
    string start_time_str, end_time_str;
    int hourly_rate;

    infile >> num_tables;
    infile >> start_time_str >> end_time_str;
    infile >> hourly_rate;

    int start_time = parse_time(start_time_str);
    int end_time = parse_time(end_time_str);

    vector<Table> tables(num_tables);
    map<string, int> clients;
    vector<string> queue;

    string line;
    getline(infile, line); // Считывание остатка строки после третьей строки

    vector<Event> events;
    while (getline(infile, line)) {
        istringstream iss(line);
        Event event;
        iss >> event.time >> event.id;
        string part;
        while (iss >> part) {
            event.body.push_back(part);
        }
        events.push_back(event);
    }

    cout << start_time_str << endl;

    for (const auto &event : events) {
        string event_time = event.time;
        int event_id = event.id;
        string client = event.body[0];
        int table_number = event.body.size() >= 2 ? stoi(event.body[1]) : -1;

        int current_time = parse_time(event_time);

        string from_event_12 = "";

        switch (event_id) {
            case 1: { // Клиент пришел
                if (current_time < start_time || current_time >= end_time) {
                    print_error(event, "NotOpenYet");
                    continue;
                }
                if (clients.find(client) != clients.end()) {
                    print_error(event, "YouShallNotPass");
                    continue;
                }
                clients[client] = -1; // -1 означает, что клиент в клубе, но не за столом
                break;
            }
            case 2: { // Клиент сел за стол
                if (clients.find(client) == clients.end()) {
                    print_error(event, "ClientUnknown");
                    continue;
                }
                if (tables[table_number - 1].client != "" && tables[table_number - 1].client != client) {
                    print_error(event, "PlaceIsBusy");
                    continue;
                }
                if (tables[table_number - 1].client == client) {
                    continue; // Клиент уже сидит за столом
                }
                if (tables[table_number - 1].client != "") {
                    clients[tables[table_number - 1].client] = -1; // Освободить текущего клиента
                    int occupied_time = current_time - start_time;
                    tables[table_number - 1].occupied_minutes += occupied_time;
                    tables[table_number - 1].revenue += hourly_rate * ((occupied_time + 59) / 60);
                }
                tables[table_number - 1].client = client;
                clients[client] = current_time;
                break;
            }
            case 3: { // Клиент ожидает
                if (any_of(tables.begin(), tables.end(), [](const Table &t) { return t.client.empty(); })) {
                    print_error(event, "ICanWaitNoLonger!");
                    continue;
                }
                if (queue.size() >= num_tables) {
                    cout << event_time << " 11 " << client << endl;
                    clients.erase(client);
                    continue;
                }
                queue.push_back(client);
                break;
            }
            case 4: { // Клиент ушел
                if (clients.find(client) == clients.end()) {
                    print_error(event, "ClientUnknown");
                    continue;
                }

                for (auto &table : tables) {
                    if (table.client == client) {
                        int occupied_time = current_time - clients[client];
                        table.occupied_minutes += occupied_time;
                        table.revenue += hourly_rate * ((occupied_time + 59) / 60);
                        table.client = "";

                        if (!queue.empty()) {
                            string next_client = queue.front();
                            queue.erase(queue.begin());

                            table.client = next_client;
                            clients[next_client] = current_time;

                            ostringstream oss;
                            oss << event_time << " 12 " << next_client << " " << (&table - &tables[0] + 1) << endl;
                            from_event_12 = oss.str();
                        }
                        break;
                    }
                }
                clients.erase(client);
                break;
            }
        }
        cout << event_time << " " << event_id;
        for (const auto &item : event.body) {
            cout << " " << item;
        }
        cout << endl;

        if (from_event_12 != "") cout << from_event_12;
    }

    for (const auto &current_client : clients) {
        cout << end_time_str << " 11 " << current_client.first << endl;
    }

    cout << end_time_str << endl;

    for (int i = 0; i < num_tables; ++i) {
        if (tables[i].client != "") {
            int occupied_time = end_time - clients[tables[i].client];
            tables[i].occupied_minutes += occupied_time;
            tables[i].revenue += hourly_rate * ((occupied_time + 59) / 60);
            tables[i].client = "";
        }
    }

    for (int i = 0; i < num_tables; ++i) {
        cout << i + 1 << " " << tables[i].revenue << " " << setw(2) << setfill('0') << tables[i].occupied_minutes / 60 << ":" << setw(2) << setfill('0') << tables[i].occupied_minutes % 60 << endl;
    }

    return 0;
}