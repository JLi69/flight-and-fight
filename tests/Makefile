TESTS=$(wildcard *.cpp)
CPP=c++
TEST_BIN=$(subst .cpp,,$(TESTS))

test: $(TEST_BIN)
	
%: %.cpp
	@$(CPP) $< ../src/$<.o -o $@
	@echo testing $<\...
	@./$@
	@echo TEST PASSED!
	@rm -f $@
