





Wecode代码检查记录文档
 修改日期：2022/1/17
 
1.	代码说明
代码名称：LcCode_V0.1_2022.1.11
功能说明：已实现高层协议栈G节点端LC层UM模式（mode 2-2）的基础功能，待完成与上下层的接口对应。
代码结构：
-header
-lc_dl_segment.h
-lc_entity_if.h
-lc_header_if.h
-lc_msg_if.h
-lc_ul_reassembly.h
-logicChannel_if.h
-msg_common.h
-source
-lc_dl_segment.c
-lc_entity_if.c
-lc_ul_reassembly.c

 
2.	检查结果
Wecode对代码进行了四类静态检查，分别是Codecheck、CodeMetrics、CodingStyle、CodeMars。

1)	Codecheck检查
简要说明：每个文件几乎都违反以下规则中的一半条以上。对每个文件的代码修改工作内容和工作量没有太大差别。
检查结果：所有文件违反规则如下。
规则1.1 标识符命名使用驼峰风格
规则1.3.1 使用数组作为函数参数时，应该同时将其长度作为函数的参数
规则2.1 只允许使用空格(space)进行缩进，每次缩进为4个空格。不允许使用Tab键进行缩进。
规则2.2 使用 K&R 缩进风格
规则2.8 switch 语句的 case/default 要缩进一层
规则2.13 水平空格应该突出关键字和重要信息，避免不必要的留白
规则3.1 文件头注释必须包含版权许可和功能说明
规则3.4 注释符与注释内容间要有1空格；右置注释与前面代码至少1空格
规则3.5 不用的代码段直接删除，不要注释掉
规则4.5 禁止通过声明的方式引用外部函数接口、变量。只能通过包含头文件的方式使用其他模块或文件提供的接口。
规则5.2 避免函数的代码块嵌套过深，不要超过4层。函数的代码块嵌套深度指的是函数中的代码控制块（例如：if、for、while、switch等）之间互相包含的深度。
规则6.6 禁止使用内存操作类危险函数，如memcpy，应改为memcpy_s
规则7.3 禁止无效、冗余的变量初始化
规则7.4 不允许使用魔鬼数字，解决途径见华为提供的网页版说明指导。
建议2.1 行宽不超过120个字符

2)	CodeMetrics检查
简要说明：给出的指标项中有两个指标项超出了给定的阈值。分别是dangerousFuncsTotal（危险函数个数）以及redundantCodeTotal（冗余代码数）。
检查结果：下表对所有文件进行了CodeMetrics检查并总结了各指标项的相关数据。需要额外注意的指标项已上色标出。
Summary Metrics	Result	Threshold
codeSize	893	/
rawLines	1406	/
methodsTotal（函数总数）
30	/
cyclomaticComplexityTotal	138	/
cyclomaticComplexityPerMethod	4.6	5
maximumCyclomaticComplexity	21	/
hugeCyclomaticComplexityTotal	1	50
hugeCyclomaticComplexityRatio	3.33	/
ccaCyclomaticComplexityTotal	134	/
ccaCyclomaticComplexityPerMethod	4.47	/
maximumCcaCyclomaticComplexity	21	/
hugeCcaCyclomaticComplexityTotal	1	/
ccaCyclomaticComplexityThreshold	20	/
maximumDepth（最大圈复杂度）
7	/
methodLines	541	/
linesPerMethod	18.03	30
hugeMethodTotal	2	200
hugeMethodRatio	6.67	/
filesTotal	10	/
foldersTotal	1	/
linesPerFile	89.3	300
hugeHeaderfileTotal	0	500
hugeHeaderfileRatio	0.0	/
hugeNonTotal	0	2000
hugeNonRatio	0.0	/
hugeFolderTotal（超大目录个数）	0	50
hugeFolderRatio	0.0	/
hugeDepthTotal（超大函数个数）	2	4
hugeDepthRatio
6.67	/
fileDuplicationTotal（重复文件数）	0	/
fileDuplicationRatio	0.0	/
fileDuplicationTotalNon	0	/
fileDuplicationRatioNon
0.0	4
codeDuplicationTotal（重复代码数）	28	/
codeDuplicationRatio
3.14	10
codeDuplicationTotalNon	0	/
codeDuplicationRatioNon	0.0	/
dangerousFuncsTotal（危险函数个数）	7	0
dangerousFuncsRatio	7	0
redundantCodeTotal（冗余代码数）	22	0
redundantCodeRatio	0.0	0
warnningSuppressionTotal（告警抑制声明个数）	0	0
warnningSuppressionRatio	0.0	0
hugeFolderThreshold	50	50
hugeHeaderfileThreshold	500	500
hugeNonThreshold	2000	2000
codeDuplicationThreshold	10	10
hugeMethodThreshold	50	200
hugeCyclomaticComplexityThreshold	20	50

3)	CodingStyle检查
简要说明：该检查与第一项CodeCheck检查的检查结果非常相近。可以在依据第一项CodeCheck检查的结果进行修改之后再进行该项检查。
检查结果：所有文件违反规则包括如下。
规则2.1 使用空格进行缩进，每次缩进4个空格
规则2.2 使用 K&R 缩进风格
规则2.8 switch 语句的 case/default 要缩进一层
规则2.13 水平空格应该突出关键字和重要信息，避免不必要的留白
规则3.1 文件头注释必须包含版权许可和功能说明
规则3.4 注释符与注释内容间要有1空格；右置注释与前面代码至少1空格
规则3.5 不用的代码段直接删除，不要注释掉
规则4.3 头文件应当自包含。自包含即任意一个头文件均可独立编译。如果一个文件包含某个头文件，还要包含另外一个头文件才能工作的话，给这个头文件的用户增添不必要的负担。
规则4.5 禁止通过声明的方式引用外部函数接口、变量
规则5.1 避免函数过长，函数不超过50行（非空非注释）
规则5.2 避免函数的代码块嵌套过深，不要超过4层规则7.3 禁止无效、冗余的变量初始化
规则7.1 模块间，禁止使用全局变量作接口
规则7.4 不允许使用魔鬼数字
建议1.2 文件命名统一采用小写字符
建议2.1 行宽不超过120个字符
建议2.3 指针类型"*"跟随变量名或者类型，不要两边都留有空格或都没有空格建议2.4 减少不必要的空行，保持代码紧凑
建议5.5 函数的指针参数如果不是用于修改所指向的对象就应该声明为指向const的指针

4)	CodeMars检查
简要说明：该项检查给出两项错误指示，第一项LengthNeedPass意味着使用数组作为函数参数时，应该同时将其长度作为函数的参数；第二项RiskyFunction意味着禁止使用内存操作类危险函数，应改为使用memcpy_s等安全函数。
检查结果：
Message	Rule
'scale' of function 'calculateUmRxWindowScale' is a static or dynamic array, when an array is used as a function argument, its length should be passed into the function.	LengthNeedPass

'scale' of function 'isInUmRecvWindow' is a static or dynamic array, when an array is used as a function argument, its length should be passed into the function.	
'scale' of function 'UmPduShouldBePutInWindow' is a static or dynamic array, when an array is used as a function argument, its length should be passed into the function.	
Risky function 'memcpy' is found. It is recommended to use corresponding safe function 'memcpy_s' instead.	RiskyFunction


Risky function 'memmove' is found. It is recommended to use corresponding safe function 'memmove_s' instead.	

