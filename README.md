# Arduino_esp32sshclient

Arduino library for ESP32 that implement an SSH client to connect and execute commands in a remote Server.

## Notes

- The library builtin on top of libssh2, and due to libssh2 dynamic memory being, and the need to release and reallocate again SSH session for each command send, this library is not suitable to be used in a stable system.
- This is just a proof of concept library, think twice before use it for production.
