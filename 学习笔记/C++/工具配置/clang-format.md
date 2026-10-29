Language: Cpp 
指定格式化语言为 C++
AccessModifierOffset: -4
访问控制符相对于类的缩进偏移量，向左偏移 4 个空格
AlignAfterOpenBracket: Align 
多行内容与左括号后的第一个元素对齐
AlignConsecutiveAssignments: false
不对齐连续赋值语句中的等号
AlignConsecutiveDeclarations: false 
 不对齐连续声明的变量名
AlignEscapedNewlines: Right 
 预处理指令续行的反斜杠尽可能向右对齐
AlignOperands: true 
 多行表达式中的操作数尽可能对齐
AlignTrailingComments: true 
 对齐连续代码行末尾的注释
AllowAllParametersOfDeclarationOnNextLine: false 
 不允许将声明的所有参数整体移动到下一行
AllowShortBlocksOnASingleLine: true 
 允许短代码块写在同一行
AllowShortCaseLabelsOnASingleLine: true 
 允许短 case 标签及其语句写在同一行
AllowShortFunctionsOnASingleLine: None 
 不允许短函数定义压缩为单行
AllowShortIfStatementsOnASingleLine: true 
 允许短 if 语句写在同一行
AllowShortLoopsOnASingleLine: true 
 允许短循环语句写在同一行
AlwaysBreakAfterReturnType: None 
 不强制在返回类型之后换行
AlwaysBreakBeforeMultilineStrings: false 
 不强制在多行字符串字面量之前换行
AlwaysBreakTemplateDeclarations: true 
 模板声明始终单独换行处理
BinPackArguments: true 
 允许函数调用的多个参数尽可能放在同一行
BinPackParameters: true 
 允许函数声明的多个参数尽可能放在同一行
BraceWrapping: 
 自定义花括号换行规则
  AfterClass: false # 类定义的左花括号不另起一行
  AfterControlStatement: false # 控制语句的左花括号不另起一行
  AfterEnum: false # 枚举定义的左花括号不另起一行
  AfterFunction: false # 函数定义的左花括号不另起一行
  AfterNamespace: false # 命名空间定义的左花括号不另起一行
  AfterStruct: false # 结构体定义的左花括号不另起一行
  AfterUnion: false # 联合体定义的左花括号不另起一行
  AfterExternBlock: false # extern 块的左花括号不另起一行
  BeforeCatch: false # catch 前不换行
  BeforeElse: false # else 前不换行
  IndentBraces: false # 花括号本身不额外缩进
  SplitEmptyFunction: false # 不强制将空函数的花括号拆分到多行
  SplitEmptyRecord: false # 不强制将空类或结构体的花括号拆分到多行
  SplitEmptyNamespace: false # 不强制将空命名空间的花括号拆分到多行
BreakBeforeBinaryOperators: NonAssignment 
 非赋值类二元运算符放在换行后的行首
BreakBeforeBraces: Custom 
 使用自定义的花括号换行规则
BreakBeforeTernaryOperators: false 
 三元运算符不强制放在换行后的行首
BreakConstructorInitializers: AfterColon 
 构造函数初始化列表在冒号后换行
BreakStringLiterals: false 
 不自动拆分字符串字面量
ColumnLimit: 0 
 不限制每行的最大列数
CompactNamespaces: true 
 允许连续嵌套的命名空间采用紧凑形式
ConstructorInitializerAllOnOneLineOrOnePerLine: false 
 不强制初始化列表全部写在一行或每项独占一行
ConstructorInitializerIndentWidth: 4 
 构造函数初始化列表缩进 4 个空格
ContinuationIndentWidth: 4 
 换行后续内容缩进 4 个空格
Cpp11BracedListStyle: true 
 使用 C++11 花括号初始化列表风格
DerivePointerAlignment: false 
 不根据现有代码自动推断指针符号的对齐方式
FixNamespaceComments: true 
 自动修正命名空间结束处的注释
IndentCaseLabels: false 
 case 标签不额外缩进
IndentPPDirectives: None 
 不对预处理指令额外缩进
IndentWidth: 4 
 每级缩进 4 个空格
IndentWrappedFunctionNames: false 
 函数名因换行而单独出现在新行时不额外缩进
KeepEmptyLinesAtTheStartOfBlocks: false 
 删除代码块开头的空行
MaxEmptyLinesToKeep: 1 
 最多保留一个连续空行
NamespaceIndentation: None 
 命名空间内部不增加额外缩进
PointerAlignment: Right 
 指针符号靠近变量名，例如 int \*ptr
ReflowComments: true 
 允许重新排版注释以符合格式规则
SortIncludes: false 
 不自动排序头文件
SortUsingDeclarations: false 
 不自动排序 using 声明
SpaceAfterCStyleCast: false 
 C 风格类型转换的右括号后不强制添加空格
SpaceAfterTemplateKeyword: true 
 template 关键字后添加空格
SpaceBeforeAssignmentOperators: true 
 赋值运算符前添加空格，例如 a = b
SpaceBeforeParens: ControlStatements 
 控制语句关键字后添加空格，例如 if (x)
SpaceInEmptyParentheses: false 
 空圆括号内不添加空格，例如 foo()
SpacesBeforeTrailingComments: 1 
 行尾注释前保留一个空格
SpacesInAngles: false 
 模板尖括号内部不添加空格，例如 vector<int>
SpacesInCStyleCastParentheses: false 
 C 风格类型转换的圆括号内不添加空格，例如 (int)x
SpacesInContainerLiterals: true 
 容器或列表字面量的分隔符内部允许添加空格
SpacesInParentheses: false 
 普通圆括号内部不添加空格，例如 (x + y)
SpacesInSquareBrackets: false 
 方括号内部不添加空格，例如 arr[i]
Standard: Cpp11 
 按 C++11 标准相关规则处理代码格式
TabWidth: 4 
 一个制表符的显示宽度为 4 个空格
UseTab: Never 
 始终使用空格缩进，不使用制表符
AllowShortRecordsOnASingleLine: true 
 允许短类、结构体等记录类型定义为单行