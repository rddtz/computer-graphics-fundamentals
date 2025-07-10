Computer Graphics Fundamentals Final Project
===
This repository contains the code of the game that we made for out final project of the **Computer Graphics Fundamentals** course, a game phase (almost a tutorial phase) inspired by the valve's [[Portal game](https://store.steampowered.com/app/400/Portal/)]. This project was made by Rayan Raddatz ([@rddtz](https://github.com/rddtz)) and Gabriel Henrique ([@gabhen-fisbran](https://github.com/gabhen-fisbran)). During the process of making the game, Rayan's main task was to deal with the collision detection and handling and also the game physics, while Gabriel had the task of dealing with the modelling, the textures, the animation, the illumination and the portal logics. 

## Use of computer graphics concepts in the game
Here is a breef explanation of where we used the computer graphics concepts learned in the the game to fullfil the requirements:

- ***Complex Mesh:*** All the portal game props like the cube, the platform, the button and even the portal gun are complex mesh loaded through an .obj file. We downloaded most of the props from [this site](https://www.models-resource.com/pc_computer/portal/).
- ***Geometric Transformation of Virtual Objects:*** We used the mouse and keyboard to control the portals through the game, and also to carry the box and place it above the button in order to end the phase.
- ***Virtual Cameras:*** We have three cameras in the game, a free camera for the player, and two look-at cameras, one for each portal.
- ***Multiples Instances of an Object:*** The floor and the walls of the map are the same vertex set, also we have two portals and two platforms, who are diferent instances of the same vertex set.
- ***Intersection Tests:*** We used diferent intersection tests during the game. We used a line-plane test for the portal shoot, a point-plane test for the collision with wall and a cube-cube test for collisions with the platform and the button.
- ***Illumination Models:*** The diffuse (Lambert) illumination is implement by the platforms and the walls, while the Blinn-Phong all the other object with the expection of the floor. About the interpolations, almost all the objects use the Phong interpolation, with the exception being the door indicator sphere. 
- ***Textures:*** All our objects have their colors defined by a texture (with the exception of the cute little bunny that appears in the portals and the door indicator sphere).
- ***Bezier Splines:*** We used a bezier spline to define the path for the moving platform that leads the player towards the box.
- ***Time-based animations:*** All the movimentation and animations of the game are based on the real life time instead of varying based on the CPU.

### How to play?
The game has an easy and simple mechanic, the player must grab the box that lies in the highest platform and place it in the button in order to open the door and conclude the phase. The player can place portals in the walls in order to easily move around the map and achieve the goal. The following table describes the game commands:

![image](https://github.com/user-attachments/assets/350ade6d-9868-470e-969d-529635cee655)

![image](https://github.com/user-attachments/assets/243f78ab-2d9b-44cb-b586-dd080534629b)

![image](https://github.com/user-attachments/assets/2bd18f64-f980-43e3-8670-296bf07ca171)

| Key | Action |
| ----------- | ----------- |
| AWSD | Move the player |
| Space | Jump |
| E | Grab the box when close to it |
| Left Mouse Button | Shoot blue portal |
| Right Mouse Button | Shoot orange portal |

### Compilling and Running

#### On Linux with Make
To compile and run the code on a linux-based system, you can run the following command inside the game folder:
```
make run
```

#### On Windows (with VSCode and CMake)
On a windows system, you can configure the CMake extension in VSCode to compile and run the game.

### Note on use of AI Tools
We used LLM AIs in a few situations in order to understand better a concept (Rayan's case) or to configure libraries and tools (Gabriel's case). Rayan's used the Google Gemini in order to understand one equation that he was using for one of the collision tests, while Gabriel used Github Copilot integrate in VSCode to configure the automatic compilation of the project. We believe that the use of AI was beneficial in some small cases that do not involve the game logic, even though the models did not help in resolving bugs on the code.
