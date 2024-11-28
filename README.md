# CPlusPlus3DRenderer

CPlusPlus3DRenderer is a windows program that makes 3D rendering on a CPU of scenes that are supported by assimp library. 

- The only file to be compiled is "win32_GraphicsPractise.cpp", other .cpp files are used to store functions that are added to main cpp file
- You also need to link an assimp library file in:
	- ./libs/assimp/lib/Debug/assimp-vc143-mtd.lib
- Built program need to be in the folder ./bin or be in the folder with same files that ./bin has

# How to add a model

All models are stored in ./data folder under their names. But to use them you need to change the code of "win32_GraphicsPractise.cpp" and "win32_GraphicsPractise.h" files

In "win32_GraphicsPractise.h":
- add to the struct GlobalState new member of type model

In "win32_GraphicsPractise.h":
- before while loop you need to add data of 3D model to the new 3D-model-member of GlobalState struct. Use function AssetLoadModel() give her a path of a folder where is 3D model file and a name of 3D model file
	- GlobalState.myModel = AssetLoadModel((char*)"../data/myModel/", (char*)"myModel.gltf");)

# Structure

The project consists of 4 modules:
- win32_GraphicsPractise
	- is a the core, where are declared all main structures and there is main function for window application, and other major functions of rendering process
- graphicsMath
	- is a part with all algebraic geometry structs (like vectors and matrices) and base functions to work with them
- clipper
	- is a part that makes clipping of a models polygon (cuts parts of polygon that are not presented on the screen)
- assets
	- is a part that uses assimp library to load models from different file types

Folders:
- bin
	- is a folder where you need to build a program
- data
	- is a folder with models, you can add new folders inside with your models (if their formats are supported by assimp)
- libs
	- contains header files of libraries that were used in programm
- doc
	- contains ukrainian tutorial made with Obsidian's .canvas format that can give you step by step understanding how the program were written 
# Credits

Thanks a lot to "УкрЗнання(UkrZnanja)" YouTube channel which is a source of all code and idea behind the project, you can find a lot of usefull information om ukrainian language with them
- the link to repository: https://github.com/UkrKnowledge/GraphicsTutorial
- the link to channel: https://www.youtube.com/channel/UC6TBxnPZnT_VY-P4ifSt_Hw 