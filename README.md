# Srey's GFX Demo Framework 

A Custom-Built OpenGL and C++ Graphics Framework by Sreyash Raychaudhuri

Add or remove shapes in the scene. You can add cubes, spheres or quads - and do whatever you want with them. 
Transform them and apply the shading you desire for each shape: Shapes can be light sources, or be shaded according to the Blinn-Phong or PBR models - upto the user.
Apply texture packs or just do pain shading, apply filters, toggle deferred shading, choose environments and more. Important Note: Deferred Shading is only setup for PBR.

Also, the engine has audio!  
Select any song and let your shapes groove. 

Some salient features are listed as follows:

Graphics Features:
* Physically Based Rendering (PBR) using texture maps or plain shading
* Diffuse Image Based Lighting (IBL)
* Blinn-Phong Lighting
* Deferred Shading (Only for PBR)
* Point Shadows (Only works with deferred rendering and for one light source right now)
* Normal Mapping
* Post-Processing filters: Saturation, Inversion and Outlines
* HDR and Tone-mapping

Important Notes:
* Shadow Mapping only works with Deferred Shading for now
* Outlines around selected shapes appear only when Def. Shading is off
* Texture Packs don't work on Cubes and Quads yet. For these other forms of shading works just fine.

