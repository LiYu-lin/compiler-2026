# Compiler Competition 2026

一个面向 SysY 子集的 C++ 编译器项目。当前主链路已经可以将 `.sy` 源码解析为 AST，生成项目自定义 IR，执行基础 pass 流水线，并输出 RISC-V 汇编。

## 项目状态

- 语言标准：C++17
- 构建系统：CMake
- 前端：手写 Lexer、Parser Combinator 和 SysY 语法规则
- 中端：自定义 IR、IRBuilder、PassManager
- 后端：面向 RISC-V 的汇编生成、栈帧处理、活跃变量分析、冲突图染色寄存器分配
- 测试：`ctest` 驱动的后端和编译器端到端冒烟测试
- 测评入口：生成名为 `compiler` 的可执行文件，兼容 `compiler testcase.sysy -S -o testcase.s [-O1]`

当前已接入的 pass：

- `SimplifyCFG`
- `InstCombine`
- `MemoryOpt`
- `DCE`

当前 pass 流水线已经具备基础优化能力，包括恒定条件分支折叠、不可达基本块删除、代数恒等式化简、常量比较折叠、基本块内 load forwarding、重复 store 消除和死代码删除。后续仍需要继续补齐跨基本块的数据流优化和更完整的 SSA 变换。

## 目录结构

```text
.
├── cmake/                  # CMake 辅助函数
├── fixtures/               # SysY 输入样例和部分汇编样例
├── include/
│   ├── Parse/              # Lexer、Parser、Grammar、AST 接口
│   ├── GenIR/              # AST 到 IR 的 Visitor 接口
│   ├── ir/                 # IR 类型、Value、Instruction、Module、Pass 接口
│   ├── backend/            # RISC-V 后端接口
│   └── utils/              # 通用工具
├── lib/
│   ├── Parse/              # 词法、语法、AST 实现
│   ├── IR/                 # IR、IRBuilder、Pass、IR 生成实现
│   ├── backend/            # RISC-V 汇编生成与寄存器分配实现
│   └── utils/              # 工具实现
├── tests/                  # CTest 测试程序
├── tools/
│   └── sysy_compiler.cpp   # 命令行编译器入口
└── CMakeLists.txt
```

## 编译与测试

在项目根目录执行：

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build --output-on-failure
```

Windows/MinGW 单配置生成器通常会在 `build/compiler.exe` 生成测评入口；Visual Studio 等多配置生成器可能位于 `build/Debug/compiler.exe`。项目仍保留 `sysy_compiler` 目标用于本地兼容。

在 Ubuntu 24.04 测评环境中，本项目使用 C++17 和 CMake 构建，目标编译器可执行文件统一命名为 `compiler`。

## 使用方式

输出 RISC-V 汇编：

```powershell
.\build\compiler.exe fixtures\sample.sy -S -o sample.s
```

性能测评入口兼容 `-O1`：

```powershell
.\build\compiler.exe fixtures\sample.sy -S -o sample.s -O1
```

保留的调试用法：

```powershell
.\build\sysy_compiler.exe --emit-asm fixtures\sample.sy -o sample.s
```

输出项目 IR：

```powershell
.\build\sysy_compiler.exe --emit-ir fixtures\sample.sy -o sample.ir
```

如果省略 `-o`，结果会输出到标准输出。

## 编译流程

主流程位于 `tools/sysy_compiler.cpp`：

```text
SysY source
  -> Lexer
  -> Parser / Grammar
  -> AST
  -> GenIR Visitor
  -> IR Module
  -> PassManager
  -> RISC-V backend or IR output
