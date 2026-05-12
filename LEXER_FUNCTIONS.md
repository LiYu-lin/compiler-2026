# Lexer 函数说明

本文档对应：
- `include/Parse/Lexer.h`
- `lib/Parse/Lexer.cpp`

## 1. Token 相关函数

### `Token::Token(Type type)`
- 作用：按类型创建一个基础 Token。

### `Token::Token(Type type, size_t line, size_t column, size_t tokPos)`
- 作用：创建并记录 Token 的位置信息（行、列、字符偏移）。

### `bool Token::is(Type t) const`
- 作用：判断当前 Token 类型是否等于 `t`。

### `std::string token::Ident::toString() const`
- 作用：把标识符 Token 转成可读字符串，如 `Ident(x)`。

### `std::string token::FloatConst::toString() const`
- 作用：把浮点数 Token 转成可读字符串，如 `FloatConst(3.140000)`。

### `std::string token::IntConst::toString() const`
- 作用：把整数 Token 转成可读字符串，如 `IntConst(42)`。

### `std::string token::Operator::toString() const`
- 作用：把运算符 Token 转成可读字符串，如 `Operator(+)`。

### `std::string token::Delimiter::toString() const`
- 作用：把分隔符 Token 转成可读字符串，如 `Delimiter(;)`。

## 2. TokenPtrIterator 函数

### `TokenPtr TokenPtrIterator::get() const`
- 作用：获取当前迭代器指向的 Token；到结尾时返回 `nullptr`。

### `TokenPtr TokenPtrIterator::operator*() const`
- 作用：解引用，等价于 `get()`。

### `TokenPtr TokenPtrIterator::operator->() const`
- 作用：成员访问；到结尾时返回 `nullptr`。

### `TokenPtrIterator &TokenPtrIterator::operator++()`
- 作用：前置自增，迭代器向后移动一位。

### `TokenPtrIterator TokenPtrIterator::operator+(size_t n) const`
- 作用：返回向后偏移 `n` 的新迭代器，不修改原迭代器。

### `bool TokenPtrIterator::operator!=(const TokenPtrIterator &other) const`
- 作用：判断两个迭代器是否不相等。

## 3. Lexer::Error 函数

### `Lexer::Error::Error(std::string message, size_t line, size_t column, size_t tokPos)`
- 作用：创建词法错误对象并保存定位信息。

### `std::string Lexer::Error::toString() const`
- 作用：把错误信息格式化成可读字符串，包含行列与偏移。

## 4. Lexer 构造与对外接口

### `Lexer::Lexer()`
- 作用：默认构造函数（当前实现中不建议直接使用，因为 `input` 是引用成员）。

### `Lexer::Lexer(std::istream &input)`
- 作用：绑定输入流，初始化词法分析器。

### `std::shared_ptr<Token> Lexer::nextToken()`
- 作用：词法分析主入口，每次调用返回一个下一个 Token。
- 关键行为：
  - 跳过空白字符。
  - 识别并跳过 `//`、`/*...*/` 注释。
  - 识别标识符、整数、浮点数、分隔符、运算符。
  - 输入结束时返回 `nullptr`。
  - 遇到非法字符时抛出 `newError(...)`。

### `Lexer::Error Lexer::newError(const std::string &message) const`
- 作用：按当前位置构造一个错误对象，统一错误上报格式。

## 5. Lexer 私有辅助函数

### `std::string Lexer::readAll(std::stringstream &buffer)`
- 作用：从 `stringstream` 中读取内容，遇到 `"EOF"` 停止。
- 备注：当前词法主流程未使用该函数，属于保留/遗留辅助函数。

### `char Lexer::peek()`
- 作用：查看下一个字符但不消耗输入位置。

### `char Lexer::get()`
- 作用：读取并消耗一个字符，同时更新 `tokPos`、`line`、`column`。
- 细节：读到换行符会让 `line++` 且 `column` 重置为 `1`。

### `void Lexer::unget()`
- 作用：回退一个字符，并回滚位置信息。
- 使用场景：超前读到“不属于当前 token”的字符时回退。

### `TokenPtr Lexer::nextIdentifier()`
- 作用：读取标识符（字母/数字/下划线序列），返回 `token::Ident`。
- 行为：若内容为空会抛出错误。

### `std::string Lexer::nextDecimalStr()`
- 作用：读取连续十进制数字串（仅字符形式）。
- 行为：若一个数字都没读到会抛出错误。

### `TokenPtr Lexer::nextHexadecimal()`
- 作用：读取十六进制数字（在 `0x` 后调用），返回 `token::IntConst`。
- 行为：使用 `std::stoi(..., 16)` 转换。

### `TokenPtr Lexer::nextOctal()`
- 作用：读取八进制数字（`0` 开头，非 `0x`、非 `0.`），返回 `token::IntConst`。
- 行为：若读不到后续八进制位，返回数值 `0`。

### `TokenPtr Lexer::nextNumber()`
- 作用：数字分发函数，统一处理整型/浮点、十进制/八进制/十六进制。
- 规则：
  - `0x...` -> 十六进制整数。
  - `0...` -> 八进制整数。
  - `0.xxx`、`.xxx`、`xxx.xxx` -> 浮点数。
  - `xxx` -> 十进制整数。

### `void Lexer::skipLineComment()`
- 作用：跳过单行注释内容（`//` 到行尾）。

### `void Lexer::skipMultilineComment()`
- 作用：跳过多行注释内容（`/* ... */`）。
- 行为：循环直到遇到配对结束符或 `EOF`。

## 6. 额外说明

- 关键字表在 `Token::reserved` 中定义：`if, else, while, return, break, continue, const, int, float, void`。
- 当前 `nextIdentifier()` 统一返回 `Ident`，关键字语义通常在后续语法阶段再区分。
