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
//#include <string.hpp>

using namespace godot;



class logger : public signalr::log_writer
{
    // This prints logs
    virtual void __cdecl write(const std::string & entry) override
    {
        godot::UtilityFunctions::print(entry.c_str());
    }
};


signalr::value Godot_SignalR::convertGodotVariantToSignalRValue(const godot::Variant& variant) {
    signalr::value result; // Default to NIL

    switch (variant.get_type()) {
        case godot::Variant::Type::NIL:
            // Handle nil type
            break;
        case godot::Variant::Type::BOOL:
            // Handle boolean type
            //result = signalr::value(variant.bool());
            break;
        case godot::Variant::Type::INT:
            // Handle integer type
            //result = signalr::value(variant.int());
            break;
        case godot::Variant::Type::FLOAT:
            // Handle real (floating-point) type
			//float fl = variant.float()
            //result = signalr::value(variant.float());
            break;
        case godot::Variant::Type::STRING:
            // Handle string type
			//godot::String str = variant.String();

            result = signalr::value(variant.stringify().utf8());
            break;
        default:
            // Handle unsupported types or log an error
			godot::UtilityFunctions::printerr("Invalid value passed");
            break;
    }

    return result;
}

void Godot_SignalR::_bind_methods()
{
	//ClassDB::bind_method(D_METHOD("build"), &Godot_SignalR::build);
	ClassDB::bind_method(D_METHOD("build"), &Godot_SignalR::async_build);
	ClassDB::bind_method(D_METHOD("invoke"), &Godot_SignalR::invoke);
	ADD_SIGNAL(MethodInfo("receive_message", PropertyInfo(Variant::OBJECT, "message")));
	ADD_SIGNAL(MethodInfo("receive_invoke_result", PropertyInfo(Variant::OBJECT, "result")));
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


void Godot_SignalR::invoke(String function_name, godot::Array args) {
	if (args.is_empty()) {
		return;
	}
	if(connection != nullptr) {
		std::vector<signalr::value> signalRVector;
		for (int i = 0; i < args.size(); ++i) {
    		signalRVector.push_back(convertGodotVariantToSignalRValue(args[i]));
		std::string func = function_name.utf8();
		connection->invoke(func, signalRVector, [this](const signalr::value& value, std::exception_ptr exception)
    	{
            if (value.is_string())
            {
                std::cout << "Received: " << value.as_string() << std::endl;
            }
            else
            {
                std::cout << "hub method invocation has completed" << std::endl;
            }
			receive_invoke_response(value);
        });
		}	
	}
}




// Async/Threading
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
}

// Threading allowed for events, unfortunately config crashes the game and i dont know why
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
	//signalr::signalr_client_config nativeConfig = signalr::signalr_client_config();
	signalr::hub_connection_builder builder = signalr::hub_connection_builder::create(url);
	builder.with_logging(std::make_shared <logger>(), signalr::trace_level::verbose);
	//godot::UtilityFunctions::print(" building hub, " + address);

	signalr::hub_connection local_connection = builder.build();
	connection = &local_connection;
	// TODO - Fix this
	//auto nativeConfig = signalr::signalr_client_config();
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
void Godot_SignalR::receive_invoke_response(const signalr::value& value) {
	if (value.is_string()){
		emit_signal("receive_invoke_result", value.as_string().c_str());
	}
}




