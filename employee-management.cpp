/**
*   @file employee-management.cpp
*
*   @description This file contains the application code for an employee management system. 
*
*   @author Titus Moore
*   @contact <titusdmoore@gmail.com>
*/

#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;
    
class Employee;

void printScreenHeader(string screenName, int headerWidth = 44); 
void generateInitialUsersVector(vector<Employee> employees);
void initializeApplication(fs::path employeeDir, vector<Employee> &employees);

int main() {
    // This will be the dir we set employee files, as well as the vector that tracks the application employees.
    const fs::path employeeDir = "employees";
    vector<Employee> employees;
    
    initializeApplication(employeeDir, employees);

    printScreenHeader("Welcome to FooBar Employee Management");
    cout << "***  Login to Continue  ***" << endl << endl;
    string username, password;

    while(true) {
        cout << "Username> ";
        getline(cin, username);

        cout << "Password> ";
        getline(cin, password);

        if(true) {
            break;  
        }
    }

    cout << username << " " << password << endl;

	return 0;
}

/**
* @function printScreenHeader
*
* @param string screenName - String value name of the screen to be placed into the formatted header and displayed.
* @param int headerWidth default=44 - Int value of the width of the header, does place sensible default.
*
* @description This function will take in the params noted, then will calculate the height needed to provide spacing around the title, and will print via cout.
*
* @return void
*/
void printScreenHeader(string screenName, int headerWidth) {
    // To calc height we need to divide to find the number of lines needed, we want a minimum of 5 lines.
    // We include 2 blank spaces on both sides horizontally, then we also want a blank line above and below screen.
    int lineCount = (int) ceil((float) screenName.length() / (headerWidth - 4));
    int height = max(lineCount +  2, 5);
    
    // This is the index in which we need to start printing the screenName
    int startIndex = round(((float) height / 2) - floor(lineCount / 2)) - 1;

    for(int i = 0; i < height; ++i) {
        if(i == 0 || i == height - 1) {
            string s(headerWidth, '*');

            cout << s << endl;
            continue;
        }

        // For now we are going to support one line, here we are going to build the line where we provide space around the word.
        if(i == startIndex) {
            string left(ceil((headerWidth - screenName.length()) / 2.0), ' ');
            string right(left.length() - 1, ' ');
            left[0] = '*';
            right[right.length() - 1] = '*';

            cout << left << screenName << right << endl;
            continue;
        }

        string s(headerWidth, ' ');
        s[0] = '*';
        s[s.length() - 1] = '*';

        cout << s << endl;
    }

    // I want a newline after the title
    cout << endl;
}

// This may not actually not be needed anymore
void generateInitialUsersVector(vector<Employee> employees) {
    return; 
}

/**
* @function initializeApplication 
*
* @description - Run once at the beginning of the application, handles reading the employees directory and populating the employees vector as needed. 
*  - If employeeDir does not exist, it will be created.
*
* @param fs::path employeeDir - The directory in which we are storing the employee files.
* @param vector<Employee> *employees - Reference to the application vector used to keep track of employees.
*
* @return void
*
*/
void initializeApplication(fs::path employeeDir, vector<Employee> &employees) {
    // We check if path exists and if not, we create. If we get into the if we can return becuase we know that there are no employee files.
    if(!fs::exists(employeeDir)) {
        fs::create_directory(employeeDir);
        return;
    }
    
    // Iterates through employee directory, will call Employee::from to get an instance of Employee to add to employees vector.
    for(const auto& employeeFile : fs::directory_iterator(employeeDir)) {
        cout << employeeFile.path().stem().string() << endl;
    }
}

class Employee {
    int id;
    short permissions;
    string password;
    fs::path file;

public:
    string name;
    string username;

    Employee(string employeeName, string employeeUsername, string employeePassword) {
        name = employeeName;
    }

    /**
    * @function write
    *
    * @description - writes the current state of Employee to associated file. Will create file if not exists.
    *
    * @return void
    *
    */
    void write() {
        return;
    }

    /**
    * @function from - static 
    *
    * @description - writes the current state of Employee to associated file. Will create file if not exists.
    *
    * @param file - file address of file that the employee will be built from.
    * @param employee - Pointer to the instance of employee that will be written to if file reading succeeds
    *
    * @return bool - returns true if file reading and employee creation was successful, false otherwise. 
    *
    */
    static bool from(fs::path employeeFile, Employee *employee) {
        return false;
    }
};
