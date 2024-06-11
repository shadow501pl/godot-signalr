#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <future>
#include <iostream>
#include "hub_connection.h"
#include "hub_connection_builder.h"
#include <http_client.h>
#include "signalr_client_config.h"

using namespace godot;


class Godot_SignalR : public Node
{
	GDCLASS(Godot_SignalR, Node);


protected:
	static void _bind_methods();

public:

	Godot_SignalR();
	~Godot_SignalR();

    void Start();

    bool build(String address, String http_headers);
};
