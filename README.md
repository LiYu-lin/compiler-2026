# Compiler Competition 2026

这是一个面向 SysY 子集的 C++17 编译器项目。当前主流程可以把 `.sy` 源程序解析为 AST，生成项目自定义 IR，运行基础优化 pass，并输出 RV64 RISC-V 汇编。

## 项目状态

- 构建系统：CMake
- 语言标准：C++17
- 前端：手写 Lexer、Parser Combinator、SysY Grammar 和 AST
- 中端：自定义 IR、IRBuilder、PassManager
- 后端：RV64 RISC-V 汇编生成、栈帧布局、调用约定处理、活跃变量分析、冲突图染色寄存器分配、基础 spill 重写
- 测评入口：生成名为 `compiler` 的可执行文件，兼容 `compiler testcase.sy -S -o testcase.s [-O1]`

当前已接入的优化 pass：

- `SimplifyCFG`
- `InstCombine`
- `MemoryOpt`
- `DCE`

当前后端已重点处理：

- `/` 词法识别和注释识别共存
- RV64 下 `ra` 使用 `sd/ld` 保存恢复
- 函数调用前后保存 caller-saved 整数临时寄存器
- 函数调用前后保存 caller-saved 浮点临时寄存器
- 多于 8 个参数时的栈上传参与读取
- 大栈帧使用 `li` + `sub/add sp`，避免超出 `addi` 12-bit 立即数范围
- `int` 算术运算使用 RV64 word 指令，如 `addw/subw/mulw/divw/remw`

## 目录结构

```text
.
├── cmake/                  # CMake 辅助函数
├── fixtures/               # 简单输入样例和部分汇编样例
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
├── test/custom/            # 本地 SysY 样例
├── tests/                  # CTest 测试程序
├── tools/                  # 编译器入口和测试脚本
└── CMakeLists.txt
```

## 构建

在项目根目录执行：

```powershell
cmake -S . -B build
cmake --build build
```

启用项目级优化开关（默认关闭）：

```powershell
# 在配置阶段启用优化（将添加编译器优化标志并定义宏 ENABLE_OPTIMIZATION）
cmake -S . -B build -DENABLE_OPTIMIZATION=ON
cmake --build build
```

Windows/MinGW 单配置生成器通常会生成：

```text
build/compiler.exe
```

Visual Studio 等多配置生成器可能生成在：

```text
build/Debug/compiler.exe
```

如果直接运行 `compiler.exe` 没有输出或异常退出，请确认 MinGW 运行库目录已经加入 `PATH`。本地脚本会尝试自动添加仓库中配置过的 MinGW 路径。

## 使用方式

输出 RISC-V 汇编：

```powershell
.\build\compiler.exe test\custom\reduction.sy -S -o test\custom\out\reduction.s
```

测评兼容的优化参数：

```powershell
.\build\compiler.exe testcase.sy -S -o testcase.s -O1
```

如果省略 `-o`，结果会输出到标准输出。

## 调试输出

编译器支持输出前端到后端的中间结果：

```powershell
.\build\compiler.exe test/custom/reduction.sy --dump-tokens -o test/custom/out/reduction.tokens.txt
.\build\compiler.exe test/custom/reduction.sy --dump-ast -o test/custom/out/reduction.ast.txt
.\build\compiler.exe test/custom/reduction.sy --emit-raw-ir -o test/custom/out/reduction.raw.ir
.\build\compiler.exe test/custom/reduction.sy --emit-ir -o test/custom/out/reduction.ir
.\build\compiler.exe test/custom/reduction.sy -S -o test/custom/out/reduction.s
```

建议排查顺序：

```text
tokens -> AST -> raw IR -> optimized IR -> ASM
```

例如，遇到除法问题时先看 token 中是否有 `Operator(/)`；遇到后端问题时再看 `.s` 中是否生成 `divw`、栈帧恢复是否和函数入口匹配。

## 批量测试

运行 CTest：

