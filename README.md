# Heap Overflow DotsAndBoxes

## 使用方法
确保MinGW使用MSVCRT，gcc版本为11.2。请使用[MinGW-w64](https://ysod.azurewebsites.net/%E5%BC%80%E5%8F%91/MinGW/winlibs-x86_64-posix-seh-gcc-11.2.0-mingw-w64msvcrt-10.0.0-r1.zip)

### 使用CMake 
首先安装[CMake](https://github.com/Kitware/CMake/releases/download/v3.24.0-rc4/cmake-3.24.0-rc4-windows-x86_64.msi)
#### VS Code
1. git clone https://gitee.com/YunShu007/dots-and-boxes.git
2. 安装[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)插件
3. 重启VS Code
4. 重新打开会让你选择工具,选择`GCC 11.2.0 X86_64-w64-mingw32-make`
5. 如果上一步不小心选错，按`Ctrl`+`Shift`+`P`打开指令栏，输入kit,点击`CMake: 选择工具包`重新选择
6. 成功后点击VS Code下方状态栏上的运行按钮即可运行，如需调试请安装官方插件[C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)

tasks.json:
```json
{
	"version": "2.0.0",
	"tasks": [
		{
			"type": "cmake",
			"label": "CMake: build",
			"command": "build",
			"targets": [
				"dots_and_boxes"
			],
			"group": {
				"kind": "build",
				"isDefault": true
			},
			"problemMatcher": [],
			"detail": "CMake template build task"
		}
	]
}
```

launch.json:
```json
{
    // 使用 IntelliSense 了解相关属性。 
    // 悬停以查看现有属性的描述。
    // 欲了解更多信息，请访问: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(gdb) 启动",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/dots_and_boxes.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/build/",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "为 gdb 启用整齐打印",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description":  "将反汇编风格设置为 Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "CMake: build"
        },
    ]
}
```

#### Clion
1. 打开Clion，选择从VCS导入，输入https://gitee.com/YunShu007/dots-and-boxes.git
2. 等待加载，完成后会弹出配置cmake的窗口，直接点确定即可。

> Clion自带Cmake和符合要求的MinGW,所以可以开箱即用
## 开发备忘库
### AI设计
入口函数为`void UCT(Board *board, int player)`,在此函数中更改AI逻辑

### 棋盘
棋盘是11x11大小,有点(DOT),横线(Horizon),竖线(Vertical),被占据的线(OCCed_LINE),格子(_BOX_),先手黑方格子(BLACK__BOX_),后手白方格子(WHITE__BOX_).

棋盘使用二位数组int8_t _MAP_[11][11]储存，横纵坐标皆为偶数时点，皆为奇数时格子，横偶竖奇是横线，横奇竖偶是竖线。

判定一个坐标是否为未被占据的线可以使用if(_MAP_[i][j]!=OCCed_LINE)或者if(_MAP_[i][j]==Horizon||_MAP_[i][j]==Vertical)

判定一个格子是否被占可以使用if(_MAP_[i][j]==_BOX_)


### 坐标
