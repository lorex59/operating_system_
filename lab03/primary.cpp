#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;

// Класс позволяет проверить, является ли программа первой запущеной.
// Если да - создаёт временный файл который будет являться признаком первенства.
class SingleInstance {
private:
    string temp_file_path;
    bool is_primary_instance;

    void create_temp_file() {
        ofstream temp_file(temp_file_path);
        if (!temp_file.is_open()) {
            throw runtime_error("Failed to create temporary file.");
        }
        temp_file.close();
    }

    void delete_temp_file() {
        if (filesystem::exists(temp_file_path)) {
            filesystem::remove(temp_file_path);
        }
    }

public:
    explicit SingleInstance(const string& temp_file_name)
        : temp_file_path(temp_file_name), is_primary_instance(false) {
        if (filesystem::exists(temp_file_path)) {
            is_primary_instance = false; 
        } else {
            create_temp_file();
            is_primary_instance = true;
        }
    }

    ~SingleInstance() {
        if (is_primary_instance) {
            delete_temp_file(); 
        }
    }

    bool isPrimary() const {
        return is_primary_instance;
    }
};