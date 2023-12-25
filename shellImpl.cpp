#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <grp.h>
#include <iomanip>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <readline/history.h>
#include <readline/readline.h>
using namespace std;
//some needed global variables
string cwdPath = ""; 
string append = "";
string prevCwd = "";
vector<pid_t> backgroundProcesses;
//function to convert string to const char
const char *convertStringToConstChar(string s) {
    const char *cString = s.c_str();
    return cString;
}
//this function returns the root directory as part of the question the current working directory from where this code is executed
//will be the root directory for this custom shell
string getRootDirectory() {
    char buffer[8192];

    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return buffer;
    } else {
        cout << "Failed to get current directory." << endl;
        return "";
    }
}

string rootPath = getRootDirectory();
// this function will take care of printing the correct prompt on the terminal
string printOnTerminal(string append) {
    const char *username = getenv("USER");

    if (username != nullptr) {
        string userNameString = username;
        struct utsname systemInfo;

        if (uname(&systemInfo) == 0) {
            string systemInfoString = systemInfo.nodename;
            string currDirectory = getRootDirectory();
            string printPath = userNameString + "@" + systemInfoString + ":~";

            if (append.length() > 0) {
                printPath += "/" + append;
            }

            return printPath + ">";
        } else {
            cout  << "Failed to retrieve system name." << endl;
        }
    } else {
        cout << "Username not found." << endl;
    }

    return ">"; // Default prompt
}
// code for handling echo
void echoImplementation(vector<string> &wordsInCommands) {
    for (size_t i = 1; i < wordsInCommands.size(); ++i) {
        cout << wordsInCommands[i];

        // Print a space between arguments (except the last one)
        if (i < wordsInCommands.size() - 1) {
            cout << " ";
        }
    }
    cout << endl;
}
// code for handling pwd
void pwdImplementation() {
    char buffer[4096];
    char *res = getcwd(buffer, sizeof(buffer));
    if (res != nullptr) {
        cout << buffer << endl;
    } else {
        cout << "Error retrieving current working directory." << endl;
    }
}
//this function has been used in ls command implementation
void printFileDetails(const string &path) {
    struct stat info;
    if (stat(path.c_str(), &info) == 0) {
        cout << ((S_ISDIR(info.st_mode)) ? "d" : "-");
        cout << ((info.st_mode & S_IRUSR) ? "r" : "-");
        cout << ((info.st_mode & S_IWUSR) ? "w" : "-");
        cout << ((info.st_mode & S_IXUSR) ? "x" : "-");
        cout << ((info.st_mode & S_IRGRP) ? "r" : "-");
        cout << ((info.st_mode & S_IWGRP) ? "w" : "-");
        cout << ((info.st_mode & S_IXGRP) ? "x" : "-");
        cout << ((info.st_mode & S_IROTH) ? "r" : "-");
        cout << ((info.st_mode & S_IWOTH) ? "w" : "-");
        cout << ((info.st_mode & S_IXOTH) ? "x" : "-");
        cout << " " << info.st_nlink;
        cout << " " << getpwuid(info.st_uid)->pw_name;
        cout << " " << getgrgid(info.st_gid)->gr_name;
        cout << " " << setw(7) << info.st_size;
        cout << " " << put_time(localtime(&info.st_mtime), "%b %d %H:%M");
        cout << " " << path << endl;
    } else {
        perror("Error getting file information");
    }
}
//to check if there is a file or directory on the given path
bool isFileOrDirectory(const string &path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}
//to check if the file is a regular file or not
bool isRegularFile(const string &path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0) {
        return S_ISREG(buffer.st_mode);
    }
    return false;
}

blkcnt_t getFileBlockSize(const string &path) {
    struct stat info;
    if (stat(path.c_str(), &info) == 0) {
        return (info.st_blocks / 2);
    } else {
        perror("Error getting file information");
        return 0;
    }
}

