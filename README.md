# Deferred Rendering + SSAO in openFrameworks

A basic and work in progress deferred rendering system + SSAO for openFrameworks. It's currently in a very verbose and unoptimized state, but I've decided to post a version of it in this form as it should be a lot clearer and hopefully serve as a clear example of how to get started with deferred rendering in OF.

Tested with OF 0073 on OS X 10.7+10.8. This uses OpenGl 2.1 + GLSL 1.20 in order to stay as compatible as possible with the way OF works with rendering.

A GBuffer class and shader (gbuffer.frag/.vert) show how to generate a GBuffer containing view-space position, view-space normals, linear depth, and albedo (full colour)

mainScene.frag shows how to use the GBuffer textures to perform deferred shading in view-space.

ssao.frag does a simple SSAO pass using the GBuffer position, depth, and normal data + a random jitter texture.
The SSAO implemented in this example uses the basic method outlined in this great article by Nathaniel Meyer: http://devmaster.net/posts/3095/shader-effects-screen-space-ambient-occlusion.

Very much WIP - a few things left to do:
- implement light volumes to cut down on the number of fragments processed
- extract view-space position from depth and remove view-space position from GBuffer
- material indexing in storing lookup values for material properties in a 3D texture (this presents a bit of a challenge in OF as it won't work with the way things are currently rendered)
- fix a bug where about 120px are being cropped from top of FBO textures on my MBP's Nvidia 330m card. Fixed this in another project awhile back, but forget what I did! I'll have to look it up - if anyone know the answer in the meantime, please hit me up ;)


Screenshots from http://www.flickr.com/photos/85184046@N07/

<img src="http://farm9.staticflickr.com/8367/8385201806_a60b986ff0_c_d.jpg" />
<img src="http://farm9.staticflickr.com/8223/8253081005_2e6493be39_c_d.jpg" />
<img src="http://farm9.staticflickr.com/8355/8384117427_00fcdb6f7e_c_d.jpg" />
