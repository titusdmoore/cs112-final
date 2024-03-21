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
#include <sstream>
#include <stdlib.h>

using namespace std;
namespace fs = std::filesystem;

const fs::path employeeDir = "employees";

/** 
 * PERMISSION CONSTANTS 
 * For now I have limited the permission system to these consts, but the permission value saved on employee supports more rigid control through bitwise operators.
 * 
 * Full permissions is 31 or 0b11111. This will allow users to Add, View, Seach, Modify, and Delete employees.
 *  - For now there will not be a way to add direct full permissions, because you should trust no one by default. If you want people to be able to do everything, you need to walk that path fully.
 *
 * Permission Breakdown:
 * 0
 * b
 * 1 - Allowed to delete employees, most destructive, highest permission.
 * 1 - Allowed to create employees.
 * 1 - Allowed to modify employees.
 * 1 - Allowed to view all employee records.
 * 1 - Allowed to view their own employee record.
 *
 */
const short HR_PERMS = 28; // 0b11100
const short MANAGEMENT_PERMS = 2; // 0b00010
const short GENERAL_PERMS = 1; // 0b00001

/**
 * @class Employee
 *
 * @description - This class is what handles all of the logic and data storage for employees
 *
 * @prop private id int - id of employee
 * @prop private permissions short - permissions of employee
 * @prop private password string - password of employee
 * @prop private file fs::path - file path of employee
 * @prop public firstName string - first name of employee
 * @prop public lastName string - last name of employee
 * @prop public username string - username of employee
 *
 * @method public write - Writes the current state of Employee to associated file. Will create file if not exists.
 * @method public isValidLogin - This function will check if the username and password provided are valid for the employee.
 * @method public static from - This function will read the contents of the file provided and create an instance of Employee from it.
 * @method public hasPermission - This function will check if the employee has the permission provided.
 * 
 */
class Employee {
    int id;
    short permissions;
    string password;
    fs::path file;

public:
    string firstName;
    string lastName;
    string username;

    /**
    * @function write
    *
    * @description - writes the current state of Employee to associated file. Will create file if not exists.
    *  - Employee file will be named after the employee's id.
    *  - File contents will be in the format of:
    *  - id username firstName lastName password permissions
    *
    * @return bool - indicates the success or failure of writing to employeeFile     
    *
    */
    bool write() {
        ostringstream oss;
        oss << this->id << ".txt";

        this->file = employeeDir/oss.str();
        
        ofstream employeeFile;
        employeeFile.open(oss.str(), ios::out | ios::trunc);

        // Something went wrong while creating or opening the file, we need to return false;
        if(!employeeFile) {
            return false;
        }
        
        oss.str(string());
        oss << this->id << "," << this->username << "," << this->firstName << "," << this->lastName << "," << this->password << "," << this->permissions;

        employeeFile << oss.str() << endl;  
        
        employeeFile.close();

        return true;
    }

    /**
     * @function isValidLogin
     *
     * @description - This function will check if the username and password provided are valid for the employee.
     *
     * @param string username - The username that the user has entered.
     * @param string password - The password that the user has entered.
     *
     * @return bool - Returns true if the login is valid, false otherwise.
     *
     */
    bool isValidLogin(string username, string password) {
        return this->username == username && this->password == password;
    }

    /**
    * @function from - static 
    *
    * @description - This function will read the contents of the file provided and create an instance of Employee from it. 
    *
    * @param file - file address of file that the employee will be built from.
    * @param employee - Pointer to the instance of employee that will be written to if file reading succeeds
    *
    * @return bool - returns true if file reading and employee creation was successful, false otherwise. 
    *
    */
    static bool from(fs::path employeeFile, Employee *employee) {
        ifstream file;
        file.open(employeeFile);

        // Something has gone wrong with getting the file, so we return false
        if(!file) {
            return false;
        }

        string contents;
        while(getline(file, contents)) {
            istringstream iss(contents);

            iss >> employee->id >> employee->username >> employee->firstName >> employee->lastName >> employee->password >> employee->permissions;
        }

        employee->file = employeeFile;

        file.close();

        return true;
    }

    /**
     * @function hasPermission 
     *
     * @description - This function will check if the employee has the permission provided.
     *
     * @param short permission - The permission that we are checking if the employee has.
     *
     * @return bool - Returns true if the employee has the permission, false otherwise.
     *
     */
    bool hasPermission(short permission) {
        return (this->permissions & permission) != 0;
    }
};

/* DECLARE FUNCTIONS */
void printScreenHeader(string screenName, int headerWidth = 44); 
void initializeApplication(fs::path employeeDir, vector<Employee> &employees);
bool login(string username, string password, vector<Employee> employees, Employee *employee);
void clearScreen();

int main() {
    // This will be the dir we set employee files, as well as the vector that tracks the application employees.
    vector<Employee> employees;
    Employee employee;

    initializeApplication(employeeDir, employees);
    clearScreen();

    /* START LOGIN */
    printScreenHeader("Welcome to FooBar Employee Management");
    cout << "***  Login to Continue  ***" << endl << endl;
    string username, password;


    while(true) {
        cout << "Username> ";
        getline(cin, username);

        cout << "Password> ";
        getline(cin, password);

        if(login(username, password, employees, &employee)) {
            break;  
        }

        cout << endl << "Invalid login, please try again." << endl;
    }

    clearScreen();
    ostringstream oss;

    oss << "Welcome " << employee.firstName << " " << employee.lastName << "!";
    printScreenHeader(oss.str());
    cout << "***  What do you need to do today?  ***" << endl << endl;

    // TODO: This is a very basic menu, we need to expand this to be more dynamic and based on the permissions of the user.
    if(employee.hasPermission(HR_PERMS) || employee.hasPermission(MANAGEMENT_PERMS)) {
        cout << "1. View Employees" << endl;
        cout << "2. Search Employees" << endl;
    } 
    if (employee.hasPermission(HR_PERMS)) {
        cout << "3. Add Employee" << endl;
        cout << "4. Remove Employee" << endl;
    } 
    if (employee.hasPermission(GENERAL_PERMS)) {
        cout << "5. View Your File" << endl;
    }
    cout << "6. Exit" << endl << endl;

    int choice;
    cout << "Choice> ";
    string input;
    cin >> input;
    istringstream iss(input);

    iss >> choice;

    cout << "You chose " << choice;
   
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
            string right(headerWidth - (left.length() + screenName.length()), ' ');
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
        Employee e;
        Employee::from(employeeFile.path(), &e);

        employees.push_back(e);
    }
}

/**
* @function login
*
* @description - This function will handle the login process for the application. 
*
* @param string username - The username that the user has entered.
* @param string password - The password that the user has entered.
* @param vector<Employee> employees - The vector of employees that the application is currently tracking.
* @param Employee *employee - Pointer to the employee that will be written to if the login is successful.
*
* @return bool - Returns true if the login was successful, false otherwise.
*
*/
bool login(string username, string password, vector<Employee> employees, Employee *employee) {
    for(auto e : employees) {
        if(e.isValidLogin(username, password)) {
            *employee = e;
            return true;
        }
    }

    return false;
}

void clearScreen() {
#if defined _WIN32
    system("cls");
#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
    system("clear");
#elif defined (__APPLE__)
    system("clear");
#endif
}
