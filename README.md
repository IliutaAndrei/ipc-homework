Clone the repository.

Open ipc-homework.slnx in Visual Studio.

Make sure vcpkg is configured to install the dependencies from vcpkg.json.

Build the solution.


Run the server executable. The server listens on port 5000.

Open a terminal and run the client with the file path as an argument:

.\x64\Debug\client.exe .\test.txt

The client connects to 127.0.0.1:5000 and sends the file.

The server saves the received file in the received_files directory, mentaining its original filename.