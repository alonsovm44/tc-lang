#!/bin/bash
# Test script for argument count validation

echo "Testing argument count validation..."
echo ""

# Test 1: Too few arguments
echo "Test 1: Too few arguments (should fail with E015)"
../tigc wrong_args.tc 2>&1 | grep -q "E015"
if [ $? -eq 0 ]; then
    echo "  PASS: Caught too few arguments"
else
    echo "  FAIL: Did not catch too few arguments"
fi
echo ""

# Test 2: Too many arguments
echo "Test 2: Too many arguments (should fail with E016)"
../tigc too_many_args.tc 2>&1 | grep -q "E016"
if [ $? -eq 0 ]; then
    echo "  PASS: Caught too many arguments"
else
    echo "  FAIL: Did not catch too many arguments"
fi
echo ""

# Test 3: Correct number of arguments
echo "Test 3: Correct number of arguments (should succeed)"
cat > correct_args.tc << 'EOF'
fn i32 add: i32 a, i32 b {
    ret a + b
}

fn i32 main: {
    ret add(1, 2)
}
EOF
../tigc correct_args.tc -o correct_args 2>&1 | grep -q "error"
if [ $? -ne 0 ]; then
    echo "  PASS: Correct arguments compiled successfully"
    rm -f correct_args correct_args.tc
else
    echo "  FAIL: Correct arguments failed to compile"
    rm -f correct_args correct_args.tc
fi
