#include "Godot_signalr.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/time.hpp>
#include <iostream>
#include <sstream>
#include <future>
#include "hub_connection.h"
#include "hub_connection_builder.h"
//#include <http_client.h>
#include "signalr_client_config.h"
#include "signalr_value.h"

using namespace godot;



class logger : public signalr::log_writer
{
    // This prints logs
    virtual void __cdecl write(const std::string & entry) override
    {
        godot::UtilityFunctions::print(entry.c_str());
    }
};

void Godot_SignalR::_bind_methods()
{
	//ClassDB::bind_method(D_METHOD("build"), &Godot_SignalR::build);
	ClassDB::bind_method(D_METHOD("async_build"), &Godot_SignalR::async_build);
	ADD_SIGNAL(MethodInfo("receive_message", PropertyInfo(Variant::STRING, "message")));
}

// Not sure if this works, but oh well
void Godot_SignalR::Stop()
{
    // Check if the connection exists before attempting to stop it
    if(this->connection!= nullptr) {
		std::promise<void> stopTask;
        this->connection->stop([&stopTask](std::exception_ptr exception){
			godot::UtilityFunctions::print("Connection stopped.");
			//this->connection = nullptr;
		});
    } else {
        godot::UtilityFunctions::print("No active connection to stop.");
    }
}

Godot_SignalR::Godot_SignalR()
{
}

Godot_SignalR::~Godot_SignalR()
{

}


void Godot_SignalR::_notification(int p_what) {
    // Prevents this from running in the editor, only during game mode.
    switch (p_what) {
        case NOTIFICATION_EXIT_TREE: { // Thread must be disposed (or "joined"), for portability.
            // Wait until it exits.
            if (async.is_valid()) {
                async->wait_to_finish();
            }

            async.unref();
        } break;
    }
}



// Async in progress
void Godot_SignalR::async_build(String _address, String _http_headers) 
{
	if(async.is_valid()) {
		godot::UtilityFunctions::print("There is already an active connection.");
		return;
	}
	async.instantiate();
	address = _address;
	http_headers = _http_headers;
	async->start(callable_mp(this, &Godot_SignalR::build), Thread::PRIORITY_NORMAL);
	//build(address, http_headers); // Implement threads to prevent freezes that distrupt gameplay
}

// I suspect putting this on a thread(async) will make config/ReceiveMessage work
// Currently only way to pass in auth is thru address, (just append &access_token=<token>)
void Godot_SignalR::build()
{
	std::string url = address.utf8();
	std::string headers = http_headers.utf8();
	std::map<std::string, std::string> resultMap;
    std::istringstream iss(headers);
    std::string line;
	if(!headers.empty())
	{
    	while (std::getline(iss, line)) {
        	size_t pos = line.find(':');
        	if (pos!= std::string::npos) {
            	std::string key = line.substr(0, pos);
            	std::string value = line.substr(pos + 1);
            	resultMap[key] = value;
        	}
    	}
	}

	signalr::hub_connection_builder builder = signalr::hub_connection_builder::create(url);
	builder.with_logging(std::make_shared <logger>(), signalr::trace_level::verbose);

	godot::UtilityFunctions::print(" building hub, " + address);

	signalr::hub_connection local_connection = builder.build();
	connection = &local_connection;
	// TODO - Fix this
	//auto nativeConfig = signalr::signalr_client_config();
	//signalr::signalr_client_config nativeConfig;
	//if(!headers.empty()) {
	//	config.set_http_headers(resultMap);
	//}

	//connection.set_client_config(nativeConfig);
	local_connection.on("ReceiveMessage", [this](const std::vector<signalr::value>& m)
    {
		receive_message(m);
		//emit_signal("receive_message", m[0].as_string().c_str());
    });

	std::promise<void> start_task;
	local_connection.start([&start_task](std::exception_ptr exception) {
    	start_task.set_value();
	});

	start_task.get_future().get();

}

void Godot_SignalR::receive_message(const std::vector<signalr::value>& m) {
	if (m[0].is_string()){
		emit_signal("receive_message", m[0].as_string().c_str());
	}
}
