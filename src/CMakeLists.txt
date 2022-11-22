set(APPS SamplePDFDUNE_test
         SamplePDFDUNE_NDTest
         jointFitDUNE
         DUNE_LLHscan_osc
         sigmavarDUNEFD
         probtest)

foreach(a ${APPS})
	  add_executable(${a} ${a}.cpp)
	  target_link_libraries(${a} SamplePDFDUNE)
	  install(TARGETS ${a} DESTINATION bin)
endforeach()



