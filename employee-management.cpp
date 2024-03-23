/**
 *   @file employee-management.cpp
 *
 *   @description This file contains the application code for an employee
 * management system.
 *
 *   @author Titus Moore
 *   @contact <titusdmoore@gmail.com>
 */

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdlib.h>
#include <unordered_map>
#include <vector>

// TODO: Remove this and use std::
using namespace std;
namespace fs = std::filesystem;

const fs::path EMPLOYEE_DIR = "employees";
const short HEADER_WIDTH = 44;

/**
 * PERMISSION CONSTANTS
 * For now I have limited the permission system to these consts, but the
 * permission value saved on employee supports more rigid control through
 * bitwise operators.
 *
 * Full permissions is 31 or 0b11111. This will allow users to Add, View, Seach,
 * Modify, and Delete employees.
 *  - For now there will not be a way to add direct full permissions, because
 * you should trust no one by default. If you want people to be able to do
 * everything, you need to walk that path fully.
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
const short HR_PERMS = 28;        // 0b11100
const short MANAGEMENT_PERMS = 2; // 0b00010
const short GENERAL_PERMS = 1;    // 0b00001

class Application;

/**
 * @class Employee
 *
 * @description - This class is what handles all of the logic and data storage
 * for employees
 *
 * @prop private id int - id of employee
 * @prop private permissions short - permissions of employee
 * @prop private password string - password of employee
 * @prop private file fs::path - file path of employee
 * @prop public firstName string - first name of employee
 * @prop public lastName string - last name of employee
 * @prop public username string - username of employee
 *
 * @method public write - Writes the current state of Employee to associated
 * file. Will create file if not exists.
 * @method public isValidLogin - This function will check if the username and
 * password provided are valid for the employee.
 * @method public static from - This function will read the contents of the file
 * provided and create an instance of Employee from it.
 * @method public hasPermission - This function will check if the employee has
 * the permission provided.
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

  Employee() {}
  Employee(int id, string firstName, string lastName, string username,
           string password, short permissions) {
    this->id = id;
    this->firstName = firstName;
    this->lastName = lastName;
    this->username = username;
    this->password = password;
    this->permissions = permissions;
  }

  /**
   * @function write
   *
   * @description - writes the current state of Employee to associated file.
   * Will create file if not exists.
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

    this->file = EMPLOYEE_DIR / oss.str();

    ofstream employeeFile;
    employeeFile.open(this->file, ios::out | ios::trunc);

    // Something went wrong while creating or opening the file, we need to
    // return false;
    if (!employeeFile) {
      return false;
    }

    oss.str(string());
    oss << this->id << " " << this->username << " " << this->firstName << " "
        << this->lastName << " " << this->password << " " << this->permissions;

    employeeFile << oss.str() << endl;

    employeeFile.close();

    return true;
  }

  /**
   * @function isValidLogin
   *
   * @description - This function will check if the username and password
   * provided are valid for the employee.
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
   * @description - This function will read the contents of the file provided
   * and create an instance of Employee from it.
   *
   * @param file - file address of file that the employee will be built from.
   * @param employee - Pointer to the instance of employee that will be written
   * to if file reading succeeds
   *
   * @return bool - returns true if file reading and employee creation was
   * successful, false otherwise.
   *
   */
  static bool from(fs::path employeeFile, Employee *employee) {
    ifstream file;
    file.open(employeeFile);

    // Something has gone wrong with getting the file, so we return false
    if (!file) {
      return false;
    }

    string contents;
    while (getline(file, contents)) {
      istringstream iss(contents);

      iss >> employee->id >> employee->username >> employee->firstName >>
          employee->lastName >> employee->password >> employee->permissions;
    }

    employee->file = employeeFile;

    file.close();

    return true;
  }

  /**
   * @function hasPermission
   *
   * @description - This function will check if the employee has the permission
   * provided.
   *
   * @param short permission - The permission that we are checking if the
   * employee has.
   *
   * @return bool - Returns true if the employee has the permission, false
   * otherwise.
   *
   */
  bool hasPermission(short permission) {
    return (this->permissions & permission) != 0;
  }
};

class Screen {
public:
  string name;
  string headerText;
  short headerWidth;

  virtual ~Screen() = 0;

  void display() {
    this->printScreenHeader();
    this->renderScreenBody();
    this->renderInteractiveContent();
  }

  virtual void renderScreenBody() = 0;
  virtual void renderInteractiveContent() = 0;

