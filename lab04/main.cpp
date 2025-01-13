#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <deque>
#include <ctime>
#include <iomanip>
#include <numeric>

#include "port.cpp"
using namespace std;

struct LogEntry {
    time_t timestamp;
    int value;
};

void write_to_log_file(const string& filename, const deque<LogEntry>& log_entries) {
    ofstream log(filename, ios::trunc);
    for (const auto& entry : log_entries) {
        log << put_time(localtime(&entry.timestamp), "%Y-%m-%d %H:%M:%S") << " " << entry.value << endl;
    }
}

double calculate_average(const deque<LogEntry>& log_entries) {
    if (log_entries.empty()) return 0.0;
    int sum = accumulate(log_entries.begin(), log_entries.end(), 0, [](int total, const LogEntry& entry) {
        return total + entry.value;
    });
    return static_cast<double>(sum) / log_entries.size();
}

int main() {
    string port_name;
#ifdef _WIN32  
    port_name = "COM2";
#else
    port_name = "/dev/pts/4";
#endif

    SerialPort port(port_name);
    deque<LogEntry> log_entries;
    deque<LogEntry> hourly_log_entries;
    deque<LogEntry> daily_log_entries;

    time_t last_hourly_log_time = time(nullptr);
    time_t last_daily_log_time = time(nullptr);

    while (true) {
        string value_str = port.readUntil('\n');
        int value = stoi(value_str);

        time_t now = time(nullptr);

        log_entries.push_back({now, value});

        // Удаление старых записей (старше 24 часов)
        while (!log_entries.empty() && difftime(now, log_entries.front().timestamp) > 24 * 3600) {
            log_entries.pop_front();
        }

        write_to_log_file("./log_file.txt", log_entries);

        // Обновление логов для средней температуры за час и за день
        hourly_log_entries.push_back({now, value});
        daily_log_entries.push_back({now, value});

        // Удаление старых записей (старше 1 часа) из hourly_log_entries
        while (!hourly_log_entries.empty() && difftime(now, hourly_log_entries.front().timestamp) > 3600) {
            hourly_log_entries.pop_front();
        }

        // Удаление старых записей (старше 1 дня) из daily_log_entries
        while (!daily_log_entries.empty() && difftime(now, daily_log_entries.front().timestamp) > 24 * 3600) {
            daily_log_entries.pop_front();
        }

        // Запись средней температуры за час в hourly_log.txt раз в час
        if (difftime(now, last_hourly_log_time) >= 3600) {
            double hourly_average = calculate_average(hourly_log_entries);
            ofstream hourly_log("./hourly_log.txt", ios::app);
            hourly_log << put_time(localtime(&now), "%Y-%m-%d %H:%M:%S") << " " << hourly_average << endl;
            last_hourly_log_time = now;

            // Удаление старых записей (старше 1 месяца) из hourly_log.txt
            ifstream hourly_log_read("./hourly_log.txt");
            deque<LogEntry> hourly_log_entries_file;
            string line;
            while (getline(hourly_log_read, line)) {
                istringstream iss(line);
                tm tm = {};
                int value;
                iss >> get_time(&tm, "%Y-%m-%d %H:%M:%S") >> value;
                time_t timestamp = mktime(&tm);
                if (difftime(now, timestamp) <= 30 * 24 * 3600) {
                    hourly_log_entries_file.push_back({timestamp, value});
                }
            }
            hourly_log_read.close();
            write_to_log_file("./hourly_log.txt", hourly_log_entries_file);
        }

        // Запись средней температуры за день в daily_log.txt раз в день
        if (difftime(now, last_daily_log_time) >= 24 * 3600) {
            double daily_average = calculate_average(daily_log_entries);
            ofstream daily_log("./daily_log.txt", ios::app);
            daily_log << put_time(localtime(&now), "%Y-%m-%d %H:%M:%S") << " " << daily_average << endl;
            last_daily_log_time = now;

            // Удаление старых записей (старше 1 года) из daily_log.txt
            ifstream daily_log_read("./daily_log.txt");
            deque<LogEntry> daily_log_entries_file;
            string line;
            while (getline(daily_log_read, line)) {
                istringstream iss(line);
                tm tm = {};
                int value;
                iss >> get_time(&tm, "%Y-%m-%d %H:%M:%S") >> value;
                time_t timestamp = mktime(&tm);
                if (difftime(now, timestamp) <= 365 * 24 * 3600) {
                    daily_log_entries_file.push_back({timestamp, value});
                }
            }
            daily_log_read.close();
            write_to_log_file("./daily_log.txt", daily_log_entries_file);
        }

        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}