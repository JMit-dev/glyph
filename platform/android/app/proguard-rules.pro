# SDL and Glyph have no Java code to obfuscate beyond SDLActivity.
# Keep SDL's JNI-called methods so R8 doesn't strip them.
-keep class org.libsdl.app.** { *; }