```powershell
ctest --test-dir build --output-on-failure
```

批量生成 `test/custom` 下所有样例的 IR 和汇编：

```powershell
.\tools\test_all_sysy.ps1 -Build
```

该脚本会检查生成结果中是否出现明显坏汇编模式，如：

- `unknown`
- 未替换的虚拟寄存器
- `sp_ref`
- 可疑浮点立即数加载

如果本地具备 RISC-V 工具链和运行库，可以使用 Python 脚本尝试链接运行：

```powershell
python .\tools\run_sysy_tests.py -t test/custom/reduction.sy --build
```

该运行方式需要：

- `riscv64-linux-gnu-gcc`
- `qemu-riscv64-static`
- `test/official/sylib.c`

缺少这些文件或工具时，脚本会退化为 compile-only。

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
  -> RISC-V backend
```

### 前端

前端代码集中在 `include/Parse` 和 `lib/Parse`：

- `Lexer` 负责识别标识符、整数、浮点数、运算符、分隔符和注释
- `Parser.h` 实现轻量 parser combinator
- `Grammar.cpp` 描述 SysY 语法规则并构造 AST
- `AST.h` 定义 AST 节点类型和 Visitor 接口

表达式优先级按以下层次实现：

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

### IR 和优化

IR 相关代码集中在 `include/ir` 和 `lib/IR`：

- `Value / User / Use` 表示值和使用关系
- `Instruction` 定义二元运算、比较、内存访问、分支、返回、调用、Phi 等 IR 指令
- `BasicBlock / Function / Module` 组成 CFG 和模块结构
- `Pass / PassManager` 提供优化 pass 流水线

当前优化能力包括：

- 删除不可达块
- 简化恒定条件分支
- 常量表达式折叠
- 局部代数恒等式化简
- 基本块内 load forwarding
- 重复 store 删除
- 死代码删除

### 后端

后端代码集中在 `include/backend` 和 `lib/backend`：

- `AsmModule` 管理汇编模块输出
- `AsmFunction` 管理函数级汇编生成、栈帧、活跃变量分析、冲突图和寄存器分配
- `AsmBasicBlock` 将 IR 指令转换为 RISC-V 指令
- `Instructions.h` 定义 RISC-V 指令建模和输出
- `RiscVOperand` 定义寄存器、立即数、标签、地址等操作数

当前目标是生成 RV64 RISC-V 汇编。SysY `int` 按 32 位语义处理，因此整数算术使用 word 指令；地址计算和栈指针操作仍使用 64 位寄存器语义。

## 已知限制

- 当前本地脚本主要验证能否生成 IR/ASM 以及基础汇编格式，是否能运行取决于本机是否具备 RISC-V GCC、QEMU 和 SysY 运行库
- 寄存器分配和 caller-saved 保存策略偏保守，仍有优化空间
- 完整 ABI、复杂浮点场景、数组和指针场景仍建议继续扩展官方样例覆盖
- 尚未实现完整 `Mem2Reg`、`LowerPhi`、全局值编号等更强优化

## 开发建议

推荐修改后至少运行：

```powershell
cmake --build build
.\tools\test_all_sysy.ps1 -Build
```

对后端改动，建议额外检查：

- 函数入口 `sp` 减少量和返回前恢复量一致
- `sd/ld` 的 `sp` 偏移为 8 字节对齐
- `addi/lw/sw/flw/fsw/ld/sd` 立即数不超过 12-bit 范围
- call 前后需要保留的 caller-saved 整数/浮点临时寄存器已保存恢复

## 合规说明

本项目不依赖 GCC、LLVM 等现有开源编译器框架源码。前端、IR、中端 pass 框架和后端均为项目内自定义实现。

项目开发过程中使用 OpenAI Codex 作为辅助工具，辅助范围包括代码排查、文档整理、测试命令整理和局部实现建议。相关修改均经过人工审阅，并通过项目构建和测试脚本验证。
