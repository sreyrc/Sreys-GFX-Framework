# Srey's GFX Demo Framework 

A Custom-Built OpenGL and C++ Graphics Framework 

Add or remove shapes in the scene. You can add cubes, spheres or quads - and do whatever you want with them. 
Transform them and apply the shading you desire for each shape: Shapes can be light sources, or be shaded according to the Blinn-Phong or PBR models - upto the user.
Apply texture packs or just do pain shading, apply filters, toggle deferred shading, choose environments and more. Important Note: Deferred Shading is only setup for PBR.

Also, the engine has audio!  
Select any song and let your shapes groove. 

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

Important Notes:
* Shadow Mapping only works with Deferred Shading for now
* Outlines around selected shapes only appear when Def. Shading is off
* Texture Packs don't work on Cubes and Quads yet. For these forms of shading works just fine.

