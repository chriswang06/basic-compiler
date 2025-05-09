# Simple c++ Compiler

Compiler made from scratch including parsing tree, lexical tokenizer, and asm generator

## Grammar

<img width="464" alt="image" src="https://github.com/user-attachments/assets/0608b377-878b-48fc-923f-1bbc73a93fec" />

## PreRequisites

* On x86-64, install NASM (Minimum 3.20), and ensure LD is installed.
 ```
  sudo apt-get install binutils
  sudo apt install nasm
```
* On Arm64, Install XCode which includes the assembler and linker
```
xcode-select --install

```
  

## How to run

On AArchv8 Macos and x86-64 Linux

* run executable with a program argument of a .hy file as the code to be compiled.
* View exit code with running the resulting executable file in the cmake-build-debug directory or by typing
```
./out
echo $?
```




  

