


# COMP3016 - Free Project
Final Grade: 100%
### ScubaSim

This program was developed using Visual Studio 2022 in Windows 11 using an Intel CPU.

The executable was compiled for the above platform & chipset.
The executable requires Visual Studio c++ runtimes to be installed.

I started this project using the template provided in this repository and the project2 repository.

Most of the setup and camera code were modified from the learnopengl.com examples.

The OBJ loader was entirely done by myself.

The GLTF loader was mostly done by myself; however, I used [this walk through video](https://www.youtube.com/watch?v=AB_f4slL2H0) to understand how the GLTF file format works.

The depth of field filter was adapted from the bloom tutorial on learnopengl.com and heavily modified to reverse it into a depth of field effect.

[Video demonstration](https://youtu.be/3IZtSvUnrOI)

## What makes this project unique
There are a few scuba diving games out there. The main two are subnautica and abzu. Both games have breathtaking scenery and do a fantastic job of portraying our underwater world's beauty. However, these games do not tackle some of the skills and knowledge required for real-life scuba diving. The two points that I have aimed to cover in this game are <b>Colour Skewing</b> and <b>Buoyancy</b>.

### Colour Skewing
As you go deeper underwater, different wavelengths of light can penetrate different depths. This means that as you go deeper, colours look different. The depth that wavelengths of light can penetrate can also depend on the conditions. Below is an example of how far light can penetrate in ideal conditions.

![Light penetration at depth](https://scubadiverlife.com/wp-content/uploads/2015/07/Color-light-and-depth-II-1024x768.jpg)
For this game, the conditions are less than perfect and are at more average levels.

 - Red light can penetrate 5m.
 - Green light can penetrate 25m.
 - Blue light can penetrate 35m.

An example of this in action can be seen below.

![Light penetration example](https://github.com/Plymouth-University/2022-comp3016-fp-mbruty/blob/main/images/colorshift.gif?raw=true)<br/>
This skewing is done in the default fragment shader. If this were to be done in the vertex shader, it could be more performant; however, on larger objects, the interpolation of colours needed to look better.

For the background colour, only the camera's depth is considered for the calculation.

![Light penetration backgruond](https://github.com/Plymouth-University/2022-comp3016-fp-mbruty/blob/main/images/colourskewbg.gif?raw=true)

For the other objects, depth is added to the distance from the camera to give a realistic effect. For example, a point on the boat at 2.5m would not have any visible red hues if you are more than 2.5m away from it. If the ship is set to over 35m deep, there will be no ambient light. An example of this in action for a larger object is below.

![Light penetration on boat](https://github.com/Plymouth-University/2022-comp3016-fp-mbruty/blob/main/images/colourskewboat.gif?raw=true)

The flashlight also has this colour-skewing effect applied to it. However, unlike the ambient light calculation, only the distance<sup>2</sup> is considered.

![enter image description here](https://github.com/Plymouth-University/2022-comp3016-fp-mbruty/blob/main/images/colourskewflashlight.gif?raw=true)

Finally, the jellyfish also has this colour skew effect on it; however, it is less noticeable as the jellyfish's ambient light colour is purple. So it only has a small amount of red light, and the jellyfish doesn't get far enough away from the boat for the blue light to drop off noticeably.

![enter image description here](https://github.com/Plymouth-University/2022-comp3016-fp-mbruty/blob/main/images/jellyfish.gif?raw=true)
### Buoyancy
Other scuba diving games do not mimic the effect of buoyancy on the player. This could drastically skew the expectations of people who these games have inspired to undertake real-life diving. Buoyancy is a significant factor in diving, and if done wrong can result in serious injury.

In this game, buoyancy is simulated, and the player can adjust their buoyancy using a buoyancy jacket. The buoyancy effect will alter how the player can move. For example, if the jacket is over-inflated, the player will still float up despite swimming downwards.

The compression of air in the buoyancy jacket is also simulated. The player may be neutrally buoyant but swim down far enough without adjustment, and you can find yourself dropping uncontrollably. This effect occurs in real life due to the air in the jacket compressing at depth, thus reducing the buoyancy.

In the gif below, this effect can be seen. I start out neutrally buoyant, then swim downwards to 2m. After which, I don't give any more input and let the decompression take the camera down.

![Buoyancy drop](https://github.com/Plymouth-University/2022-comp3016-fp-mbruty/blob/main/images/buoyancydrop.gif?raw=true)

## Setup and running

### Executable

[Download the executable](https://github.com/Plymouth-University/2022-comp3016-fp-mbruty/releases/download/0.1/game.zip).

### Compiling from source
### Visual Studio
Open the solution in VisualStudio, and run the build command.
Take the compiled executable and any .dll files, and move them to the project's root, as some required libraries aren't bundled into the executable.

## User interaction
The user can swim around the scene, adjust their buoyancy jacket to move up & down, and turn on a flashlight.
The user can use their mouse to look around the scene with a first-person-camera
<kbd>W</kbd> - Swim forwards
<kbd>A</kbd> - Swim to the left
<kbd>S</kbd> - Swim backwards
<kbd>D</kbd> - Swim to the right
<kbd>F</kbd> - Toggle flashlight
<kbd>Up arrow</kbd> - Increase the floor depth
<kbd>Down arrow</kbd> - Decrease the floor depth
<kbd>Shift</kbd> - Inflate buoyancy jacket
<kbd>Ctrl</kbd> - Deflate buoyancy jacket
<kbd>Space</kbd> - Dump air out of buoyancy jacket

## Classes
### Game
The game class handles the main functions of the game. On creation, it:

 - Creates the glfw window.
 - Sets up ImGui.
 - Loads in Gui images.
 - Sets up the camera.
 - Loads in and compiles shaders.
 - Sets up post-processing frame buffers.
 - Loads in required models.
 - Loads in audio files.

Once started, the game enters a while loop that first calls the update method, then the display method.

The update method processes keyboard inputs and sends them to the camera, sends mouse inputs to the camera, and rotates the jellyfish around the scene.

The display method clears the current buffer, sends the camera's view matrix to the shader, and then renders all models to a scene image and a filtered image of only the points within 5 meters of the camera.

The display method then does a two-pass gaussian blur on the scene image and then combines the blurred image with the in-focus image creating a depth-of-field effect.

The display method then draws the GUI on top of the final image.

### Camera
This class contains all the information about where the player/camera is and where they are looking. It handles all of the player movement and updates the player's y position based on the buoyancy jacket state.

The game object contains one camera object.

### Shader
This class handles the loading, compiling, error checking, binding, and deletion of shader programs.

The game object contains an array of shaders.

### GuiImage
This class handles loading images to be drawn with ImGui.
It also contains helper methods to handle the rotation of images. For example, this is used to rotate the compass on the dive computer.

### ModelLoader / ObjLoader
Both classes provide the same interface for file loading, with the model loader providing the GLTF implementation and the obj loader providing the obj implementation.

Both of these classes are used during the instantiation of the game object to load in the 3D models. The load method returns an object of type Model.

### Model
The model class stores all meshes, translations, rotations, scales, and matrices required for drawing the models into the scene. It also contains the model's position and rotation in world space.

The class contains one method to draw the mesh using the given shader program. It loops through all the provided mesh objects, calling draw on them.

### Mesh
The mesh class stores all of the vertices, indices (if provided) and textures for a mesh. On creation, it handles the creation of VAOs, VBOs and IBOs, setting the correct data into each buffer.

The draw method binds any textures if they are provided, then calls either <code>glDrawElements</code> or <code>glDrawArrays</code> depending on if indices were provided for index-based drawing.

A model is made up of an array of meshes.

### Vertex
The vertex struct contains the basic information about a vertex. It includes a vec3 for the position; a vec3 for the normal; a vec4 for the colour in RGBA format; and a vec2 for the texture coords.

A mesh is made up of an array of vertices.

### Texture
The texture class handles the loading of texture images and binding them.

A mesh can contain 0 to many textures.

### VAO
This is a helper class that encapsulates the creation of vertex arrays, linking to the buffer array via <code>glVertexAttribPointer</code>, binding, unbinding, and deletion.

Each mesh contains its own VAO.

### VBO
This is a helper class that encapsulates the creation of vertex buffers. It handles the buffer's creation, binding, unbinding, and deletion.

Each mesh contains its own VBO.

### EBO
This helper class encapsulates the creation of the element buffer object/index buffer object. It handles the buffer's creation, binding, unbinding, and deletion.

A mesh can contain one EBO if it uses index-based drawing.

## Utility methods
Inside of <code>utilities.cpp</code>, there is a reusable method for getting all of the lines from a file.

Inside of <code>Base64Decode.h</code> there is a method for getting bytes out of a base64 encoded octet-stream.

## Future improvements
The first issue with this project is that the floor that the ship is on fades to black at distance, which is how it should be; however, when the ambient background colour isn't black, the floor goes to black, when it should fade to the ambient colour.

If I had more time to work on this project, I would add:

 - Shadows.
 - Oxygen tank depleats air faster the more you move around.
 - A scoring system.
 - A larger ship to explore with more going on.
 - Add HDR support to help with the darkest areas.
 - PBR for better lighting reflections on the ship.
 - Ray marching light rays.
 - Improved depth of field using proper calculations instead of the "reverse bloom" effect.

## Resources used

 - Light penetration reference image: https://scubadiverlife.com/review-backscatter-flip-3-1-part-i/
 - Camera movement: https://learnopengl.com/Getting-started/Camera
 - Flash light and point light: https://learnopengl.com/Lighting/Light-casters
 - Basic mesh structure: https://learnopengl.com/Model-Loading/Mesh 
 - Basic model structure: https://learnopengl.com/Model-Loading/Model
 - Frame buffers: https://learnopengl.com/Advanced-OpenGL/Framebuffers
 - Bloom (base theory used for depth of field): https://learnopengl.com/Advanced-Lighting/Bloom
 - Flashlight sound: https://www.youtube.com/watch?v=Qr5UDz6B074
 - Buoyancy Jacket inflate: https://www.youtube.com/watch?v=5Y_FVK6Es0s
 - Ambient underwater breathing: https://www.youtube.com/watch?v=r141Lr8SWAs
 - Jellyfish: https://www.cgtrader.com/3d-models/animals/fish/stylized-glowing-jellyfish
 
 ### Packages
 - ImGui for gui
 - irrklang for sound
