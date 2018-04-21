CPP_CC = g++
C_CC   = gcc

RM = rm

CDEBUG = -g
LDEBUG = -g

INC_DIR = ../include
OBJ_DIR = ../obj
LIB_DIR = ../lib
BIN_DIR = ../bin

all: $(BIN_DIR)/CVML

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(BIN_DIR)/CVML

CPP_SRC = \
CVMLArgument.cpp \
CVML.cpp \
CVMLData.cpp \
CVMLDataSrc.cpp \
CVMLDebug.cpp \
CVMLDecode.cpp \
CVMLEncode.cpp \
CVMLExecute.cpp \
CVMLLabel.cpp \
CVMLLine.cpp \
CVMLOp.cpp \
CVMLParse.cpp \
CVMLTest.cpp \

CPP_OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(CPP_SRC))

CPPFLAGS = \
-std=c++14 \
-I$(INC_DIR) \
-I. \
-I../../CArgs/include \
-I../../CStrUtil/include \
-I../../CFile/include \
-I../../CMath/include \
-I../../CReadLine/include \
-I../../COS/include \
-I../../CUtil/include \

LFLAGS = \
$(LEBUG) \
-L$(LIB_DIR) \
-L../../CArgs/lib \
-L../../CStrUtil/lib \
-L../../CFile/lib \
-L../../CReadLine/lib \
-L../../COS/lib \
-lCArgs -lCFile -lCReadLine -lCOS -lCStrUtil \
-lreadline

.SUFFIXES: .cpp

$(CPP_OBJS): $(OBJ_DIR)/%.o: %.cpp
	$(CPP_CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

$(BIN_DIR)/CVML: $(CPP_OBJS)
	$(CPP_CC) -o $(BIN_DIR)/CVML $(CPP_OBJS) $(LFLAGS)
