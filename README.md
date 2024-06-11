# Godot-SignalR
A **WIP** SignalR adaptation for godot, using GdExtension
- Uses [microsoft-signalr](https://github.com/aspnet/SignalR-Client-Cpp/tree/main?tab=readme-ov-file#command-line-build) and all of its dependencies
- Currently there is no precompiled version, you need to compile it yourself
Created to allow for pubsubconnectionhandles in godot-playfab

### 🕹️ Usage
```GDScript
# To create a connection, you first need a signal_r node refrence
var node = $Signal_R
# For a connection you need the url and http_headers (they can be left blank if not needed)
node.build("http://localhost:5000", "")
# Now get the handle (unfinished)
var handle : String = node.get_handle()
# (todo documentation)
```

### ✅ Functionality
- Connect to a signalr server (note: Not fully tested, and may not work at all on diffrent operating systems)
### ❌ To be added
- Add signalr::signalr_client_config to be able to add html-headers
- Add signals/events in godot
- Add functions to manage the connection in godot

### 📌 Guide to developing this addon
1. Clone Repo
2. Run ```git submodule update --init``` to get godot-cpp
3. Install vpckg in this directory
4. Install [microsoft-signalr](https://github.com/aspnet/SignalR-Client-Cpp/tree/main?tab=readme-ov-file#command-line-build) using vpckg and all its dependencies
5. Done, you should now be able to develop the addon!
