# CSC4996_vocal_multitool

# Installation
### Windows
##### Using Powershell
Run `configure_windows.bat`
##### Using Git Bash
Make sure that CMake and Git Bash are installed. Using the Git Bash terminal in VS Code,
Add this to `~/.bash_profile`:
`export PATH="$PATH:/c/Program Files/CMake/bin"`
In root directory, run command `sh .\configure_windows.sh`
### Linux / Mac
TBD

# Build
After running setup, use this command to rebuild the project:
`cmake --build build`

# Installation Script Special Stuff - Austin
You have to install NSIS for this to work.
https://nsis.sourceforge.io/Main_Page
Aside from that, this branch should merge main into it periodically to create new installer files.
These installers can be put into the releases section for easy access.