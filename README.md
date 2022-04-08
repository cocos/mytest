3rd:
	Windows平台需要解压3rd/embree目录下的lib(由于github限制，无法上传大文件)
	
make: 
	windows平台
		直接点击build/win/make.bat
		打开build/win/bin下的vs工程
	macosx平台
		打开终端cd到build/mac
		执行sudo bask ./make.sh
		打开build/mac/bin下的xcode工程
	
debug:
	编辑器lightmap插件，LFX_App.ts开启lfx_debug = true, 需要重新编译，设置LFX_App.js lfx_debug = true就不需要编译了
	打开LightFX工程，将调试目录设置到工程要生成的目录，例如：..\..\..\..\cocos\NewProject-003\assets\LightFX
	编辑器lightmap面板点击生成，会等待LightFX连接，这时候以调试模式启动lightFX工程即可断点调试
