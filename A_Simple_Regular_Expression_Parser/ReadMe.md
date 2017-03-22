这个项目的目的不是构建一个正则表达式引擎,因为之前我已经写过了.
这个项目的规模应该会很小,因为我仅仅想来尝试一下用另外一种方法来写正则表达式的`Parser`,之前写的那个`Parser`,实际上使用的是堆栈式计算器的原理,貌似挺简单,但是调试起来并没有那么方便,我其实在调试的时候花费了很多的功夫,而且,我觉得上一个`Parser`使用了过多的技巧,通用性不强.

最近才知道,我们可以用通用的办法来实现这个正则表达式的`Parser`,所以我就想来尝试一下.



写了一部分才知道,用这种方法来写`Parser`非常简单,推荐使用这种方式.

`parser`中使用到的语法如下:
```shell
start -> RegularExpression

RegularExpression -> Items RegularExpression | Items  Alternative RegularExpression | ε

Items -> Item | Item Repeate Greedy

Item -> Range | method | LBracket Item RBracket

method -> methodLBraket action RBracket

action -> positiveMatch | reverseMatch | catch | anonymousCatch | check

positiveMatch -> equal RegularExpression

reverseMatch ->  exclaimation RegularExpression

catch -> colon catchName RegularExpression

anymousCatch -> colon RegularExpression

check -> checkName
```

`start`是开始符号.

`Range`表示字符范围,是一个终结符,它包括所有的字符,还包括`[xxx]`, `[ ^ xxx]`之类的字符,你可能会感到奇怪,为啥呢么我们不用语法来解析字符,兄弟,清醒一点,这么干真的没有必要,还会极大地增加语法的复杂性,你试想一下,如果我们要用语法来匹配`[xxx]`这种东西,我们要花费多少条规则,如果真这么弄下去,你最终会干不下去的.其实单个的字符,以及`[xxx]`,`.`都是等价的,都表示一个字符范围,只是单个字符表示的范围里只有一个字符而已.

`equal`表示`=`,是终结符,`exclaimation`表示`!`, `colon`表示`!`,它们都是终结符.

`checkName`表示这样的东西`<$xxx>`,`catchName`表示`<#xxx>`,它们也是终结符.

`LBracket`表示`(`,`RBracket`表示`)`,`MethodBracket`表示`(:`,它们是终结符.

`Repeate`有三种形式,分别是`+`, `*`, `{x,y}`,这里直接将这三种形式标记为`Repeate`,也是终结符,`?`表示非贪婪,也就是`Greedy`,终结符.

其余的都是非终结符.



语法没有太大的问题,没有左递归,用递归向下分析的办法可以非常容易地写出一个`Parser`.

