# Srey's GFX Demo Framework 

A Custom-Built OpenGL and C++ Graphics Framework 

Add or remove shapes in the scene. You can add cubes, spheres or quads - and do whatever you want with them
Transform them and apply the shading you desire for each shape: Shapes can be light sources, or be shaded according to the Blinn-Phong or PBR models - upto the user
Deferred Shading is setup only PBR Rendering. 

Oh, And the engine has audio!  
Select any song to make your shapes dance. 

Some salient features are listed as follows:

Graphics Features:
* PBR lighting using texture maps or plain shading
* Diffuse IBL
* Blinn-Phong Lighting
* Deferred Shading (Only for PBR)
* Point Shadows (Only works with deferred rendering and for one light source right now)
* Normal Mapping
* Post-Processing filters: Saturation, Inversion and Outlines
* HDR

Other features:
* Resource Manager to store textures, texture packs and environment maps
* Editor UI made using Dear ImGUI which allows shape selection, shape properties configuration, 
  G-Buffer display when doing Deferred Shading, environment map selection, filters and more.  
* Simple Audio Player: Audio data is also used to scale the shapes. The shapes dance, yes.


