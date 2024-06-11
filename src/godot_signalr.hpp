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

using namespace godot;


class Godot_SignalR : public Node
{
	GDCLASS(Godot_SignalR, Node);


private:
	signalr::hub_connection* connection;

protected:
	static void _bind_methods();

public:
    void Stop();

    Godot_SignalR();
    ~Godot_SignalR();


    void build(String address, String http_headers);
	void async_build(String address, String http_headers);
};
