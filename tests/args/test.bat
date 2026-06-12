@echo off
REM Test script for argument count validation

echo Testing argument count validation...
echo.

REM Test 1: Too few arguments
echo Test 1: Too few arguments (should fail with E015)
..\tigc wrong_args.tc 2>&1 | findstr /C:"E015" >nul
if %errorlevel% equ 0 (
    echo   PASS: Caught too few arguments
) else (
    echo   FAIL: Did not catch too few arguments
)
echo.

REM Test 2: Too many arguments
echo Test 2: Too many arguments (should fail with E016)
..\tigc too_many_args.tc 2>&1 | findstr /C:"E016" >nul
if %errorlevel% equ 0 (
    echo   PASS: Caught too many arguments
) else (
    echo   FAIL: Did not catch too many arguments
)
echo.

REM Test 3: Correct number of arguments
echo Test 3: Correct number of arguments (should succeed)
(
echo fn i32 add: i32 a, i32 b {
echo     ret a + b
echo }
echo.
echo fn i32 main: {
echo     ret add^(1, 2^)
echo }
) > correct_args.tc
..\tigc correct_args.tc -o correct_args 2>&1 | findstr /C:"error" >nul
if %errorlevel% neq 0 (
    echo   PASS: Correct arguments compiled successfully
    del correct_args.tc
    del correct_args.exe 2>nul
) else (
    echo   FAIL: Correct arguments failed to compile
    del correct_args.tc
    del correct_args.exe 2>nul
)
