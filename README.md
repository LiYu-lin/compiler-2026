# Compiler Competition 2026

这是一个面向 SysY 子集的 C++17 编译器项目。当前主流程可以把 `.sy` 源程序解析为 AST，生成项目自定义 IR，根据参数运行中端优化 pass，并输出 RV64 RISC-V 汇编。

## 项目状态

- **构建系统**：CMake
- **语言标准**：C++17
- **前端**：手写 Lexer、Parser Combinator、SysY Grammar 和 AST
- **中端**：自定义 IR、IRBuilder、PassManager（支持整体不动点安全迭代）
- **后端**：RV64 RISC-V 汇编生成、栈帧布局、调用约定处理、活跃变量分析、冲突图染色寄存器分配（支持 Spill 多轮循环着色图迭代）、基础 spill重写
- **测评入口**：生成名为 `compiler` 的可执行文件，兼容 `compiler testcase.sy -S -o testcase.s [-O1]`

当前已接入的优化 pass（默认在 `-O0` 下关闭，`-O1`/`-O2` 激活）：

- `SimplifyCFG`（常数分支折叠与不可达块消除）
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
- 指针与 64 位全字长变量 Spill 时严格配对 `ld/sd` 指令，防止大地址空间下高 32 位被截断

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

构建
在项目根目录执行（默认编译为安全无优化的编译器版本）：

PowerShell
cmake -S . -B build
cmake --build build
启用项目级优化开关（编译得到采用 -O2 性能加速的编译器本身）：

PowerShell
# 在配置阶段启用优化（将添加编译器优化标志并定义宏 ENABLE_OPTIMIZATION）
# Compiler Competition 2026

这是一个面向 SysY 子集的 C++17 编译器项目。当前主流程可以把 `.sy` 源程序解析为 AST，生成项目自定义 IR，根据参数运行中端优化 pass，并输出 RV64 RISC-V 汇编。

## 项目状态

- **构建系统**：CMake
- **语言标准**：C++17
- **前端**：手写 Lexer、Parser Combinator、SysY Grammar 和 AST
- **中端**：自定义 IR、IRBuilder、PassManager（支持整体不动点安全迭代）
- **后端**：RV64 RISC-V 汇编生成、栈帧布局、调用约定处理、活跃变量分析、冲突图染色寄存器分配（支持 Spill 多轮循环着色图迭代）、基础 spill重写
- **测评入口**：生成名为 `compiler` 的可执行文件，兼容 `compiler testcase.sy -S -o testcase.s [-O1]`

当前已接入的优化 pass（默认在 `-O0` 下关闭，`-O1`/`-O2` 激活）：

- `SimplifyCFG`（常数分支折叠与不可达块消除）
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
- 指针与 64 位全字长变量 Spill 时严格配对 `ld/sd` 指令，防止大地址空间下高 32 位被截断

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

在项目根目录执行（默认编译为安全无优化的编译器版本）：

```powershell
cmake -S . -B build
cmake --build build
```

启用项目级优化开关（编译得到采用 -O2 性能加速的编译器本身）：

```powershell
# 在配置阶段启用优化（将添加编译器优化标志并定义宏 ENABLE_OPTIMIZATION）
cmake -S . -B build -DENABLE_OPTIMIZATION=ON
cmake --build build
```

Windows/MinGW 单配置生成器通常会生成：

```
build/compiler.exe
build/sysy_compiler.exe
```

如果直接运行 compiler.exe 没有输出或异常退出，请确认 MinGW 运行库目录已经加入 PATH。

## 使用方式

默认无优化编译（-O0 模式，中端流水线完全放行，最安全稳定）：

```powershell
.\build\compiler.exe test\custom\reduction.sy -S -o test\custom\out\reduction.s
```

带中端优化管线编译（-O1 或 -O2 模式，激活常数分支折叠、死代码消除等）：

```powershell
.\build\compiler.exe testcase.sy -S -o testcase.s -O1
```

如果省略 -o，结果会输出到标准输出。

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