blkcnt_t getTotalBlockCount(const string &path) {
    DIR *directory;
    struct dirent *ent;
    blkcnt_t totalSizeBlocks = 0;
    if ((directory = opendir(path.c_str())) != nullptr) {
        while ((ent = readdir(directory)) != nullptr) {
            string fileName = ent->d_name;
            if (fileName != "." && fileName != "..") {
                string fullPath = path + "/" + fileName;
                totalSizeBlocks += getFileBlockSize(fullPath);
            }
        }
        closedir(directory);
    }
    return totalSizeBlocks;
}
//this function has also been used as part of ls Implementation
void printFileDetailsMod(const string &path) {
    struct stat info;
    if (stat(path.c_str(), &info) == 0) {
        cout << ((S_ISDIR(info.st_mode)) ? "d" : "-");
        cout << ((info.st_mode & S_IRUSR) ? "r" : "-");
        cout << ((info.st_mode & S_IWUSR) ? "w" : "-");
        cout << ((info.st_mode & S_IXUSR) ? "x" : "-");
        cout << ((info.st_mode & S_IRGRP) ? "r" : "-");
        cout << ((info.st_mode & S_IWGRP) ? "w" : "-");
        cout << ((info.st_mode & S_IXGRP) ? "x" : "-");
        cout << ((info.st_mode & S_IROTH) ? "r" : "-");
        cout << ((info.st_mode & S_IWOTH) ? "w" : "-");
        cout << ((info.st_mode & S_IXOTH) ? "x" : "-");
        cout << " " << info.st_nlink;
        cout << " " << getpwuid(info.st_uid)->pw_name;
        cout << " " << getgrgid(info.st_gid)->gr_name;
        cout << " " << setw(7) << info.st_size;
        cout << " " << put_time(localtime(&info.st_mtime), "%b %d %H:%M");
        cout << " ";

        // Extract only the filename without path
        size_t lastSlashPos = path.find_last_of('/');
        string fileName = (lastSlashPos != string::npos) ? path.substr(lastSlashPos + 1) : path;

        cout << fileName << endl;
    } else {
        perror("Error getting file information");
    }
}
//this code prpvides the ls command implementation
void lsImplementation(vector<string> &wordsInCommands) {
    DIR *directory;
    struct dirent *ent;

    string path = ".";
    bool showHidden = false;
    bool longFormat = false;
    bool isCustomPath = false;

    // Handle command options and arguments
    for (size_t i = 1; i < wordsInCommands.size(); ++i) {
        string word = wordsInCommands[i];

        if (word[0] == '~') {
            word = getRootDirectory() + word.substr(1);
        }

        if (word[0] == '-') {
            for (size_t j = 1; j < word.size(); ++j) {
                char flag = word[j];
                if (flag == 'a') {
                    showHidden = true;
                } else if (flag == 'l') {
                    longFormat = true;
                } else {
                    cout << "Unknown flag: " << flag << endl;
                    return;
                }
            }
        } else {
            if (isCustomPath) {
                cout << "Only one custom path allowed." << endl;
                return;
            }
            path = word;
            isCustomPath = true;
        }
    }

    vector<string> fileNames;
    if (isFileOrDirectory(path.c_str())) {
        if (isRegularFile(path)) {
            if (longFormat) {
                printFileDetails(path);
            } else {
                cout << path << endl;
            }
            return;
        } else if ((directory = opendir(path.c_str())) != nullptr) {
            while ((ent = readdir(directory)) != nullptr) {
                if (!showHidden && ent->d_name[0] == '.') {
                    continue;
                }
                fileNames.push_back(ent->d_name);
            }
            closedir(directory);
            
            if (longFormat) {
                cout << "total " << getTotalBlockCount(path) << endl;
            }
            for (const string &fileName : fileNames) {
                if (longFormat) {
                    string fullPath = path + "/" + fileName;
                    printFileDetailsMod(fullPath);
                } else {
                    cout << fileName << " ";
                }
            }
            if (!longFormat) {
                cout << endl;
            }
        }
    } else {
        cout << "Error opening directory " << path << endl;
    }
}




