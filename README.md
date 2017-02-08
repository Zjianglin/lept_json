# lept_json
##Simple json parser followed with [json_tutorial](https://github.com/miloyip/json-tutorial)

###json sytax
```
    JSON-text = ws value ws
    ws = *(%x20 / %x09 / %x0A / %x0D)
    value = null / false / true 
    null  = "null"
    false = "false"
    true  = "true"
```
当中 %xhh 表示以 16 进制表示的字符，/ 是多选一，* 是零或多个，() 用于分组。
