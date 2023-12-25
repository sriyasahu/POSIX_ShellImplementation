## POSIX Shell Implementation


* **Instruction**
  * For compiling the code for kindly provide below code on terminal
  ```
  g++ shell.cpp -lreadline
  
  ```
  
  * Now for running the code give below code on terminal
  ```
   ./a.out
  ```
 
  * Now you can see the prompt for the custom shell. Enter the desired command and press enter. 
  
  
* **Working Procedure**
  * The code initializes various global variables and retrieves system information like the username and system name to construct the custom shell's prompt.
  * The user enters a command, which is read as a string.The input string is tokenized into individual words, splitting on spaces and tabs. These words are stored in a vector for further processing.
  * The code checks the first word of the input (the command) to determine which action to perform.Each command has a corresponding function that handles its execution.
  * The code allows running processes in the background by appending & to a command.Background processes are forked and executed separately from the main shell process.
  * The pinfo command is implemented to display process information. It can show information about the current shell process or a specified process by its PID.
  * The search command searches for a file or folder within the current directory and its subdirectories.
  * The shell can be exited by typing the exit command or by using the Ctrl+D keyboard shortcut, which triggers the end of the input loop.
  * To check the history of commands used: history (optionally, a number can be passed as argument to the command)
  * For ls commands we can give flags like -l,-a,-la,-al etc.
  * Similarly,for cd we can give flags like -,~,.,.. etc.
  




