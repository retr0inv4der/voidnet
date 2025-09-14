# Makefile for UDP client/server + mheader encoder
# Edit SRC_* variables if your filenames/paths differ.

CXX        := g++
CXXFLAGS   := -std=gnu++17 -Wall -Wextra -Wpedantic -g -pthread
INCLUDES   := -I./mheader
LDFLAGS    :=
LDLIBS     :=

# Source files (change names if your files differ)
SRC_CLIENT := src/client.cpp
SRC_SERVER := src/server.cpp
SRC_MHDR   := src/mheader/mheader_encoder.cpp

# Targets
TARGET_CLIENT := udp_client
TARGET_SERVER := udp_server

# Object files (derived)
OBJ_CLIENT := $(SRC_CLIENT:.cpp=.o)
OBJ_SERVER := $(SRC_SERVER:.cpp=.o)
OBJ_MHDR   := $(SRC_MHDR:.cpp=.o)

.PHONY: all clean distclean run-client run-server

all: $(TARGET_CLIENT) $(TARGET_SERVER)

# Client: link client.o + mheader.o
$(TARGET_CLIENT): $(OBJ_CLIENT) $(OBJ_MHDR)
	@echo "Linking $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# Server: link server.o + mheader.o
$(TARGET_SERVER): $(OBJ_SERVER) $(OBJ_MHDR)
	@echo "Linking $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# Generic rule for .cpp -> .o
%.o: %.cpp
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Convenience run targets:
# Example: make run-client ARGS="127.0.0.1 3000"
# The default args will be nothing so you can set them on commandline.
run-client: $(TARGET_CLIENT)
	@echo "Running $(TARGET_CLIENT) $(ARGS)"
	@./$(TARGET_CLIENT) $(ARGS)

run-server: $(TARGET_SERVER)
	@echo "Running $(TARGET_SERVER) $(ARGS)"
	@./$(TARGET_SERVER) $(ARGS)

clean:
	@echo "Cleaning objects and binaries..."
	@rm -f $(OBJ_CLIENT) $(OBJ_SERVER) $(OBJ_MHDR) $(TARGET_CLIENT) $(TARGET_SERVER)

distclean: clean
	@echo "Distclean done."

# Helpful reminder target
help:
	@printf "Makefile targets:\n"
	@printf "  make           - build client and server\n"
	@printf "  make client    - build client only (same as make udp_client)\n"
	@printf "  make server    - build server only (same as make udp_server)\n"
	@printf "  make run-client ARGS=\"<args>\" - run client with optional ARGS\n"
	@printf "  make run-server ARGS=\"<args>\" - run server with optional ARGS\n"
	@printf "  make clean\n"

# allow building individual targets by name
client: $(TARGET_CLIENT)
server: $(TARGET_SERVER)
