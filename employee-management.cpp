/**
 *   @file employee-management.cpp
 *
 *   @description This file contains the application code for an employee
 *   management system. A CLI based application where users can log in and view, add,
 *   modify, and delete employee records. Employees are stored in a directory called
 *   employees, and each employee is stored in a file named after their id. The file
 *   contains the employee's id, username, first name, last name, password, and
 *   permissions. The permissions are stored as a short, and they control what the
 *   employee can do in the application. 
 * 
 *   @devnote This file 1200% should be broken up into multiple files. But since the problem 
 *   upload wants a single file, I have to keep it all in one file. 
 * 
 *   @devnote To login on initial run, you can use the following credentials:
 *   - username: testing 
 *   - password: password 
 *
 *   @author Titus Moore <titusmoore.dev>
 *   @contact <titusdmoore@gmail.com>
 *   @repo https://github.com/titusdmoore/cs112-final
 *   @date 2024
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
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

const fs::path EMPLOYEE_DIR = "employees";
const short HEADER_WIDTH = 44;

struct MenuOption
{
    short menuPosition;
    std::string screenName;
    std::string name;
};

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
 * @prop private permissions short - permissions of employee
 * @prop private password string - password of employee
 * @prop public id int - id of employee
 * @prop public firstName string - first name of employee
 * @prop public lastName string - last name of employee
 * @prop public username string - username of employee
 * @prop public file fs::path - file path of employee
 *
 * @method public write - Writes the current state of Employee to associated
 * file. Will create file if not exists.
 * @method public isValidLogin - This function will check if the username and
 * password provided are valid for the employee.
 * @method public static from - This function will read the contents of the file
 * provided and create an instance of Employee from it.
 * @method public hasPermission - This function will check if the employee has
 * the permission provided.
 * @method public updatePassword - This function will update the password of the
 * employee.
 * @method public updatePermissions - This function will update the permissions
 * of the employee.
 *
 */
class Employee
{
    short permissions;
    std::string password;

public:
    int id;
    std::string firstName;
    std::string lastName;
    std::string username;
    fs::path file;

