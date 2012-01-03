# Sensor Application Development/Declarative Language

TARGET=seal
FLEX=flex
YACC=yacc

CFLAGS=-g -Wall
LDFLAGS=-lfl

APP_OBJECTS = main.o objects.o parser.o condition.o generator.o

########################################
.PHONY: all parser lexer clean test

all: parser lexer $(APP_OBJECTS)
	$(CXX) $(CFLAGS) y.tab.o lex.yy.o $(APP_OBJECTS) -o $(TARGET) $(LDFLAGS)

parser: parser.y
	$(YACC) -d parser.y
	$(CC) $(CFLAGS) -c y.tab.c

lexer: lexer.l
	$(FLEX) lexer.l
	$(CC) $(CFLAGS) -c lex.yy.c

clean:
	rm -f $(APP_OBJECTS) y.tab.* lex.yy.* $(TARGET)

test:
	./$(TARGET) ./test-cases/1.sl
	./$(TARGET) ./test-cases/2.sl
	./$(TARGET) ./test-cases/3.sl
	./$(TARGET) ./test-cases/4.sl
	./$(TARGET) ./test-cases/5.sl
	./$(TARGET) ./test-cases/6.sl
	./$(TARGET) ./test-cases/6a.sl
	./$(TARGET) ./test-cases/6b.sl
	./$(TARGET) ./test-cases/7.sl
	./$(TARGET) ./test-cases/7a.sl
	./$(TARGET) ./test-cases/7b.sl
	./$(TARGET) ./test-cases/8.sl

test6:
	./$(TARGET) ./test-cases/6.sl
	./$(TARGET) ./test-cases/6a.sl
	./$(TARGET) ./test-cases/6b.sl

test7:
	./$(TARGET) ./test-cases/7.sl
	./$(TARGET) ./test-cases/7a.sl
	./$(TARGET) ./test-cases/7b.sl

%.o : %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@
