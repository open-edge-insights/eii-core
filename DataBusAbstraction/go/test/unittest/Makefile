UNIT_TEST_PATH = IEdgeInsights/DataBusAbstraction/go/test/unittest

check: 
	@echo "command to run Unit test: make <options>"
	@echo "options are:"
	@echo "1)TestNegativeSub "
	@echo "2)TestNegativeContextDestroy"
	@echo "3)TestPattern"
	@echo "4)TestSub"
	@echo "5)TestPub"
	@echo "6)TestInit"
	@echo "7)TestCreateContext"
	@echo "8)TestContextDestroy"

# TODO: Run the all the testcases at a time instead of running individually once the DBA C stack works for multiple subscribers from a single process.

TestNegativeSub: 
	go test -timeout 30s $(UNIT_TEST_PATH) -run TestNegativeSub

TestNegativeContextDestroy:
	go test -timeout 30s $(UNIT_TEST_PATH) -run TestNegativeContextDestroy

TestPattern:
	go test -timeout 30s $(UNIT_TEST_PATH) -run TestPattern

TestSub:
	go test -timeout 30s $(UNIT_TEST_PATH) -run TestSub

TestPub:
	go test -timeout 30s $(UNIT_TEST_PATH) -run TestPub

TestInit:
	go test -timeout 30s $(UNIT_TEST_PATH) -run TestInit

TestCreateContext:
	go test -timeout 30s $(UNIT_TEST_PATH) -run TestCreateContext

TestContextDestroy:
	go test -timeout 30s $(UNIT_TEST_PATH) -run TestContextDestroy