    Employee() {}
    Employee(int id, std::string firstName, std::string lastName, std::string username,
             std::string password, short permissions)
    {
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
    bool write()
    {
        std::ostringstream oss;
        oss << this->id << ".txt";

        this->file = EMPLOYEE_DIR / oss.str();

        std::ofstream employeeFile;
        employeeFile.open(this->file, std::ios::out | std::ios::trunc);

        // Something went wrong while creating or opening the file, we need to
        // return false;
        if (!employeeFile)
        {
            return false;
        }

        oss.str(std::string());
        oss << this->id << " " << this->username << " " << this->firstName << " "
            << this->lastName << " " << this->password << " " << this->permissions;

        employeeFile << oss.str() << std::endl;

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
    bool isValidLogin(std::string username, std::string password)
    {
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
    static bool from(fs::path employeeFile, Employee *employee)
    {
        std::ifstream file;
        file.open(employeeFile);

        // Something has gone wrong with getting the file, so we return false
        if (!file)
        {
            return false;
        }

        std::string contents;
        while (getline(file, contents))
        {
            std::istringstream iss(contents);

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
    bool hasPermission(short permission)
    {
        return (this->permissions & permission) != 0;
    }

    /**
     * @function toString
     *
     * @description - Takes a strategy parameter and returns a string with the
     * employee information.
     *
     * @param short mode - number value of what type of display
     *  - 0 - Default, short list based display "id: First Last, Username"
     *  - 1 - Page version of the display, that contains all the public
     * information for a employee
     *
     *  @returns string - built string of display for the employee
     */
    std::string toString(short mode)
    {
        std::ostringstream oss;

        switch (mode)
        {
        case 1:
            oss << "ID: " << this->id << std::endl
                << "Name: " << this->firstName << " " << this->lastName << std::endl
                << "Username: " << this->username << std::endl;
            break;
        default:
            oss << this->id << ": " << this->firstName << " " << this->lastName
                << ", " << this->username << std::endl;
            break;
        }

        return oss.str();
    }

    /**
    * @function updatePassword
    *
    * @description - This function will update the password of the employee.
    * 
    * @param string password - The new password that the employee will have.
    * 
    * @return void
    * 
    */
    void updatePassword(std::string password)
    {
        this->password = password;
    }

    /**
    * @function updatePermissions
    *
    * @description - This function will update the permissions of the employee.
    * 
    * @param int permissions - The new permissions that the employee will have.
    * 
    * @return void   
    */
    void updatePermissions(int permissions)
    {
        this->permissions = permissions;
    }
};


/**
 * @class Screen
 * 
 * @description - This class is a abstract class that will be used to create
 * screens for the application. It will provide a base for the screens to be
 * built from.
 * 
 * @prop name - The name of the screen.
 * @prop headerText - The text that will be displayed in the header of the screen.
 * @prop headerWidth - The width of the header of the screen.
 * 
 * @method display - This function will be used to display the screen to the user.
 * @method renderScreenBody - Pure virtual function that will be used to render
 * the body of the screen.
 * @method renderInteractiveContent - Pure virtual function that will be used to render
 * the interactive content of the screen.
 * @method printScreenHeader - This function will be used to print the header of the screen.
 * @method clearScreen - This function will be used to clear the screen.
 * 
*/
class Screen
{
public:
    std::string name;
    std::string headerText;
    short headerWidth;

    virtual ~Screen() = 0;

    /**
     * @function display
     * 
     * @description - This function will be used to display the screen to the user.
     * 
     * @return void
     * 
    */
    void display()
    {
        Screen::clearScreen();

        this->printScreenHeader();
        this->renderScreenBody();
        this->renderInteractiveContent();
    }

    /**
     * @function renderScreenBody - pure virtual
     *
     * @description - This function will be used to render the body of the screen. Which is 
     * displayed just prior to interactive content.
     * 
     * @return void
     */
    virtual void renderScreenBody() = 0;

    /**
     * @function renderInteractiveContent - pure virtual
     *
     * @description - This function will be used to render the interactive content of the screen.
     * 
     * @return void
     */
    virtual void renderInteractiveContent() = 0;

    /**
     * @function printScreenHeader
     *
     * @description This function will calculate the height needed to provide
     * spacing around the title, and will print via cout.
     *
     * @return void
     */
    virtual void printScreenHeader()
    {
        // To calc height we need to divide to find the number of lines needed, we
        // want a minimum of 5 lines. We include 2 blank spaces on both sides
        // horizontally, then we also want a blank line above and below screen.
        int lineCount =
            (int)ceil((float)this->headerText.length() / (this->headerWidth - 4));
        int height = std::max(lineCount + 2, 5);

        // This is the index in which we need to start printing the screenName
        int startIndex = round(((float)height / 2) - floor(lineCount / 2)) - 1;

        for (int i = 0; i < height; ++i)
        {
            if (i == 0 || i == height - 1)
            {
                std::string s(headerWidth, '*');

                std::cout << s << std::endl;
                continue;
            }

            // For now we are going to support one line, here we are going to build
            // the line where we provide space around the word.
            if (i == startIndex)
            {
                std::string left(ceil((headerWidth - this->headerText.length()) / 2.0), ' ');
                std::string right(headerWidth - (left.length() + this->headerText.length()),
                             ' ');
                left[0] = '*';
                right[right.length() - 1] = '*';

                std::cout << left << this->headerText << right << std::endl;
                continue;
            }

            std::string s(headerWidth, ' ');
            s[0] = '*';
            s[s.length() - 1] = '*';

            std::cout << s << std::endl;
        }

        // I want a newline after the title
        std::cout << std::endl;
    }

    /**
     * @function clearScreen
     * 
     * @description - This function will be used to clear the screen.
     * 
     * @return void
    */
    static void clearScreen()
    {
#if defined _WIN32
        system("cls");
#elif defined(__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
        system("clear");
#elif defined(__APPLE__)
        system("clear");
#endif
    }
};
// Destructor
Screen::~Screen(){};

/**
 * @class LoginScreen
 * 
 * @description - This class will be used to create the login screen for the application.
 * 
 * @prop private Application *app - The application object.
 * 
 * @method public LoginScreen(Application *a) - The constructor for the login screen.
 * @method public renderInteractiveContent - This function will be used to render the interactive content of the screen.
 * @method public renderScreenBody - This function will be used to render the body of the screen.
 * 
*/
class LoginScreen : public Screen
{
    Application *app;

public:
    void renderInteractiveContent() override;
    LoginScreen(Application *a) : app(a)
    {
        name = "login";
        headerText = "Welcome to FooBar Employee Management";
        headerWidth = HEADER_WIDTH;
    }

    void renderScreenBody() override
    {
        std::cout << "***  Login to Continue  ***" << std::endl
             << std::endl;
    }
};

/**
 * @class MenuOption
 * 
 * @description - This class will be used to create menu options for the menu screen.
 * 
 * @prop private Application *app - The application object.
 * @prop private vector<string> options - The options that will be displayed to the user.
 * @prop private bool optionsInitialized - A flag to determine if the options have been initialized. 
 * This is due to them needing the application object.
 * 
 * @method public MenuOption(Application *a) - Constructor that takes the application object.
 * @method public void buildMenuOptions() - This function will be used to build the menu options.
 * 
*/
class MenuScreen : public Screen
{
    Application *app;
    std::vector<MenuOption> options;
    bool optionsInitialized;

public:
    MenuScreen(Application *a) : app(a)
    {
        this->name = "menu";
        this->headerWidth = HEADER_WIDTH;
        this->headerText = "testing";
        this->optionsInitialized = false;
    }

    void prePrintHeader();
    void renderInteractiveContent() override;
    void buildMenuOptions();

    /**
     * @function renderScreenHeader
     * 
     * @description - This function will be used to render the header of the screen. 
     * This is an override, that calls the base implementation.
     * 
     * @return void
    */
    void printScreenHeader() override
    {
        this->prePrintHeader();

        Screen::printScreenHeader();
    }

    /**
     * @function renderScreenBody
     * 
     * @description - This function will be used to render the body of the screen.
     * 
     * @return void
    */
    void renderScreenBody() override
    {
        std::cout << "***  What do you need to do today?  ***" << std::endl
             << std::endl;
    }
};

/**
 * 
 * @class ListScreen
 * 
 * @description - This class will be used to create the list screen for the application. 
 * Is also used for search results and listing on the remove page.
 * 
 * @prop private Application *app - The application object.
 * @prop public vector<Employee> employees - The employees that will be displayed on the screen.
 * @prop public bool employeesOverriden - A flag to determine if the employees have been overriden. 
 * If true use this class's employees. Otherwise use the application's employees.
 * @prop private bool isRemove - A flag to determine if this is the remove screen.
 * 
 * @method public ListScreen(Application *a) - The constructor for the list screen.
 * @method public ListScreen(Application *a, string searchQuery, vector<Employee> employees) - 
 * The constructor for the list screen with search results.
 * @method public ListScreen(Application *a, bool isRemove) - The constructor for the list screen for the remove screen.
 * @method public void renderInteractiveContent - This function will be used to render the interactive content of the screen.
 * @method public vector<Employee> getEmployees - This function will be used to get the employees. 
 * This is where the employeesOverriden flag is used.
 * 
 * 
*/
class ListScreen : public Screen
{
    bool isRemove;
    Application *app;

public:
    std::vector<Employee> employees;
    bool employeesOverriden;

    void renderInteractiveContent() override;
    std::vector<Employee> getEmployees();

    ListScreen(Application *a) : app(a)
    {
        name = "list";
        headerText = "Viewing All Employees";
        headerWidth = HEADER_WIDTH;
        employeesOverriden = false;
        isRemove = false;
    }

    ListScreen(Application *a, std::string searchQuery, std::vector<Employee> employees) : app(a)
    {
        name = "search-list";

        std::ostringstream oss;
        oss << "Showing employees like \"" << searchQuery << "\"";
        headerText = oss.str();
        headerWidth = HEADER_WIDTH;

        this->employees = employees;
        employeesOverriden = true;
        isRemove = false;
    }

    ListScreen(Application *a, std::string remove) : app(a)
    {
        name = "list";
        headerText = "Remove Employee";
        headerWidth = HEADER_WIDTH;
        isRemove = true;
        employeesOverriden = false;
    }

    void renderScreenBody() override
    {
        if (this->isRemove)
        {
            std::cout << "***  Insert Id of Employee to Remove ***" << std::endl
                 << std::endl;
        }
        else
        {
            std::cout << "***  Insert Id of Employee to Edit/View  ***" << std::endl
                 << std::endl;
        }
    }
};

/**
 * @class SearchScreen
 * 
 * @description - This class will be used to create the search screen for the application.
 * 
 * @prop private Application *app - The application object.
 * 
 * @method public SearchScreen(Application *a) - The constructor for the search screen.
 * @method public void renderInteractiveContent - This function will be used to render the interactive content of the screen.
 * 
*/
class SearchScreen : public Screen
{
    Application *app;

public:
    void renderInteractiveContent() override;
    SearchScreen(Application *a) : app(a)
    {
        name = "search";
        headerText = "Search Employees";
        headerWidth = HEADER_WIDTH;
    }

    void renderScreenBody() override
    {
        std::cout << "***  Insert Search Query by names, or username to Search ***" << std::endl
             << std::endl;
    }
};

/**
 * @class AddEmployeeScreen
 * 
 * @description - This class will be used to create the add employee screen for the application.
 * 
 * @prop private Application *app - The application object.
 * 
 * @method public AddEmployeeScreen(Application *a) - The constructor for the add employee screen.
 * @method public void renderInteractiveContent - This function will be used to render the interactive content of the screen.
 * 
*/
class AddEmployeeScreen : public Screen
{
    Application *app;

public:
    void renderInteractiveContent() override;
    AddEmployeeScreen(Application *a) : app(a)
    {
        name = "add";
        headerText = "Add a new Employee";
        headerWidth = HEADER_WIDTH;
    }

    void renderScreenBody() override
    {
        std::cout << "***  Answer prompts to add new employee.  ***" << std::endl
             << std::endl;
    };
};

/**
 * @class EditScreen
 * 
 * @description - This class will be used to create the edit screen for the application.
 * 
 * @prop private Application *app - The application object.
 * @prop private Employee *employee - The employee object.
 * 
 * @method public EditScreen(Application *a, Employee *employee) - The constructor for the edit screen.
 * @method public void renderInteractiveContent - This function will be used to render the interactive content of the screen.
 * 
*/
class EditScreen : public Screen
{
    Application *app;
    Employee *employee;

public:
    void renderInteractiveContent() override;

    EditScreen(Application *a, Employee *employee) : app(a)
    {
        name = "edit";
        headerText = "Edit Employee";
        headerWidth = HEADER_WIDTH;
        this->employee = employee;
    }

    void renderScreenBody() override
    {
        std::cout << "***  Answer prompts to employee information (Leave blank for no change).  ***" << std::endl
             << std::endl;
    };
};

/**
 * @class FileScreen
 * 
 * @description - This class will be used to create the file screen for the application.
 * 
 * @prop private Application *app - The application object.
 * @prop private Employee *employee - The employee object.
 * @prop private bool employeeOverriden - The flag to check if the employee object is overriden.
 * 
 * @method public FileScreen(Application *a) - The constructor for the file screen.
 * @method public FileScreen(Application *a, Employee *employee) - The constructor for the file screen with a specific employee.
*/
class FileScreen : public Screen
{
    Application *app;
    Employee *employee;
    bool employeeOverriden;

public:
    void renderInteractiveContent() override;
    Employee *getEmployee();
    FileScreen(Application *a) : app(a)
    {
        name = "file";
        headerText = "Viewing Your Profile";
        headerWidth = HEADER_WIDTH;
    }

    FileScreen(Application *a, Employee *employee) : app(a)
    {
        name = "specific file";
        headerText = "Viewing Profile";
        headerWidth = HEADER_WIDTH;
        this->employee = employee;
        employeeOverriden = true;
    }

    void renderScreenBody() override{};
};

/**
 * @class Application
 * 
 * @description - This class will be used to create the application object for the application.
 * 
 * @prop private Employee employee - The employee that is signed in object.
 * @prop private std::unordered_map<std::string, std::unique_ptr<Screen>> screens - The screens object.
 * @prop public employees - List of all employees being tracked by the application. 
 * @prop public int currentId - The current highest id of the employees.
 * 
 * @method public void loadScreens - This function will be used to load all the screens for the application.
 * @method public void run - This function will be used to run the application.
 * @method public void navigateToScreen - This function will be used to navigate to a specific screen.
 * @method public Employee *getLoggedInEmployee - This function will be used to get the employee object.
 * @method public void login - This function will be used to login the employee.
 * @method public Employee *findEmployeeById - This function will be used to find an employee by id.
 * @method public void removeEmployeeById - This function will be used to remove an employee by id.
 * @method public bool searchMatch - Takes in input string and query and does a case insensitive search.
 * @method public bool uniqueUsername - This function will be used to check if the username is unique. 
 * Two signatures, one with a string that will compare all users, and one that takes in an int to skip 
 * comparison of by employee id.
 * 
*/
class Application
{
    Employee employee;
    std::unordered_map<std::string, std::unique_ptr<Screen>> screens;

    void loadScreens()
    {
        std::unique_ptr<Screen> loginScreen = std::make_unique<LoginScreen>(this);
        this->screens[loginScreen->name] = std::move(loginScreen);

        std::unique_ptr<Screen> menuScreen = std::make_unique<MenuScreen>(this);
        this->screens[menuScreen->name] = std::move(menuScreen);

        std::unique_ptr<Screen> listScreen = std::make_unique<ListScreen>(this);
        this->screens[listScreen->name] = std::move(listScreen);

        std::unique_ptr<Screen> searchScreen = std::make_unique<SearchScreen>(this);
        this->screens[searchScreen->name] = std::move(searchScreen);

        std::unique_ptr<Screen> addEmployeeScreen =
            std::make_unique<AddEmployeeScreen>(this);
        this->screens[addEmployeeScreen->name] = std::move(addEmployeeScreen);

        std::unique_ptr<Screen> fileScreen = std::make_unique<FileScreen>(this);
        this->screens[fileScreen->name] = std::move(fileScreen);
    }

public:
    std::vector<Employee> employees;
    int currentId;

    Application()
    {
        this->currentId = 1;

        // We check if path exists and if not, we create. If we get into the if we
        // can return becuase we know that there are no employee files.
        if (!fs::exists(EMPLOYEE_DIR))
        {
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
        for (const auto &employeeFile : fs::directory_iterator(EMPLOYEE_DIR))
        {
            Employee e;
            Employee::from(employeeFile.path(), &e);

            try
            {
                int id = std::stoi(employeeFile.path().stem());

                if (id > this->currentId)
                {
                    this->currentId = id;
                }
            }
            catch (...)
            {
                std::cout << "Invalid file name." << std::endl;
            }

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
     * @function navigateToScreen
     *
     * @description - This function handles looking up a screen by key (name) and
     * printing it.
     *
     * @param screeenName string - The key of the screen to display.
     *
     * @return void
     *
     */
    void navigateToScreen(std::string screenName)
    {
        if (this->screens.count(screenName) != 0)
        {
            this->screens.at(screenName)->display();
        }
    }

    /**
     * @function getLoggedInEmployee
     * 
     * @description - This function will return the employee object that is currently logged in.
     * 
     * @return Employee * - The employee object that is currently logged in.
     * 
    */
    Employee *getLoggedInEmployee() { return &this->employee; }

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
    bool login(std::string username, std::string password)
    {
        for (auto e : this->employees)
        {
            if (e.isValidLogin(username, password))
            {
                this->employee = e;
                return true;
            }
        }

        return false;
    }

    /**
     * @function findEmployeeById
     * 
     * @description - This function will find an employee by their id.
     * 
     * @param int id - The id of the employee to find.
     * 
     * @return Employee * - The employee object that was found, or nullptr if not found.
     * 
    */
    Employee *findEmployeeById(int id)
    {
        for (auto &e : this->employees)
        {
            if (e.id == id)
            {
                return &e;
            }
        }

        return nullptr;
    }

    /**
     * 
     * @function removeEmployeeById
     * 
     * @description - This function will remove an employee by their id.
     * 
     * @param int id - The id of the employee to remove. Won't let you delete the currently logged in employee.
     * 
     * @return void 
    */
    void removeEmployeeById(int id)
    {
        // Prevent deleting currently logged in employee
        if (id == this->employee.id)
        {
            return;
        }

        for (auto it = this->employees.begin(); it != this->employees.end(); ++it)
        {
            if (it->id == id)
            {
                fs::remove(it->file);
                this->employees.erase(it);
                break;
            }
        }
    }

    /**
     * @function searchMatch
     * 
     * @description - This function will search for a query in a test string. Not case sensitive.
     * 
     * @param string test - The string to search in.
     * @param string query - The string to search for.
     * 
     * @return bool - Returns true if the query is found in the test string, false otherwise.
    */
    bool searchMatch(std::string test, std::string query)
    {
        std::transform(test.begin(), test.end(), test.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c)
                       { return std::tolower(c); });

        return test.find(query) != std::string::npos;
    }

    std::vector<Employee> searchEmployees(std::string query)
    {
        std::vector<Employee> out;

        for (auto e : this->employees)
        {
            if (this->searchMatch(e.firstName, query) || this->searchMatch(e.lastName, query) || this->searchMatch(e.username, query))
            {
                out.push_back(e);
            }
        }

        return out;
    }

    /**
     * @function uniqueUsername
     * 
     * @description - This function will check if a username is unique.
     * 
     * @param string username - The username to check.
     * 
     * @return bool - Returns true if the username is unique, false otherwise.
    */
    bool uniqueUsername(std::string username)
    {
        for (auto e : this->employees)
        {
            if (e.username == username)
            {
                return false;
            }
        }

        return true;
    }

    /**
     * @function uniqueUsername
     * 
     * @description - This function will check if a username is unique, but will skip a specific id.
     * 
     * @param string username - The username to check.
     * @param int skipId - The id to skip.
     * 
     * @return bool - Returns true if the username is unique, false otherwise.
    */
    bool uniqueUsername(std::string username, int skipId)
    {
        for (auto e : this->employees)
        {
            if (e.username == username && e.id != skipId)
            {
                return false;
            }
        }

        return true;
    }
};

/**
 * ******************************************************
 * DEFINE METHODS FOR SCREENS THAT DEPEND ON APPLICATION
 * ******************************************************
*/

/**
 * @function LoginScreen::renderInteractiveContent
 *
 * @description - This function prints two input prompts for the user to enter their
 * username and password. It will then attempt to login the user. If the login is successful,
 * it will navigate to the menu screen. 
 *
 * @return void
 *
 */
void LoginScreen::renderInteractiveContent()
{
    /* START LOGIN */
    std::string username, password;

    while (true)
    {
        std::cout << "Username> ";
        getline(std::cin, username);

        std::cout << "Password> ";
        getline(std::cin, password);

        if (this->app->login(username, password))
        {
            break;
        }

        std::cout << std::endl
             << "Invalid login, please try again." << std::endl;
    }

    // We are successful, request next screen.
    this->app->navigateToScreen("menu");
}

/**
 * @function MenuScreen::prePrintHeader
 *
 * @description - This function will set the header text to welcome the logged in employee.
 *
 * @return void
 *
 */
void MenuScreen::prePrintHeader()
{
    const Employee *employee = app->getLoggedInEmployee();
    std::ostringstream oss;

    oss << "Welcome " << employee->firstName << " " << employee->lastName << "!";
    this->headerText = oss.str();
}

void MenuScreen::buildMenuOptions()
{
    Employee *employee = app->getLoggedInEmployee();
    std::string screens[5][2] = {
        {"list", "View Employees"},
        {"search", "Search Employees"},
        {"add", "Add Employee"},
        {"remove", "Remove Employee"},
        {"file", "View Your File"}
    };

    // Loop through each screen and add it to the menu if the employee has permission.
    for (int i = 0; i < 5; ++i)
    {
        switch (i)
        {
        case 0:
        case 1:
            if (employee->hasPermission(HR_PERMS) ||
                employee->hasPermission(MANAGEMENT_PERMS))
            {
                MenuOption newOption;
                newOption.name = screens[i][1];
                newOption.menuPosition = this->options.size() + 1;
                newOption.screenName = screens[i][0];

                this->options.push_back(newOption);
            }
            break;
        case 2:
        case 3:
            if (employee->hasPermission(HR_PERMS))
            {
                MenuOption newOption;
                newOption.name = screens[i][1];
                newOption.menuPosition = this->options.size() + 1;
                newOption.screenName = screens[i][0];

                this->options.push_back(newOption);
            }
            break;
        default:
            if (employee->hasPermission(GENERAL_PERMS))
            {
                MenuOption newOption;
                newOption.name = screens[i][1];
                newOption.menuPosition = this->options.size() + 1;
                newOption.screenName = screens[i][0];

                this->options.push_back(newOption);
            }
        }
    }

    // Set flag to true so we don't rebuild the menu options.
    this->optionsInitialized = true;
}

/**
 * @function MenuScreen::renderInteractiveContent
 *
 * @description - This function will print the menu options to the screen and prompt the user
 * to make a selection. If the user selects an option, it will navigate to the appropriate screen.
 *
 * @return void
 *
 */
void MenuScreen::renderInteractiveContent()
{
    if (!this->optionsInitialized)
    {
        this->buildMenuOptions();
    }

    for (auto o : this->options)
    {
        std::cout << o.menuPosition << ". " << o.name << std::endl;
    }

    std::cout << std::endl
         << "0. Exit Application" << std::endl
         << std::endl;

    int choice;
    while (true)
    {
        std::cout << "Choice> ";
        std::string input;
        std::cin >> input;
        std::istringstream iss(input);
        iss >> choice;

        if (choice == 0 || (choice > 0 && choice - 1 < this->options.size()))
        {
            break;
        }

        std::cout << std::endl
             << "Please input a valid option." << std::endl;
    }

    if (choice == 0)
    {
        return;
    }

    // Remove
    if (this->options.at(choice - 1).screenName == "remove")
    {
        ListScreen removeScreen(this->app, "remove");
        removeScreen.display();
    }

    this->app->navigateToScreen(this->options.at(choice - 1).screenName);
}

/**
 * @function ListScreen::getEmployees
 * 
 * @description - This function will return a list of employees. If the employees have been overriden, it will return
 * the overriden list. Otherwise, it will return the list of employees from the application.
 * 
 * @return std::vector<Employee> - A list of employees.
*/
std::vector<Employee> ListScreen::getEmployees()
{
    if (this->employeesOverriden)
    {
        return this->employees;
    }

    return this->app->employees;
}

/**
 * @function ListScreen::renderInteractiveContent
 *
 * @description - This function will print a list of employees to the screen. If the screen is in remove mode,
 * it will allow the user to select an employee to remove. If the screen is in list mode, it will simply display
 * the list of employees.
 *
 * @return void
 *
 */
void ListScreen::renderInteractiveContent()
{
    for (auto e : this->getEmployees())
    {
        // This will prevent users from seeing their own account to delete
        if (!(this->isRemove && e.id == this->app->getLoggedInEmployee()->id))
        {
            std::cout << e.toString(0);
        }
    }

    std::cout << std::endl
         << "0. Return to Menu" << std::endl
         << std::endl;

    int id;
    Employee *employee;
    while (true)
    {
        std::cout << "Choice> ";
        std::string input;
        std::cin >> input;
        std::istringstream iss(input);
        iss >> id;

        if (!iss.fail())
        {
            employee = this->app->findEmployeeById(id);
            if (id == 0 || employee != nullptr)
            {
                break;
            }
        }

        std::cout << std::endl
             << "ID must be of type int." << std::endl;
    }

    if (id == 0)
    {
        this->app->navigateToScreen("menu");
    }

    if (this->isRemove)
    {
        // Build the remove screen
        this->app->removeEmployeeById(id);
        this->display();
    }
    else
    {
        // This is the default behavior for the list screen.
        FileScreen fileScreen(this->app, employee);
        fileScreen.display();
    }
}

/**
 * @function SearchScreen::renderInteractiveContent
 * 
 * @description - This function will prompt the user to input a search query. It will then search the employees
 * in the application for the query and display the results to the screen.
 * 
 * @return void
*/
void SearchScreen::renderInteractiveContent()
{
    /* START SEARCH */
    std::string query;

    std::cout << "Query> ";
    std::cin >> query;

    std::vector<Employee> results = this->app->searchEmployees(query);
    ListScreen searchList(this->app, query, results);
    searchList.display();
}

/**
 * 
 * @function AddEmployeeScreen::renderInteractiveContent
 * 
 * @description - This function will prompt the user to input information about a new employee. It will then
 * add the employee to the application and navigate back to the menu screen.
 * 
 * @return void
*/
void AddEmployeeScreen::renderInteractiveContent()
{
    std::string firstName, lastName, username, password;
    int isHR, isMan;

    std::cout << "First Name> ";
    std::cin >> firstName;

    std::cout << "Last Name> ";
    std::cin >> lastName;

    do
    {
        std::cout << "Username> ";
        std::cin >> username;
    } while (username.empty() || !this->app->uniqueUsername(username));

    std::cout << "Password> ";
    std::cin >> password;

    while (true)
    {
        std::cout << "Is employee hr? (0: no, 1: yes)> ";
        std::string input;
        std::cin >> input;
        std::istringstream iss(input);
        iss >> isHR;

        if (!iss.fail() && (isHR == 0 || isHR == 1))
        {
            break;
        }

        std::cout << std::endl
             << "Please input a valid option." << std::endl;
    }

    while (true)
    {
        std::cout << "Is employee management? (0: no, 1: yes)> ";
        std::string input;
        std::cin >> input;
        std::istringstream iss(input);
        iss >> isMan;

        if (!iss.fail() && (isMan == 0 || isMan == 1))
        {
            break;
        }

        std::cout << std::endl
             << "Please input a valid option." << std::endl;
    }

    this->app->currentId++;
    Employee e(this->app->currentId, firstName, lastName, username, password,
               (HR_PERMS * isHR) | (MANAGEMENT_PERMS * isMan) | GENERAL_PERMS);
    e.write();
    this->app->employees.push_back(e);

    this->app->navigateToScreen("menu");
}

/**
 * @function EditScreen::renderInteractiveContent
 * 
 * @description - This function will prompt the user to input information about an employee. It will then
 * update the employee in the application and navigate back to the menu screen.
 * 
 * @return void
*/
void EditScreen::renderInteractiveContent()
{
    std::string firstName, lastName, username, password;
    int isHR, isMan;

    // Clear cin because we want empty input
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "First Name (Current: " << this->employee->firstName << ")> ";
    getline(std::cin, firstName);

    std::cout << "Last Name (Current: " << this->employee->lastName << ")> ";
    getline(std::cin, lastName);

    do
    {
        std::cout << "Username (Current: " << this->employee->username << ")> ";
        getline(std::cin, username);

        if (username.empty())
        {
            break;
        }
    } while (!this->app->uniqueUsername(username, this->employee->id));

    std::cout << "Password> ";
    getline(std::cin, password);

    int currentHR = employee->hasPermission(HR_PERMS) ? 1 : 0;
    while (true)
    {
        std::cout << "Is employee hr? (0: no, 1: yes; Current: " << currentHR << ")> ";
        std::string input;
        std::cin >> input;
        std::istringstream iss(input);
        iss >> isHR;

        if (!iss.fail() && (isHR == 0 || isHR == 1))
        {
            break;
        }

        std::cout << std::endl
             << "Please input a valid option." << std::endl;
    }

    int currentMan = employee->hasPermission(MANAGEMENT_PERMS) ? 1 : 0;
    while (true)
    {
        std::cout << "Is employee management? (0: no, 1: yes; Current: " << currentMan << ")> ";
        std::string input;
        std::cin >> input;
        std::istringstream iss(input);
        iss >> isMan;

        if (!iss.fail() && (isMan == 0 || isMan == 1))
        {
            break;
        }

        std::cout << std::endl
             << "Please input a valid option." << std::endl;
    }

    bool dirty = false;
    if (!firstName.empty())
    {
        this->employee->firstName = firstName;
        dirty = true;
    }

    if (!lastName.empty())
    {
        this->employee->lastName = lastName;
        dirty = true;
    }

    if (!username.empty())
    {
        this->employee->username = username;
        dirty = true;
    }

    if (!password.empty())
    {
        this->employee->updatePassword(password);
        dirty = true;
    }

    this->employee->updatePermissions((HR_PERMS * isHR) | (MANAGEMENT_PERMS * isMan) | GENERAL_PERMS);

    if (dirty)
    {
        this->employee->write();
    }

    this->app->navigateToScreen("menu");
}

/**
 * @function FileScreen::getEmployee
 *
 * @description - This function will return the employee that the file screen is displaying. If the employee
 * is overriden, it will return the overriden employee. Otherwise, it will return the logged in employee.
 *
 * @return Employee* - The employee that the file screen is displaying.
 *
 */
Employee *FileScreen::getEmployee()
{
    if (this->employeeOverriden)
    {
        return this->employee;
    }

    return this->app->getLoggedInEmployee();
}

/**
 * @function FileScreen::renderInteractiveContent
 *
 * @description - This function will display the employee's file to the screen. It will also allow the user to
 * edit the employee's information if they have the appropriate permissions.
 *
 * @return void
 *
 */
void FileScreen::renderInteractiveContent()
{
    Employee *emp = this->getEmployee();
    std::cout << emp->toString(1);

    std::cout << std::endl
         << "0. Return to Menu";
    if (this->app->getLoggedInEmployee()->id != emp->id && this->app->getLoggedInEmployee()->hasPermission(HR_PERMS))
    {
        std::cout << std::endl
             << "1. Edit Employee";
    }
    std::cout << std::endl
         << std::endl;

    int choice;
    while (true)
    {
        std::cout << "Choice> ";
        std::string input;
        std::cin >> input;
        std::istringstream iss(input);
        iss >> choice;

        if (!iss.fail())
        {
            break;
        }

        std::cout << std::endl
             << "ID must be of type int." << std::endl;
    }

    if (choice == 1)
    {
        EditScreen editScreen(this->app, emp);
        editScreen.display();
    }
    else
    {
        this->app->navigateToScreen("menu");
    }
}

int main()
{
    Application app;
    app.start();

    return 0;
}
