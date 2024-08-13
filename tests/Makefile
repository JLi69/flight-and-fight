TESTS=$(wildcard *.cpp)
CPP=c++
TEST_BIN=$(subst .cpp,,$(TESTS))

test: $(TEST_BIN)
	
%: %.cpp
	@$(CPP) $< ../src/$<.o -o $@
	@echo testing $<\...
	@./$@
	@echo "\033[32;49;1mTEST PASSED\033[0m"
	@rm -f $@
