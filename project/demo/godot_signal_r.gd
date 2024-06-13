extends Godot_SignalR


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	
	async_build("http://localhost:5000", "")
