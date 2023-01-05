project(P5-Interpreter)

add_executable(p5-int src/main.cpp src/Assembler.cpp src/Assembler.h src/LabelTable.cpp src/LabelTable.h src/p5_common.h src/addr_ops.h src/addr_ops.cpp src/p5_errors.h src/Lexer.cpp src/Lexer.h src/Interpreter.cpp src/Interpreter.h)