//provide cd command implementation
void cdImplementation(vector<string> &wordsInCommands, string &cwdPath, string &append) {
    string currentDir = cwdPath;

    if (wordsInCommands.size() < 2 || wordsInCommands[1] == "~") {
        const char *rootDirectory = rootPath.c_str();
        if (chdir(rootDirectory) == 0) {
            cwdPath = rootPath;
            append = "";
        } else {
            perror("chdir");
            cout << "Unable to change to the root directory" << endl;
        }
    } else if (wordsInCommands[1] == "-") {
        if (chdir(prevCwd.c_str()) == 0) {
            cwdPath = prevCwd;
            append = cwdPath.substr(rootPath.length());
            if (append[0] == '/') {
                append = append.substr(1); 
            }
        } else {
            perror("chdir");
            cout << "Unable to change to the previous directory: " << prevCwd << endl;
        }
    } else if (wordsInCommands[1] == ".") {
        // If "." is used, stay in the current working directory
        // No change needed
    } else if (wordsInCommands[1] == "..") {
        if (cwdPath == rootPath.c_str()) {
            cout << rootPath << endl;
            prevCwd = cwdPath;
            append = "";
            return;
        }

        size_t found = cwdPath.find_last_of("/");
        if (found != string::npos) {
            string parentDir = cwdPath.substr(0, found);
            if (chdir(parentDir.c_str()) == 0) {
                cwdPath = parentDir;
                append = cwdPath.substr(rootPath.length());
                if (append[0] == '/') {
                    append = append.substr(1); 
                }
            } else {
                perror("chdir");
                cout << "Unable to change to the parent directory: " << parentDir << endl;
            }
        } else {
            cout << "Parent directory not found." << endl;
        }
    } else {
        string targetDir = wordsInCommands[1];
        string newDir;

        if (targetDir.front() == '/') {
            newDir = targetDir;
        } else {
            newDir = cwdPath + "/" + targetDir;
        }

        if (chdir(newDir.c_str()) == 0) {
            cwdPath = newDir;
            append = cwdPath.substr(rootPath.length());
            if (append[0] == '/') {
                append = append.substr(1); 
            }
        } else {
            perror("chdir");
            cout << "Unable to change to the specified directory: " << targetDir << endl;
        }
    }

    prevCwd = currentDir;
}
//it helps in search command implementation
bool searchFileOrFolder(const string &currentDir, const string &target) {
    DIR *directory;
    struct dirent *ent;

    if ((directory = opendir(currentDir.c_str())) != nullptr) {
        while ((ent = readdir(directory)) != nullptr) {
            string fileName = ent->d_name;
            if (fileName != "." && fileName != "..") {
                string fullPath = currentDir + "/" + fileName;

                struct stat info;
                if (stat(fullPath.c_str(), &info) == 0) {
                    if (fileName == target) {
                        cout << "True" << endl;
                        closedir(directory);
                        return true;
                    }
                    if (S_ISDIR(info.st_mode)) {
                        if (searchFileOrFolder(fullPath, target)) {
                            closedir(directory);
                            return true;
                        }
                    }
                }
            }
        }
        closedir(directory);
    }

    return false;
}

