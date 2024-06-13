#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <future>
#include <iostream>
#include "hub_connection.h"
#include "hub_connection_builder.h"
//#include <http_client.h>
#include "signalr_client_config.h"
#include <godot_cpp/classes/thread.hpp>
#include "signalr_value.h"
#include <godot_cpp/classes/thread.hpp>

using namespace godot;


class Godot_SignalR : public Node
{
	GDCLASS(Godot_SignalR, Node);


private:
	signalr::hub_connection* connection;
	godot::String address;
	godot::String http_headers;
	Ref<Thread> async;


protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
    void Stop();

    Godot_SignalR();
    ~Godot_SignalR();


    void build();
	void async_build(String _address, String _http_headers);

	void receive_message(const std::vector<signalr::value>& m);
};
