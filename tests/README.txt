Run Windows Acceptance tests by means of the command line

    node AcceptanceTests.js [optional filter arguments]
    
These will run through a suite of tests that will modify the real system - be prepared.
For example, magnification, desktop contrast, and pointer trails settings will be
activated and deactivated on the current Windows system.

The test suites available are listed in the

   tests/platform/index-windows.js
   
directory of the GPII universal project checked out as part of the current build.

In order to filter only a subset of the available suites, you can supply extra
command line arguments which will be checked as substrings of the test suite files.

For example, to run only the suites testing built-in adaptations of windows, run

    node AcceptanceTests.js builtIn
    
Or to run only the tests for the NVDA screenreader, run

    node AcceptanceTests.js nvda
    
    