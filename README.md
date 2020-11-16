### 前置条件

- Mac OS X
  - Xcode
- Visual Studio Code
  - Extensions
    - C/C++
    - hexdump for VSCode
- Shell Commands:
  - git
  - make
  - clang/gcc
  - lldb/gdb

### 初始化仓库

1. git clone https://github.com/silenaker/apue.git
2. git submodule update --init

### 编译

- 通过 VS Code 任务编译
  - 条件：打开需要编译的源文件
  - 快捷键：cmd + shift + b
- 通过命令行
  - make all
  - make \<goal\>（\<goal\> 为目标源程序，工作目录相对路径）
  - make clean

### 调试

- 通过 [VS Code C/C++ Extension][vscode-cpp]
  - 条件：打开需要调试的源文件
  - 快捷键：F5
  - 注意事项
    - 程序无法通过 VS Code Debug Console 接受输入，已在调试配置中设置 `externalConsole: true`，且在调试前需执行 Open Terminal 任务（`>Tasks: Run Task` -> `Open Terminal`），具体原因请参考这个 [issue][vscode-cpp-issue]
- 通过 lldb 命令
  - 参考 `man lldb` 或 [LLDB][lldb]

### 运行

略

### 参考链接

- [GNU make][gnu-make]
- [Sources from subdirectories in Makefile][makefile-recursive-stackoverflow]
- [Makefiles - Recursive Make For Sub-directories][makefile-recursive]
- [LLDB][lldb]
- [LLDB-MI][lldb-mi]
- [C standard library][c-standard]/[C POSIX library][c-posix]

[gnu-make]: https://www.gnu.org/software/make/manual/make.html
[makefile-recursive-stackoverflow]: https://stackoverflow.com/questions/4036191/sources-from-subdirectories-in-makefile
[makefile-recursive]: http://owen.sj.ca.us/~rk/howto/slides/make/slides/makerecurs.html
[lldb]: https://lldb.llvm.org/use/tutorial.html
[lldb-mi]: https://github.com/lldb-tools/lldb-mi
[vscode-cpp]: https://code.visualstudio.com/docs/cpp/config-clang-mac
[vscode-cpp-issue]: https://github.com/microsoft/vscode-cpptools/issues/5079#issuecomment-626090192
[c-standard]: https://en.wikipedia.org/wiki/C_standard_library
[c-posix]: https://en.wikipedia.org/wiki/C_POSIX_library
