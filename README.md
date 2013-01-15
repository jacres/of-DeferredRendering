# Deferred Rendering + SSAO in openFrameworks

A basic and work in progress deferred rendering system + SSAO for openFrameworks. It's currently in a very verbose and unoptimized state, but I've decided to post a version of it in this form as it should be a lot clearer and hopefully serve as a clear example of how to get started with deferred rendering in OF.

Tested with OF 0073 on OS X 10.7+10.8. This uses OpenGl 2.1 + GLSL 1.20 in order to stay as compatible as possible with the way OF works with rendering.

A GBuffer class and shaders show how to generate a GBuffer containing view-space position, view-space normals, linear depth, and albedo (full colour)

Very much WIP - a few things left to do:
- implement light volumes to cut down on the number of fragments processed
- extract view-space position from depth and remove view-space position from GBuffer
- material indexing in storing lookup values for material properties in a 3D texture (this presents a bit of a challenge in OF as it won't work with the way things are currently rendered)

The SSAO implemented in this example uses the basic method outlined in this great article by Nathaniel Meyer: http://devmaster.net/posts/3095/shader-effects-screen-space-ambient-occlusion.

<img src="http://farm9.staticflickr.com/8223/8253081005_2e6493be39_z_d.jpg" />
<img src="http://farm9.staticflickr.com/8367/8385201806_a60b986ff0_b_d.jpg" />
<img src="http://farm9.staticflickr.com/8190/8384116631_58567c7ffd_z_d.jpg" />
