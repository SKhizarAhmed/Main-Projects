# CHAT APP
# This is an encrypted chat app that XOR encrypts the text of the client -> sends the encrypted text to the server -> server reads the target destination and forwards the encrypted text to that specfic target
# -> the target decrypts it and sees the message.
# => Secondly there can be upto 10 active clients on the server (the number can be increased), server spawns a separate thread for a separate client.
# => This chat app code is safe to use only on a local host / computer, you can't implement the same code in an online chat app.

# Other Details: 

 Central Routing Server: Manages up to 10 concurrent client connections, maintaining individual socket tracking thread-safely via Win32 Mutexes. 

Asynchronous Multi-Threading: Dual-thread execution models on both sides:

Server: Spawns a dedicated thread per client to block on read pipelines without stalling the main connection acceptance loop.

Client: Splits into a main thread for user keyboard interaction and a background thread dedicated to receiving network traffic.

Persistent Session Mode: Features a nested input design allowing users to lock onto a target Client ID for rapid-fire communication until choosing to exit back to global selection.

Symmetric XOR Cipher: All text packet payloads are dynamically obfuscated during transit using a multi-byte repeating key sequence and seamlessly reversed upon arrival.