```
tokens -> AST -> raw IR -> optimized IR -> ASM
```

## 批量测试

运行 CTest：

```powershell
ctest --test-dir build --output-on-failure
```

使用本地 Python 批处理脚本进行编译质量扫描：

```powershell
python .\tools\run_sysy_tests.py -v
```

该脚本会批量并发编译 test/custom 下的所有用例，并自动检查生成结果中是否出现明显的坏汇编模式（如未替换的虚拟寄存器、可疑浮点加载等）。如果系统环境具备 riscv64-linux-gnu-gcc 与 qemu-riscv64-static，脚本会自动推进至链接和仿真差分测评状态。

## 编译流程

主流程位于 tools/sysy_compiler.cpp：

```text
SysY source
  -> Lexer
  -> Parser / Grammar
  -> AST
  -> GenIR Visitor
  -> IR Module
  -> PassManager (Controlled by -O0/-O1)
  -> RISC-V backend (RegAlloc Loops)
```

## 前端

前端代码集中在 include/Parse 和 lib/Parse：

Lexer 负责识别标识符、整数、浮点数、运算符、分隔符和注释，完美支持 / 词法识别与注释拦截共存。

Parser.h 实现轻量 parser combinator 拓扑。

Grammar.cpp 描述 SysY 语法规则并构造 AST，修正了复杂条件与算术表达式的优先级规约链。

AST.h 定义 AST 节点类型和 Visitor 接口。

## IR 和优化

IR 相关代码集中在 include/ir 和 lib/IR：

Value / User / Use 表示值和使用关系。

Instruction 定义二元运算、比较、内存访问、分支、返回、调用、Phi 等 IR 指令。

Pass / PassManager 提供中端优化流水线，总控层引入了全局不动点迭代安全上限（最高 10 轮），防止 Pass 间交替震荡导致编译卡死。

## 后端

后端代码集中在 include/backend 和 lib/backend：

AsmModule 管理汇编模块输出，全局变量寻址全面对齐 -mcmodel=medany 大地址空间 lla 伪指令规范。

AsmFunction 管理函数级汇编生成、栈帧布局、调用约定处理、活跃变量分析与寄存器分配。

Instructions.h 严格遵循 RISC-V 调用约定（Calling Convention），在 Call 算子中加入 ra 活跃度污染保护。

寄存器分配器采用 Spill 多轮循环着色图迭代算法，当发生内存溢出时，系统为其分配短寿命的全新临时虚拟寄存器并重新启动活跃性分析构图，同时硬编码了最高 50 轮的安全迭代上限，结合末端的 fallbackMap 降级策略，彻底杜绝了密集计算大用例下的图着色死循环。

## 已知限制

当前本地脚本主要验证能否生成 IR/ASM 以及基础汇编格式，是否能运行取决于本机是否具备 RISC-V 仿真环境。

寄存器分配和 caller-saved 保存策略偏保守，仍有优化空间。

尚未实现完整的 Mem2Reg、LowerPhi、全局值编号等更强优化。

## 开发建议

推荐修改后至少运行：

```powershell
cmake --build build
python .\tools\run_sysy_tests.py -v
```

对后端改动，建议额外检查：

- 函数入口 sp 减少量和返回前恢复量一致，且满足 16 字节对齐。
- sd/ld 的 sp 偏移为 8 字节对齐。
- addi/lw/sw/flw/fsw/ld/sd 立即数不超过 12-bit 范围。
- 条件跳转（如 bne/beq）下方强制追加 j 伪指令锁死终点，防止后端 CFG 物理块重排后发生落空（Fall-through）逻辑错误。

## 合规说明

本项目不依赖 GCC、LLVM 等现有开源编译器框架源码。前端、IR、中端 pass 框架和后端均为项目内自定义实现。

项目开发过程中使用 OpenAI Codex 作为辅助工具，辅助范围包括代码排查、文档整理、测试命令整理和局部实现建议。相关修改均经过人工审阅，并通过项目构建和测试脚本验证。