```

### 前端

前端代码集中在 `include/Parse` 和 `lib/Parse`：

- `Lexer` 负责识别标识符、整数、浮点数、运算符、分隔符和注释。
- `Parser.h` 实现轻量 parser combinator，提供 `then`、`with`、`or_`、`many`、`sep`、`chain`、`lazy` 等组合能力。
- `Grammar.cpp` 描述 SysY 语法规则，并构造 AST 节点。
- `AST.h` 定义 AST 节点类型和 Visitor 接口。

表达式优先级由以下规则逐层实现：

```text
primary_exp
unary_exp
mul_exp
add_exp
rel_exp
eq_exp
l_and_exp
l_or_exp
```

### IR 生成

IR 生成位于 `lib/IR/GenIR/Visitor.cpp`。它负责：

- 管理符号表和作用域
- 处理函数、变量、常量、参数
- 生成表达式、赋值、分支、循环、返回语句对应的 IR
- 处理 `break`、`continue` 和逻辑短路控制流

IR 指令构造主要通过 `IR::IRBuilder` 完成。`IRBuilder` 也会在构建时做一部分常量折叠，但这不能替代独立优化 pass。

### IR 与 Pass

IR 相关代码集中在 `include/ir` 和 `lib/IR`：

- `Value` / `User` / `Use` 表示 SSA 风格的值和使用关系。
- `Instruction` 定义二元运算、比较、内存访问、分支、返回、调用、Phi 等 IR 指令。
- `BasicBlock` 和 `Function` 组成 CFG。
- `Module` 管理全局变量、函数和内置函数。
- `Pass` / `PassManager` 提供中端 pass 流水线基础设施。

当前已实现的优化：

1. `SimplifyCFG`：删除不可达块，化简恒定条件分支。
2. `InstCombine`：折叠整数/浮点常量运算，化简 `x + 0`、`x * 1`、`x ^ 0`、`x == x` 等局部表达式。
3. `MemoryOpt`：在基本块内做精确地址的 load forwarding，并删除明显重复的 store。
4. `DCE`：删除无 use 且无副作用的死指令。

后续优化建议优先补齐：

1. `Mem2Reg`：提升局部标量变量，减少 `alloca/load/store`。
2. `LowerPhi`：进入后端前将 Phi 转换为前驱块末尾的 move。
3. `GVN/CSE`：消除公共子表达式。
4. `LoopSimplify/LICM`：规范循环并外提循环不变量。

### 后端

后端代码集中在 `include/backend` 和 `lib/backend`：

- `AsmModule` 管理汇编模块输出。
- `AsmFunction` 负责函数级汇编生成、栈帧、活跃变量分析、冲突图和寄存器分配。
- `AsmBasicBlock` 负责将 IR 指令转换为 RISC-V 指令。
- `RiscVOperand` 定义寄存器、立即数、标签、地址等操作数。
- `Instructions.h` 定义 RISC-V 指令建模和输出。

当前寄存器分配主要使用临时寄存器集合，并包含基础 spill 重写逻辑。后续如果要对齐完整 ABI，需要继续完善调用约定、浮点寄存器、callee-saved 寄存器和复杂栈帧场景。

技术方案要求 RISC-V 目标遵从 GCC `-mcmodel=medany` 约定并生成 64 位 RISC-V 汇编。当前后端主链路已经能生成 RISC-V 汇编并通过项目冒烟测试，但仍需要继续系统补齐 RV64 ABI、全局地址 materialization 和更完整的浮点/数组支持。

## 开发规范

推荐 VSCode 扩展：

- `ms-vscode.cpptools-extension-pack`
- `llvm-vs-code-extensions.vscode-clangd`
- `xaver.clang-format`
- `cschlosser.doxdocgen`

推荐 `.vscode/settings.json`：

```json
{
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd",
    "editor.indentSize": "tabSize",
    "editor.tabSize": 4,
    "C_Cpp.intelliSenseEngine": "disabled"
}
```

代码风格建议：

- 使用 C++17。
- 大括号放在控制语句同一行。
- 避免在头文件中使用 `using namespace xxx;`。
- 文件、类、结构体、枚举使用清晰命名。
- 局部变量使用小写加下划线或遵循所在文件已有风格。
- 注释优先解释意图和复杂逻辑，避免重复描述代码本身。

## 测试说明

现有测试包括：

- `test_backend`：从 SysY 源码生成 IR，再检查 RISC-V 汇编结构。
- `test_driver`：调用 `compiler testcase.sysy -S -o testcase.s -O1` 格式，验证端到端输出。

运行：

```powershell
ctest --test-dir build --output-on-failure
```

建议后续增加三类测试：

- Lexer/Parser 单元测试
- IR pass 单元测试
- 更多 `.sy` 端到端样例测试

## 2026 路线图

优先级建议：

1. 扩展 `SimplifyCFG` 和 `DCE`，覆盖空跳转块合并、不可达函数清理等更多场景。
2. 继续完善 `InstCombine` 和 `MemoryOpt`，扩大局部优化覆盖面。
3. 给 IR 增加更顺手的遍历、删除和替换 use 接口。
4. 实现最小可用 `Mem2Reg`。
5. 实现 `LowerPhi`，保证后端只接收无 Phi 的普通 CFG IR。
6. 系统检查 RISC-V calling convention 和栈帧布局。
7. 扩充测试覆盖，尤其是数组、函数调用、短路逻辑、循环和全局变量。

## 合规说明

本项目不依赖 GCC、LLVM 等现有开源编译器框架源码。前端、IR、中端 pass 框架和后端均为项目内自定义实现。

项目开发过程中使用了 OpenAI Codex 作为辅助工具，辅助范围包括：

- 整理 `readme.md` 项目说明和比赛技术方案适配说明。
- 修改命令行入口，使其兼容 `compiler testcase.sysy -S -o testcase.s [-O1]`。
- 调整 CMake 目标，使构建产物包含测评要求的 `compiler`。

上述修改均经过人工审阅，并通过项目构建与测试进行验证。

## 维护说明

本仓库现在只保留这一份 Markdown 文档。历史的待办、每日总结、Lexer 函数说明和贡献指南内容已经合并或整理到本文中，后续项目说明请统一维护 `readme.md`。
