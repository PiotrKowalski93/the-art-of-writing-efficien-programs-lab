GDB stands for GNU Project Debugger and is a powerful debugging tool for C (along with other languages like C++). It helps you to poke around inside your C programs while they are executing and also allows you to see what exactly happens when your program crashes. GDB operates on executable files which are binary files produced by the compilation process.

We need to use -g to add symbols:
```
clang++-17 -g -O3 -Wall -pedantic program.cpp -o example
```

Then run debugger:
```
gdb ./example -tui
```
- tui flag gives you UI to see your code on split screen

```
break main  // sets breakpoint in main func
break 5     // sets breakpoint in line 5
info break  // info about breakpoints
next        // go to the next line
run         // run code
print       // print variable
```