  /**
   * @function printScreenHeader
   *
   * @description This function will calculate the height needed to provide
   * spacing around the title, and will print via cout.
   *
   * @return void
   */
  void printScreenHeader() {
    // To calc height we need to divide to find the number of lines needed, we
    // want a minimum of 5 lines. We include 2 blank spaces on both sides
    // horizontally, then we also want a blank line above and below screen.
    int lineCount =
        (int)ceil((float)this->headerText.length() / (this->headerWidth - 4));
    int height = max(lineCount + 2, 5);

    // This is the index in which we need to start printing the screenName
    int startIndex = round(((float)height / 2) - floor(lineCount / 2)) - 1;

    for (int i = 0; i < height; ++i) {
      if (i == 0 || i == height - 1) {
        string s(headerWidth, '*');

        cout << s << endl;
        continue;
      }

      // For now we are going to support one line, here we are going to build
      // the line where we provide space around the word.
      if (i == startIndex) {
        string left(ceil((headerWidth - this->headerText.length()) / 2.0), ' ');
        string right(headerWidth - (left.length() + this->headerText.length()),
                     ' ');
        left[0] = '*';
        right[right.length() - 1] = '*';

        cout << left << this->headerText << right << endl;
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

  static void clearScreen() {
#if defined _WIN32
    system("cls");
#elif defined(__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
    system("clear");
#elif defined(__APPLE__)
    system("clear");
#endif
  }
};
Screen::~Screen(){};

class LoginScreen : public Screen {
  Application *app;

public:
  void renderInteractiveContent() override;
  LoginScreen(Application *a) : app(a) {
    name = "login";
    headerText = "Welcome to FooBar Employee Management";
    headerWidth = HEADER_WIDTH;
  }

  void renderScreenBody() override {
    cout << "***  Login to Continue  ***" << endl << endl;
  }
};

class Application {
  Employee employee;
  unordered_map<string, unique_ptr<Screen>> screens;
  vector<Employee> employees;

  void loadScreens() {
    unique_ptr<Screen> loginScreen = std::make_unique<LoginScreen>(this);
    this->screens[loginScreen->name] = std::move(loginScreen);
  }

public:
  Application() {
    // We check if path exists and if not, we create. If we get into the if we
    // can return becuase we know that there are no employee files.
    if (!fs::exists(EMPLOYEE_DIR)) {
      fs::create_directory(EMPLOYEE_DIR);
      /* THIS IS ONLY FOR TESTING/GRADING: Create file for inital login.
       * - Username: testing
       * - Password: password
       */
      Employee newEmployee(1, "Titus", "Moore", "testing", "password",
                           HR_PERMS | MANAGEMENT_PERMS | GENERAL_PERMS);
      newEmployee.write();
    }

    // Iterates through employee directory, will call Employee::from to get an
    // instance of Employee to add to employees vector.
    for (const auto &employeeFile : fs::directory_iterator(EMPLOYEE_DIR)) {
      Employee e;
      Employee::from(employeeFile.path(), &e);

      employees.push_back(e);
    }

    loadScreens();
  }

  /**
   * @function start
   *
   * @description - This function is run after everything has been setup, and
   * handles loading the first screen.
   *
   * @return void
   *
   */
  void start() { this->screens.at("login")->display(); }

  /**
   * @function start
   *
   * @description - This function is run after everything has been setup, and
   * handles loading the first screen.
   *
   * @return void
   *
   */
  void navigateToScreen(string screenName) {
    if (this->screens.count(screenName) != 0) {
      this->screens.at(screenName);
    }
  }

  /**
   * @function login
   *
   * @description - This function will handle the login process for the
   * application.
   *
   * @param string username - The username that the user has entered.
   * @param string password - The password that the user has entered.
   *
   * @return bool - Returns true if the login was successful, false otherwise.
   *
   */
  bool login(string username, string password) {
    for (auto e : this->employees) {
      if (e.isValidLogin(username, password)) {
        this->employee = e;
        return true;
      }
    }

    return false;
  }
};

// define function from loginscreen now since we know about app.
void LoginScreen::renderInteractiveContent() {
  /* START LOGIN */
  string username, password;

  while (true) {
    cout << "Username> ";
    getline(cin, username);

    cout << "Password> ";
    getline(cin, password);

    if (this->app->login(username, password)) {
      break;
    }

    cout << endl << "Invalid login, please try again." << endl;
  }
}

int main() {
  Screen::clearScreen();
  Application app;
  app.start();

  //
  // ostringstream oss;
  //
  // oss << "Welcome " << employee.firstName << " " << employee.lastName << "!";
  // printScreenHeader(oss.str());
  // cout << "***  What do you need to do today?  ***" << endl << endl;
  //
  // // TODO: This is a very basic menu, we need to expand this to be more
  // dynamic
  // // and based on the permissions of the user.
  // if (employee.hasPermission(HR_PERMS) ||
  //     employee.hasPermission(MANAGEMENT_PERMS)) {
  //   cout << "1. View Employees" << endl;
  //   cout << "2. Search Employees" << endl;
  // }
  // if (employee.hasPermission(HR_PERMS)) {
  //   cout << "3. Add Employee" << endl;
  //   cout << "4. Remove Employee" << endl;
  // }
  // if (employee.hasPermission(GENERAL_PERMS)) {
  //   cout << "5. View Your File" << endl;
  // }
  // cout << "6. Exit" << endl << endl;
  //
  // int choice;
  // cout << "Choice> ";
  // string input;
  // cin >> input;
  // istringstream iss(input);
  //
  // iss >> choice;

  return 0;
}
