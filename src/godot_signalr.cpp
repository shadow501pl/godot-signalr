#include "Godot_signalr.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>
#include <sstream>
#include <future>
#include "hub_connection.h"
#include "hub_connection_builder.h"
//#include <http_client.h>
#include "signalr_client_config.h"

using namespace godot;



class logger : public signalr::log_writer
{
    // Inherited via log_writer
    virtual void __cdecl write(const std::string & entry) override
    {
        godot::UtilityFunctions::print(entry.c_str());
    }
};

void Godot_SignalR::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("build"), &Godot_SignalR::build);
	//ClassDB::bind_method(D_METHOD("start"), &Godot_SignalR::Start);
}

Godot_SignalR::Godot_SignalR()
{
}

Godot_SignalR::~Godot_SignalR()
{

}

// Override built-in methods with your own logic. Make sure to declare them in the header as well!

void Godot_SignalR::Start()
{
}

bool Godot_SignalR::build(String address, String http_headers)
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

	signalr::hub_connection connection = builder.build();

	auto nativeConfig = signalr::signalr_client_config();
	//signalr::signalr_client_config nativeConfig;
	//if(!headers.empty()) {
	//	config.set_http_headers(resultMap);
	//}

	connection.set_client_config(nativeConfig);
	std::promise<void> start_task;
	connection.start([&start_task](std::exception_ptr exception) {
    	start_task.set_value();
	});

	start_task.get_future().get();

	return true;
}
