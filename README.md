# FBNative
Rewrite Fastbuilder in C++.  
Supported Platforms: Linux,macOS(,Windows,iOS:Hardly but possible)
# Usage
Usage is just like FastBuilder,use CCmd(Chat Command) to control,with a simple "-" in the head.  
like `-round -r 6`.  
## Command List
```
-round (-r)
-circle (-r)
-sphere (-s -r)
-ellipse (-d -l -w)
-ellipsoid (-l -w -h)
-torus (-r -l -a)
-cone (-d -h -r -a)
-pyramid (-s -r -h)
-ellipticTorus (-r -a -l -w -d)
```
## Args list & explanation
```
(double) -r : Structure radius
(string enum) -s <hollow|solid> : Structure shape.
(char enum) -d <x|y|z> : Direction.
(double) -l : Length.
(double) -w : Width.
(int) -h : Structure height.
(double) -a : Accuracy.
```