void searchImplementation(vector<string> &wordsInCommands) {
    if (wordsInCommands.size() != 2) {
        cout << "Usage: search <file_or_folder_name>" << endl;
        return;
    }

    string target = wordsInCommands[1];

    if (searchFileOrFolder(cwdPath, target)) {
        return;
    } else {
        cout << "False" << endl;
    }
}
// used in implementation of pinfo
void displaySelfProcessInfo() {
            ifstream statusFile("/proc/self/status");
            string line;
            bool pidFound = false;
            bool statusFound = false;
            bool memoryFound = false;

            if (statusFile.is_open()) {
                while (getline(statusFile, line)) {
                    if (!pidFound && line.find("Pid:") != string::npos) {
                        cout << line << endl;
                        pidFound = true;
                    }
                    if (!statusFound && (line.find("State:") != string::npos || line.find("Status:") != string::npos)) {
                        // Extract the process state from the line
                        size_t colonPos = line.find(':');
                        if (colonPos != string::npos) {
                            string stateLine = line.substr(colonPos + 1);
                            // Trim leading and trailing spaces
                            size_t start = stateLine.find_first_not_of(" \t");
                            size_t end = stateLine.find_last_not_of(" \t");
                            if (start != string::npos && end != string::npos) {
                                string processState = stateLine.substr(start, end - start + 1);
                                // Check if the process is in the foreground and add '+' accordingly
                                bool isForeground = (getpgid(getpid()) == tcgetpgrp(STDOUT_FILENO));
                                if (isForeground) {
                                    cout << "State: {" << processState << "+}" << endl;
                                } else {
                                    cout << "State: {" << processState << "}" << endl;
                                }
                            }
                        }
                        statusFound = true;
                    }
                    if (!memoryFound && line.find("VmSize:") != string::npos) {
                        cout << line << endl;
                        memoryFound = true;
                    }

                    if (pidFound && statusFound && memoryFound) {
                        break; // Stop reading the file once all required information is found
                    }
                }
                statusFile.close();

                // Display the executable path
                char buffer[PATH_MAX];
                ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer));
                if (len != -1) {
                    buffer[len] = '\0';
                    cout << "Executable Path -- " << buffer << endl;
                } else {
                    cout << "Error retrieving executable path." << endl;
                }
            } else {
                cout << "Error opening /proc/self/status." << endl;
            }
        }
        //used in implementation of pinfo
void displayProcessInfoByPID(const string &pid) {
            string statusFilePath = "/proc/" + pid + "/status";
            ifstream statusFile(statusFilePath);
            string line;

            bool pidFound = false;
            bool statusFound = false;
            bool memoryFound = false;

            if (statusFile.is_open()) {
                while (getline(statusFile, line)) {
                    if (!pidFound && line.find("Pid:") != string::npos) {
                        cout << line << endl;
                        pidFound = true;
                    }
                    if (!statusFound && (line.find("State:") != string::npos || line.find("Status:") != string::npos)) {
                        // Extract the process state from the line
                        size_t colonPos = line.find(':');
                        if (colonPos != string::npos) {
                            string stateLine = line.substr(colonPos + 1);
                            // Trim leading and trailing spaces
                            size_t start = stateLine.find_first_not_of(" \t");
                            size_t end = stateLine.find_last_not_of(" \t");
                            if (start != string::npos && end != string::npos) {
                                string processState = stateLine.substr(start, end - start + 1);
                                // Check if the process is in the foreground and add '+' accordingly
                                bool isForeground = (getpgid(stoi(pid)) == tcgetpgrp(STDOUT_FILENO));
                                if (isForeground) {
                                    cout << "State: {" << processState << "+}" << endl;
                                } else {
                                    cout << "State: {" << processState << "}" << endl;
                                }
                            }
                        }
                        statusFound = true;
                    }
                    if (!memoryFound && line.find("VmSize:") != string::npos) {
                        cout << line << endl;
                        memoryFound = true;
                    }

                    if (pidFound && statusFound && memoryFound) {
                        break; // Stop reading the file once all required information is found
                    }
                }
                statusFile.close();

                // Display the executable path
                char buffer[PATH_MAX];
                ssize_t len = readlink(("/proc/" + pid + "/exe").c_str(), buffer, sizeof(buffer));
                if (len != -1) {
                    buffer[len] = '\0';
                    cout << "Executable Path -- " << buffer << endl;
                } else if (errno != ENOENT) { // Handle ENOENT (No such file or directory) error
                    perror("Error retrieving executable path");
                }
            } else {
                cout << "Error opening " << statusFilePath << "." << endl;
            }
        }
        // it is used in dealing with background process
        void runInBackgroundFunc(vector<string> &wordsInCommands) {
        pid_t pid = fork();
        if (pid < 0) {
            cout << "Fork failed" << endl;
        } else if (pid == 0) {
            vector<char*> argv;
            for (const string& word : wordsInCommands) {
                argv.push_back(const_cast<char*>(word.c_str()));
            }
            argv.push_back(nullptr);

            // Execute the command
            execvp(argv[0], argv.data());
            perror("execvp");
            exit(1);
        } else {
            cout << "[" << pid << "]" << endl;
            backgroundProcesses.push_back(pid); // Store the PID
        }
    }

