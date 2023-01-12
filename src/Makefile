PROJECT_NAME    = bittorrent
PROJECT_DIR     = .

BUILD_CONFIG    = debug
# BUILD_CONFIG    = release

ifeq ($(OS), Windows_NT)
	$(error Unsupported platform.)
else
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		# Configure path for OpenSSL include and library
		OPENSSL_INCLUDE = /usr/include/
		OPENSSL_LIBDIR  = /usr/lib/x86_64-linux-gnu
	else
		$(error Unsupported platform.)
	endif
endif

CC = g++

SRC_DIR          = $(PROJECT_DIR)
SRC_OBJDIR       = $(PROJECT_DIR)/$(BUILD_CONFIG)/obj
SRC_LIBDIR       = $(PROJECT_DIR)/$(BUILD_CONFIG)/lib
SRC_BINDIR       = $(PROJECT_DIR)/$(BUILD_CONFIG)/bin

SRC_LIB          = $(SRC_LIBDIR)/lib$(PROJECT_NAME).a
SRC_RUN          = $(SRC_BINDIR)/$(PROJECT_NAME)

SRC_INCLUDEFLAGS = -I$(OPENSSL_INCLUDE) -I$(PROJECT_DIR)/include
SRC_CXXFLAGS     = -Wall -std=c++17 $(SRC_INCLUDEFLAGS)
SRC_LIBFLAGS     = $(SRC_LIB) -L$(OPENSSL_LIBDIR) -lssl -lcrypto -lpthread

ifeq ("x$(BUILD_CONFIG)", "xdebug")
	SRC_CXXFLAGS := -g -D_DEBUG $(SRC_CXXFLAGS)
endif

SRC_MODULES      = common                   \
                   peer                     \
                   socket                   \
                   torrent

SRC_FILES        = common/Helpers.cpp                 \
				   common/Logger.cpp                  \
  				   peer/BinaryFileHandler.cpp         \
   				   peer/Leecher.cpp                   \
				   peer/MessageParcel.cpp             \
				   peer/MessageParcelFactory.cpp      \
				   peer/MessagingSocket.cpp           \
				   peer/Peer.cpp                      \
				   peer/PieceParcel.cpp               \
				   peer/RequestParcel.cpp             \
				   peer/Seeder.cpp                    \
				   socket/ClientSocketObservable.cpp  \
				   socket/ConnectedSocket.cpp         \
				   socket/IPv4ClientSocket.cpp        \
				   socket/IPv4ServerSocket.cpp        \
				   socket/ServerSocketObservable.cpp  \
				   torrent/MinimalTorrentParser.cpp   \
				   torrent/Torrent.cpp                \
                   StartParams.cpp


SRC_OBJS          = $(patsubst %.cpp, %.o, $(SRC_FILES))
SRC_BUILD_DIR     = $(addprefix $(SRC_OBJDIR)/,$(SRC_MODULES))

GTEST_INCUDEDIR   = /usr/include/gtest
GTEST_LIB         = /usr/lib/x86_64-linux-gnu/libgtest.a

GTEST_CXXFLAGS    = -I$(GTEST_INCLUDEDIR)
GTEST_LIBFLAGS    = -lpthread $(GTEST_LIB)

TEST_DIR          = $(PROJECT_DIR)/unittest
TEST_INCLUDEDIR   = $(TEST_DIR)/$(BUILD_CONFIG)/include
TEST_BINDIR       = $(TEST_DIR)/$(BUILD_CONFIG)/bin
TEST_OBJDIR       = $(TEST_DIR)/$(BUILD_CONFIG)/obj

TEST_RUN          = $(TEST_BINDIR)/test_$(PROJECT_NAME)

TEST_INCLUDEFLAGS = $(SRC_INCLUDEFLAGS) -I$(TEST_DIR) -I$(TEST_INCLUDEDIR)
TEST_LIBFLAGS     = $(GTEST_LIBFLAGS) $(SRC_LIBFLAGS)
TEST_CXXFLAGS     = $(SRC_CXXFLAGS) $(GTEST_CXXFLAGS) $(TEST_INCLUDEFLAGS)

TEST_FILES        = main.cpp                     \
				    testMinimalTorrentParser.cpp 

TEST_OBJS         = $(patsubst %.cpp, %.o, $(TEST_FILES))


$(SRC_BUILD_DIR):
	@mkdir -p $@

$(SRC_OBJDIR)/%.o : $(SRC_DIR)/%.cpp
	@mkdir -p $(SRC_OBJDIR)
	$(CC) -c $(SRC_CXXFLAGS) $< -o $@

$(SRC_LIB): $(SRC_BUILD_DIR) $(addprefix $(SRC_OBJDIR)/,$(SRC_OBJS))
	@mkdir -p $(SRC_LIBDIR)
	ar rvs $(SRC_LIB) $(SRC_OBJDIR)/*.o $(addsuffix /*.o,$(SRC_BUILD_DIR))
	
$(SRC_RUN) : $(SRC_LIB) $(SRC_DIR)/main.cpp
	@mkdir -p $(SRC_BINDIR) $(SRC_OBJDIR)
	$(CC) -c $(SRC_CXXFLAGS) $(SRC_DIR)/main.cpp -o $(SRC_OBJDIR)/main.o
	$(CC) -o $(SRC_RUN) $(SRC_CXXFLAGS) $(SRC_OBJDIR)/main.o $(SRC_LIBFLAGS)

all: $(SRC_RUN)
	sudo chmod 755 $(SRC_RUN)
	$(SRC_RUN) -h

$(TEST_OBJDIR)/%.o : $(TEST_DIR)/%.cpp
	@mkdir -p $(TEST_OBJDIR)
	$(CC) -c $(TEST_CXXFLAGS) $< -o $@

tests: $(GTEST_LIB) $(SRC_LIB) $(addprefix $(TEST_OBJDIR)/, $(TEST_OBJS))
	@mkdir -p $(TEST_BINDIR)
	$(CC) -o $(TEST_RUN) $(TEST_CXXFLAGS) $(TEST_OBJDIR)/*.o $(TEST_LIBFLAGS)
	sudo chmod 755 $(TEST_RUN)
	$(TEST_RUN)

clean:
	rm -rf $(SRC_LIB) $(SRC_RUN) $(SRC_OBJDIR)/* 
	rm -rf $(TEST_RUN) $(TEST_OBJDIR)/*

