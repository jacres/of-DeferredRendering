# Exponential Shadow Mapping in openFrameworks

Tested with OF 0073 on OS X 10.7+10.8 and Arch Linux. This uses OpenGl 2.1 + GLSL 1.20 in order to
as compatible as possible with the way OF works with rendering.
 
A basic and work in progress deferred rendering system for openFrameworks. It's currently in a very verbose and unoptimized state, but I've decided to post a version of it in this form as it should be a lot clearer and hopefully serve as a clear example of how to get started with deferred rendering in OF.

A GBuffer class and shaders show how to generate a GBuffer containing view-space position, view-space normals, linear depth, and albedo (full colour)

The SSAO implemented in this example uses the basic method outlined in this great article by Nathaniel Meyer: http://devmaster.net/posts/3095/shader-effects-screen-space-ambient-occlusion.

Screenshot here:
http://www.flickr.com/photos/85184046@N07/8283730333/in/photostream