int main() {
    cwdPath = getRootDirectory();
    prevCwd = getRootDirectory();
    bool flag = true;
    int total_history_count=0;
    // initialising history
	using_history();
	stifle_history(20);
    while (flag) {
         string command;
        cout << printOnTerminal(append);

        // Read a line of input from the user
        if (!getline(cin, command)) {
            //used in the implementation of Ctrl+D
            flag = false;
            continue;
        }

        vector<string> wordsInCommands;
        
        char *command1 = const_cast<char *>(command.c_str());
        add_history(command1);
        total_history_count++;
        char *token = strtok(command1, " \t");
        while (token != nullptr) {
            string temp(token);
            wordsInCommands.push_back(temp);
            token = strtok(nullptr, " \t");
        }

        if (!wordsInCommands.empty()) {
            bool runInBackground = false;
            if(wordsInCommands.back() == "&"){
                 runInBackground = true;
                wordsInCommands.pop_back();
                 if (runInBackground) {
                  
                    runInBackgroundFunc(wordsInCommands);
                } else {
                    pid_t pid = fork();
                    if (pid < 0) {
                        cout << "Fork failed" << endl;
                    } else if (pid == 0) {
                        // Redirect standard input, output, and error to /dev/null
                        freopen("/dev/null", "r", stdin);
                        freopen("/dev/null", "w", stdout);
                        freopen("/dev/null", "w", stderr);

                        char **argv = new char *[wordsInCommands.size() + 1];
                        for (size_t i = 0; i < wordsInCommands.size(); ++i) {
                            argv[i] = const_cast<char *>(wordsInCommands[i].c_str());
                        }
                        argv[wordsInCommands.size()] = nullptr;

                        execvp(argv[0], argv);
                        perror("execvp");
                        exit(1);
                    } else {
                        waitpid(pid, nullptr, 0);
                    }
                }
            }
            else if (wordsInCommands[0] == "cd") {
                cdImplementation(wordsInCommands, cwdPath,append);
            } else if (wordsInCommands[0] == "echo") {
                echoImplementation(wordsInCommands);
            } else if (wordsInCommands[0] == "pwd") {
                pwdImplementation();
            } else if (wordsInCommands[0] == "ls") {
                lsImplementation(wordsInCommands);
            } else if (wordsInCommands[0] == "exit") {
                flag = false;
            } else if (wordsInCommands[0] == "search") {
                searchImplementation(wordsInCommands);
            }
            else if (wordsInCommands[0] == "pinfo") {
                if (wordsInCommands.size() == 1) {
                    displaySelfProcessInfo();
                } else if (wordsInCommands.size() == 2) {
                    displayProcessInfoByPID(wordsInCommands[1]);
                } else {
                    cout << "Usage: pinfo [pid]" << endl;
                }
            } 
            else if(wordsInCommands[0]=="history"){
                int hist_count = 0;
				int num = 10;
				if(wordsInCommands.size()==2){
                    num=min(num,stoi(wordsInCommands[1]));
                }
				HIST_ENTRY **entries = history_list();
				while(entries && entries[hist_count]){
					if(hist_count >= total_history_count-num){
						cout<<entries[hist_count]->line<<endl;	
					}
					hist_count++;
				}
            }
            
        }
    }

    return 0;
}
