// E is a procedural interpreted language
// It is statically typed because that makes it clearer to read

// Example:

// This is a comment
// Multiline comments like in C do not exist

// No entry point

import "foo.e" // Imports and executes another e file

const text: string = "Hello World" // The language doesn't use semicolons
var i: int = 24

print(text) // This prints "Hello World"

// If statement
if 1 == 2 or 2 == 4
{
    exit(1) // Exit from the program
}
