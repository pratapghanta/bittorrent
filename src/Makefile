
# Configure path for OpenSSL include and library
UNAME_S         = $(shell uname -s)
ifeq ($(OS),Windows_NT)
	OPENSSL_INCLUDE =

else ifeq ($(UNAME_S), Linux)
	OPENSSL_INCLUDE = /usr/local/include/
	OPENSSL_LIBDIR  = /usr/local/opt/openssl/lib/

else ifeq ($(UNAME_S), Darwin)
	OPENSSL_INCLUDE = /usr/local/opt/openssl/include/
	OPENSSL_LIBDIR  = /usr/local/opt/openssl/lib/
endif

CC = g++

PROJECT_NAME    = bittorrent
PROJECT_DIR     = .

SRC_DIR         = $(PROJECT_DIR)
SRC_OBJDIR      = $(PROJECT_DIR)/obj
SRC_LIBDIR      = $(PROJECT_DIR)/lib
SRC_BINDIR      = $(PROJECT_DIR)/bin
SRC_INCLUDEDIR  = $(PROJECT_DIR)/include

SRC_LIB         = $(SRC_LIBDIR)/lib$(PROJECT_NAME).a
SRC_RUN         = $(SRC_BINDIR)/$(PROJECT_NAME)

SRC_CXXFLAGS    = -g -Wall -std=c++11 -I$(SRC_INCLUDEDIR) -I$(OPENSSL_INCLUDE)
SRC_LIBFLAGS    = -L$(OPENSSL_LIBDIR) -lssl -lcrypto $(SRC_LIB)

SRC_FILES       = Config.cpp               \
                  helpers.cpp              \
                  Leecher.cpp              \
                  LeecherHandler.cpp       \
                  Message.cpp              \
                  MinimalTorrentParser.cpp \
                  Peer.cpp                 \
                  Piece.cpp                \
                  Seeder.cpp               \
                  Torrent.cpp

SRC_OBJS        = $(patsubst %.cpp, %.o, $(SRC_FILES))

GTEST_DIR       = /media/sf_Source/Github/googletest/googletest
GTEST_LIB       = $(GTEST_DIR)/libgtest.a

GTEST_CXXFLAGS  = -I$(GTEST_DIR)/include -I$(GTEST_DIR)
GTEST_LIBFLAGS  = -lpthread $(GTEST_LIB)

TEST_DIR        = $(PROJECT_DIR)/test
TEST_INCLUDEDIR = $(TEST_DIR)/include
TEST_BINDIR     = $(TEST_DIR)/bin
TEST_OBJDIR     = $(TEST_DIR)/obj

TEST_RUN        = $(TEST_BINDIR)/test_$(PROJECT_NAME)

TEST_CXXFLAGS   = -g $(SRC_CXXFLAGS) $(GTEST_CXXFLAGS) -I$(SRC_INCLUDEDIR) -I$(TEST_DIR) -I$(TEST_INCLUDEDIR) 
TEST_LIBFLAGS   = $(GTEST_LIBFLAGS) $(SRC_LIBFLAGS)

TEST_FILES      = main.cpp                     \
                  testConfig.cpp               \
                  testMessage.cpp              \

TEST_OBJS       = $(patsubst %.cpp, %.o, $(TEST_FILES))

$(SRC_OBJDIR)/%.o : $(SRC_DIR)/%.cpp
	@mkdir -p $(SRC_OBJDIR)
	$(CC) -c $(SRC_CXXFLAGS) $< -o $@

$(SRC_LIB): $(addprefix $(SRC_OBJDIR)/,$(SRC_OBJS))
	@mkdir -p $(SRC_LIBDIR)
	ar rvs $(SRC_LIB) $(SRC_OBJDIR)/*.o
	
$(SRC_RUN) : $(SRC_LIB) $(SRC_DIR)/main.cpp
	@mkdir -p $(SRC_BINDIR) $(SRC_OBJDIR)
	$(CC) -c $(SRC_CXXFLAGS) $(SRC_DIR)/main.cpp -o $(SRC_OBJDIR)/main.o
	$(CC) $(SRC_CXXFLAGS) $(SRC_OBJDIR)/main.o $(SRC_LIBFLAGS) -o $(SRC_RUN)

all: $(SRC_RUN)
	chmod 755 $(SRC_RUN)
	$(SRC_RUN) -h

$(TEST_OBJDIR)/%.o : $(TEST_DIR)/%.cpp
	@mkdir -p $(TEST_OBJDIR)
	$(CC) -c $(TEST_CXXFLAGS) $< -o $@

tests: $(GTEST_LIB) $(SRC_LIB) $(addprefix $(TEST_OBJDIR)/, $(TEST_OBJS))
	@mkdir -p $(TEST_BINDIR)
	$(CC) $(TEST_CXXFLAGS) $(TEST_OBJDIR)/*.o $(TEST_LIBFLAGS) -o $(TEST_RUN)
	chmod 755 $(TEST_RUN)
	$(TEST_RUN)

clean:
	rm -rf $(SRC_LIB) $(SRC_RUN) $(SRC_OBJDIR)/* 
	rm -rf $(TEST_RUN) $(TEST_OBJDIR)/*

